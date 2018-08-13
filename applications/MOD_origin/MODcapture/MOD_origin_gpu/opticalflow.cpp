#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <cmath>
#include "opticalflow.h"
#include "morph.h"
#include "features.h"
#include "utils.h"
#include "log.h"
#include "opencv2/gpu/gpu.hpp"

#define TRACKING 1

//comment this to suppress tracking vectors in debugging output
#define TRACKING_DEBUG

using namespace cv;
using namespace std;

#if 0
Rect rect1(40, 100, 120, 170);
Rect rect2(200, 100, 240, 210);
Rect rect3(480, 100, 120, 170);

Rect rect4(680, 100, 120, 170);
Rect rect5(840, 100, 240, 210);
Rect rect6(1120, 100, 120, 170);

Rect rect7(40, 460, 120, 170);
Rect rect8(200, 460,240, 210);
Rect rect9(480, 460, 120, 170);

Rect rect10(680, 460, 120, 170);
Rect rect11(840, 460, 240, 210);
Rect rect12(1120, 460, 120, 170);

Rect rect1(60,  10, 120, 220);
Rect rect2(205, 10, 230, 260);
Rect rect3(460, 10, 120, 220);

Rect rect4(700, 10, 120, 220);
Rect rect5(845, 10, 230, 260);
Rect rect6(1100, 10, 120, 220);

Rect rect7(60, 370, 120, 220);
Rect rect8(205, 370, 230, 260);
Rect rect9(460, 370, 120, 220);

Rect rect10(700, 370, 120, 220);
Rect rect11(845, 370, 230, 260);
Rect rect12(1100, 370, 120, 220);
#endif

Rect rect1(20,  50, 185, 210);
Rect rect2(205, 50, 230, 260);
Rect rect3(435, 50, 185, 220);

Rect rect4(660, 50, 185, 120);
Rect rect5(845, 50, 230, 260);
Rect rect6(1075, 50, 185, 120);

Rect rect7(20, 410, 185, 215);
Rect rect8(205, 410, 230, 290);
Rect rect9(435, 410, 185, 225);

Rect rect10(660, 410, 185, 180);
Rect rect11(845, 410, 230, 270);
Rect rect12(1075, 410, 185, 145);

static int log_level = LOG_INFO;
#define oferr 100

extern int rectstat[RECTSTAT_SIZE];

extern Mat colim;


void drawOptFlowMap (const Mat& flow, Mat& cflowmap, int step, const Scalar& color) {
    ENTER();
    for(int y = 0; y < cflowmap.rows; y += step)
        for(int x = 0; x < cflowmap.cols; x += step)
        {
            const Point2f& fxy = flow.at< Point2f>(y, x);
            line(cflowmap, Point(x,y), Point(cvRound(x+fxy.x), cvRound(y+fxy.y)),
                    color);
            circle(cflowmap, Point(cvRound(x+fxy.x), cvRound(y+fxy.y)), 1, color, -1);
        }
    LEAVE();
}

/*The below function will draw the sparse optical flow points*/
void drawoptflowsparse(Mat& prv,Mat& next,Mat& imsparse,vector<Point2f> *points,vector<uchar> &status,vector<float> err, CvScalar color)
{
    ENTER();
    //cout<<points[0].size()<<points[1].size()<<endl;
    imsparse=next.clone();
    for(size_t y=0;y<points[0].size();y++)
    {
        //cout<<int(status[y])<<" "<<status.size()<<" "<<points[0].size()<<endl;
        if(int(status[y]) && err[y]<oferr)
        {
            //cout<<"The error in this track is "<<err[y]<<endl;
            const Point2f& p1=points[0][y];
            const Point2f& p2=points[1][y];
            line(imsparse,Point(cvRound(p1.x),cvRound(p1.y)),Point(cvRound(p2.x),cvRound(p2.y)), color);
            circle(imsparse, Point(cvRound(p2.x), cvRound(p2.y)), 1, color, -1);
        }
    }
    LEAVE();
}

/*void drawoptflowsparse_gpu(Mat& prv,Mat& next,Mat& imsparse,vector<Point2f> *points,vector<uchar> &status,vector<float> err, CvScalar color)
{
    ENTER();
    //cout<<points[0].size()<<points[1].size()<<endl;
    imsparse=next.clone();
    for(size_t y=0;y<points[0].size();y++)
    {
        //cout<<int(status[y])<<" "<<status.size()<<" "<<points[0].size()<<endl;
        if(int(status[y]) && err[y]<oferr)
        {
            //cout<<"The error in this track is "<<err[y]<<endl;
            const Point2f& p1=points[0][y];
            const Point2f& p2=points[1][y];
            line(imsparse,Point(cvRound(p1.x),cvRound(p1.y)),Point(cvRound(p2.x),cvRound(p2.y)), color);
            circle(imsparse, Point(cvRound(p2.x), cvRound(p2.y)), 1, color, -1);
        }
    }
    LEAVE();
}*/

int getThreshold(int winnum) {
    if (winnum == 2 || winnum == 5 || winnum == 8 || winnum == 11) {
        return 200;
    }
    return 80;
}

bool isInsideWindow(Point2f point, Rect rect) {
	if(rect.x < point.x && point.x < rect.x+rect.width)
		if(rect.y < point.y && point.y < rect.y+rect.height) //filter points for this window
			return true;
	return false;
}

void download_p2f_of(const gpu::GpuMat& d_mat, vector<Point2f>& vec)
{
    vec.resize(d_mat.cols);
    Mat mat(1, d_mat.cols, CV_32FC2, (void*)&vec[0]);
    d_mat.download(mat);
}

void download_c_of(const gpu::GpuMat& d_mat, vector<uchar>& vec)
{
    vec.resize(d_mat.cols);
    Mat mat(1, d_mat.cols, CV_8UC1, (void*)&vec[0]);
    d_mat.download(mat);
}

void download_f_of(const gpu::GpuMat& d_mat, vector<float>& vec)
{
    vec.resize(d_mat.cols);
    Mat mat(1, d_mat.cols, CV_32FC1, (void*)&vec[0]);
    d_mat.download(mat);
}

void drawrect(Mat& prv, Mat& imsparse,Rect rect,vector<Point2f> *points,vector<uchar> &status,vector<float> err,int winnum)
{

	ENTER();
	static vector<Point2f> last_moving_features[RECTSTAT_SIZE];
	vector<Point2f> pred_points, interior_points, disp_points[2];
	static Mat last_moving_image[RECTSTAT_SIZE];
	static Mat max_moving_image[RECTSTAT_SIZE];
	vector<uchar> interior_status;
	vector<float> interior_err;
	static bool LAST_SET[RECTSTAT_SIZE] = {false};

	static bool change_anticipated[RECTSTAT_SIZE]={0};
	double flowx=0,flowy=0,flow=0, pred_flowx=0, pred_flowy=0, pred_flow=0, last_flow=0;
	int threshold = getThreshold(winnum);
	int num_detection_feats=0;


	//calculate flow
	for(unsigned int y=0;y<points[0].size();y++) {
		const Point2f& p1=points[0][y];
		const Point2f& p2=points[1][y];
		if(isInsideWindow(p1, rect)) { //filter points for this window
			if(int(status[y]) && err[y]<oferr) {
				interior_points.push_back(points[1][y]);
				num_detection_feats++;
				flowx=flowx+(p2.x-p1.x);
				flowy=flowy+(p2.y-p1.y);

				//flow=flow + (p2.x-p1.x)*(p2.x-p1.x) + (p2.y-p1.y)*(p2.y-p1.y);
			}
		}
	}
	// calculate total vector flow
	flow = flowx*flowx + flowy*flowy;
	//cout<<"window"<<winnum<<"flow "<<flow<<endl;
	if(flow > threshold && (rectstat[winnum] == 0 || num_detection_feats > 20) ) { //detection
		// moving objects in red rectangle
		rectstat[winnum]=1;
		rectangle(imsparse, rect, Scalar(0,0,255), 1, 8, 0);
		//last_moving_features[winnum] = interior_points;
		//last_moving_image[winnum] = prv;

		//if (flow > 4*threshold || !LAST_SET[winnum]) {
		//store feature points for tracking
		last_moving_features[winnum] = interior_points;
		//store image for tracking
		last_moving_image[winnum] = prv;
		//cout<<"last set"<<endl;
		//LAST_SET[winnum] = true;
		//}
	}
	else if( TRACKING && rectstat[winnum]==1) { //tracking moving object
		// calculating flow of previously detected features
        gpu::PyrLKOpticalFlow d_pyrLK;
        gpu::GpuMat d_points;
        gpu::GpuMat d_status;
        gpu::GpuMat d_err;
        gpu::GpuMat d_prv(prv);
        gpu::GpuMat d_last_moving_image(last_moving_image[winnum]);

        vector<Point2f> feats_2f = last_moving_features[winnum];

        Mat mat_temp(1, feats_2f.size(), CV_32FC2, (void*)&feats_2f[0]);
        gpu::GpuMat d_last_moving_features(mat_temp);

        d_pyrLK.sparse(
                d_last_moving_image, d_prv, 
                d_last_moving_features, d_points,
                d_status, &d_err);
        
        //download_p2f(feats_GpuMat, );
        download_p2f_of(d_points, pred_points);
        download_c_of(d_status, interior_status);
        download_f_of(d_err, interior_err);

		//calcOpticalFlowPyrLK(
        //    last_moving_image[winnum], prv,
        //    last_moving_features[winnum], pred_points,
        //    interior_status, interior_err);

		//number of features detected
		int num_feats = last_moving_features[winnum].size();

		/*for(int i = 0; i < last_moving_features[winnum].size(); i++) {
			printf("%d",interior_status[i]);	
		}*/
		//cout<<endl;
		//cout<<endl;
		//for(int i = 0; i < last_moving_features[winnum].size(); i++) {
		//    if(int(interior_status[i]) && interior_err[i]<oferr) {
		//	cout<<last_moving_features[winnum][i].x<<" ";	
		//	cout<<last_moving_features[winnum][i].y<<" ";	
		//	cout<<pred_points[i].x<<" ";	
		//	cout<<pred_points[i].y<<" ";	
		//	cout<<endl;
		//	}
		//}
		//cout<<endl;
		disp_points[0] = last_moving_features[winnum];
		disp_points[1] = pred_points;

#ifdef TRACKING_DEBUG
		drawoptflowsparse(last_moving_image[winnum], imsparse, imsparse, disp_points, interior_status, interior_err, CV_RGB(255, 255, 0));
#endif
		//imshow("SparseFlow_Seq", colim);
		flowx = 0; flowy = 0;
		vector<double> flow;
		int medianFlow = 0;
		int num_tracking_feats = 0;
		vector<Point2f> new_last_moving_features;
		for(unsigned int i=0; i < last_moving_features[winnum].size(); i++) {
			const Point2f& p1= last_moving_features[winnum][i];
			const Point2f& p2= pred_points[i];
			if(int(interior_status[i]) && interior_err[i]<50) {
				if(isInsideWindow(p2, rect)) {
					num_tracking_feats++;
					//cout<<__LINE__;
					//flowx=flowx+(p2.x-p1.x);
					//cout<<"flowx"<<flowx<<" ";
					//flowy=flowy+(p2.y-p1.y);
					//cout<<"flowy"<<flowy<<" ";
					//cout<<endl;
					flow.push_back((p2.x-p1.x)*(p2.x-p1.x) + (p2.y-p1.y)*(p2.y-p1.y));
					new_last_moving_features.push_back(p2);
				}
			}
		}
		last_moving_features[winnum] = new_last_moving_features; //follow tracking
		last_moving_image[winnum] = prv;
		float detection_rate = float(num_tracking_feats) / float(num_feats);
		//flow = flowx*flowx + flowy*flowy;
		//int normalized_flow = flow / num_tracking_feats;
        std::nth_element(flow.begin(), flow.begin() + flow.size()/2, flow.end());
        if(flow.size() > 0)
            medianFlow = flow[flow.size()/2];
		//cout<<"number last moving features:"<<last_moving_features[winnum].size()<<endl;
		//cout<<"flowx "<<flowx<<" flowy "<<flowy<<" interior flow "<<flow<<endl;
		//cout<<"no_of_feats "<<num_tracking_feats<<" normalized flow "<<medianFlow<<endl;
		if(medianFlow < 4*threshold && num_tracking_feats > 15) {
			// static objects in blue rectangle
			rectstat[winnum]=1;
			rectangle(imsparse, rect, Scalar(0,0,255), 1, 8, 0);
		}
		else {
			rectstat[winnum]=0;
			rectangle(imsparse, rect, Scalar(255,0,0), 1, 8, 0);
		}
	}
	else { //no detection or tracking
		rectstat[winnum]=0;
		rectangle(imsparse, rect, Scalar(255,0,0), 1, 8, 0);
	}
	LEAVE();
}

void findobst(Mat& prv, Mat& next,Mat& imsparse,vector<Point2f> *points,vector<uchar> &status,vector<float> err)
{
    ENTER();
    drawrect(prv, imsparse,rect1,points,status,err,1);
    drawrect(prv, imsparse,rect2,points,status,err,2);
    drawrect(prv, imsparse,rect3,points,status,err,3);

    drawrect(prv, imsparse,rect4,points,status,err,4);
    drawrect(prv, imsparse,rect5,points,status,err,5);
    drawrect(prv, imsparse,rect6,points,status,err,6);

    drawrect(prv, imsparse,rect7,points,status,err,7);
    drawrect(prv, imsparse,rect8,points,status,err,8);
    drawrect(prv, imsparse,rect9,points,status,err,9);

    drawrect(prv, imsparse,rect10,points,status,err,10);
    drawrect(prv, imsparse,rect11,points,status,err,11);
    drawrect(prv, imsparse,rect12,points,status,err,12);
    LEAVE();
}
/*
 Finds all the static objects in the frame
 */
void findstaticobst(Mat& prv, Mat& next,Mat& imsparse,vector<Point2f> *points,vector<uchar> &status,vector<float> err)
{
    ENTER();
    struct timespec zero, one, two, three, four;
    if (perf_measure) {
        clock_gettime(CLOCK_REALTIME, &zero);
    }
    vector<Point2f> pts[2];
    vector < vector<Point2i> > blobs;
    Mat binimg,cc,grady,gradx,grad;
    Mat abs_grad_x, abs_grad_y;
    Mat colnext=next.clone();
    cvtColor(next, next, CV_BGR2GRAY);
    Mat diff = abs(next-prv); //Difference of images as an indication of motion
    /*imshow("Previous Image",prv);
    imshow("Next Image",next);
    imshow("Difference Image",diff);*/
    //and an approximation of optical flow
    Sobel(next, gradx, CV_64F, 1, 0, 3);
    Sobel(next, grady, CV_64F, 0, 1, 3);
    convertScaleAbs( gradx, abs_grad_x );
    convertScaleAbs( grady, abs_grad_y );
    addWeighted( abs_grad_x, 0.5, abs_grad_y, 0.5, 0, grad );
    threshold(grad, binimg, 100, 1, THRESH_BINARY);
    imgerode(binimg, binimg, 1.2);
    imgdilate(binimg, binimg, 2);
  
    if (perf_measure) {
        clock_gettime(CLOCK_REALTIME, &one);
    }
    FindBlobs(binimg,cc, blobs);
    if (perf_measure) {
        clock_gettime(CLOCK_REALTIME, &two);
    }
    //cout<<"The number of blobs detected are "<<blobs.size()<<endl;
    for(unsigned int y=0;y<points[0].size();y++)
    {
        if(int(status[y]) && err[y]<oferr)
        {
            const Point2f& p1=points[0][y];
            const Point2f& p2=points[1][y];
            if(abs(p1.x-p2.x)<0.05 && abs(p1.y-p2.y)<0.05)
            {
                circle(imsparse, Point(cvRound(p2.x), cvRound(p2.y)), 1, CV_RGB(0, 255, 0), -1);
                pts[0].push_back(p2);
            }
        }       
    }
    if (perf_measure) {
        clock_gettime(CLOCK_REALTIME, &three);
    }
    if(pts[0].size()>0)
    {
        extractstaticwindowsrefined(colnext,diff,cc,imsparse, pts,blobs);
        //extractwindows(next,imsparse, pts);
    }
    if (perf_measure) {
        clock_gettime(CLOCK_REALTIME, &four);
        PERF("  -> time for diff & grident calc func: %0.2f ms\n", get_timediff(zero, one));
        PERF("  -> time for FindBlobs func: %0.2f ms\n", get_timediff(one, two));
        //PERF("  -> time for circle func: %0.2f ms\n", get_timediff(two, three));
        PERF("  -> time for extractstaticwindowsrefined func: %0.2f ms\n", get_timediff(three, four));

    }
    LEAVE();
}
/*
 Finds the static obstacles in the region of interests and plots it in green
 if found*/
void findstatobst(Mat& diff,Mat& next)
{
    ENTER();
    Mat featimg,binimg,blobimg,winimg;
    vector < vector<Point2i > > blobs;
    vector<KeyPoint> feats;
    vector<Point2f> points[2];
    Mat roi=next(rect2).clone();
    Mat diffroi=diff(rect2).clone();
    showorb(roi, featimg,feats);
    imshow("ORBIMAGE",featimg);
    points[0].clear();
    //cout<<"Found ORB features are "<<feats.size()<<endl;
    for(unsigned int i=0;i<feats.size();i++)
    {
        if(diffroi.at<double>(round(feats[i].pt.y),round(feats[i].pt.x)) == 0)
            points[0].push_back(Point2f(feats[i].pt.x,feats[i].pt.y));
    }
    /*
     75 Appears to be a good threshold
     */
    //cout<<"Refined the points to consider only the static ones "<<points[0].size()<<endl;
    threshold(next, binimg, 75, 1, THRESH_BINARY);
    //cout<<"About to find the blobs "<<binimg.size()<<endl;
    FindBlobs(binimg,blobimg, blobs);
    //cout<<"Found the blobs"<<endl;
    if(points[0].size()>0)
    {
        //cout<<roi.size()<<" "<<blobimg.size()<<"Blobs detected "<<blobs.size()<<endl;
        extractwindowsclose(next,blobimg,winimg,points,blobs);
        //imshow("BinaryImage",binimg);
        imshow("WindowsImage",winimg);
    }
    /*
     The sum of all the pixels in this roi should be zero for us to treat it as
     a static window ideally
     */
    LEAVE();
}
void findobstdense(const Mat& flow, Mat& cflowmap)
{
    ENTER();
    //Scalar(0,0,255) for red color
    //Scalar(255,0,0) for blue color
    double flowx=0,flowy=0;
    for(int i=rect1.x;i<rect1.width;i++)
    {
        for(int j=rect1.y;j<rect1.height;j++)
        {
            const Point2f& fxy = flow.at< Point2f>(j, i);
            flowx=flowx+fxy.x;
            flowy=flowy+fxy.y;
        }
    }
    cout<<flowx<<"x flow then now y flow"<<flowy<<endl;
    rectangle(cflowmap, rect1, Scalar(0,0,255), 1, 8, 0);
    rectangle(cflowmap, rect2, Scalar(255), 1, 8, 0);
    rectangle(cflowmap, rect3, Scalar(255), 1, 8, 0);
    rectangle(cflowmap, rect4, Scalar(255), 1, 8, 0);
    rectangle(cflowmap, rect5, Scalar(255), 1, 8, 0);
    rectangle(cflowmap, rect6, Scalar(255), 1, 8, 0);
    rectangle(cflowmap, rect7, Scalar(255), 1, 8, 0);
    rectangle(cflowmap, rect8, Scalar(255), 1, 8, 0);
    rectangle(cflowmap, rect9, Scalar(255), 1, 8, 0);
    rectangle(cflowmap, rect10, Scalar(255), 1, 8, 0);
    rectangle(cflowmap, rect11, Scalar(255), 1, 8, 0);
    rectangle(cflowmap, rect12, Scalar(255), 1, 8, 0);
    LEAVE();
}
