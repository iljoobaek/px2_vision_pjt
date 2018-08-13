#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <cmath>
#include "opticalflow.h"
#include "morph.h"
#include "features.h"
#include "utils.h"
#include "log.h"

#define TRACKING 0

//comment this to suppress tracking vectors in debugging output
//#define TRACKING_DEBUG
#define SHOW_MEDIAN_FLOW 1

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

//Front Camera-> Top-left image
Rect rect1(20,  50, 185, 210);
Rect rect2(205, 50, 230, 260);
Rect rect3(435, 50, 185, 220);

//Right Camera-> Top-right image
Rect rect4(660, 50, 185, 120);
Rect rect5(845, 50, 230, 260);
Rect rect6(1075, 50, 185, 120);

//Rear Camera-> Bottom-left image
Rect rect7(20, 410, 185, 215);
Rect rect8(205, 410, 230, 290);
Rect rect9(435, 410, 185, 225);

//Left Camera-> Bottom-right image
Rect rect10(660, 410, 185, 180);
Rect rect11(845, 410, 230, 270);
Rect rect12(1075, 410, 185, 145);

static int log_level = LOG_INFO;
#define oferr 18
#define STATIC_NOISE_THOLD 0.15
#define CENTER_ROI_MAG_THOLD 3
#define CENTER_ROI_ANG_THOLD 7
#define SIDE_ROI_MAG_THOLD 0.75
#define SIDE_ROI_ANG_THOLD 30

struct flow_index {
    double mag; //magnitude
    Point start;  //starting point or previous frame feature
    Point end;  //ending point or current frame feature
};

extern int rectstat[RECTSTAT_SIZE];

extern Mat colim;

static int optFErr = oferr;

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
            //cout<< "The error in this track is "<< err[y] << endl;
            const Point2f& p1=points[0][y];
            const Point2f& p2=points[1][y];
            line(imsparse,Point(cvRound(p1.x),cvRound(p1.y)),Point(cvRound(p2.x),cvRound(p2.y)), color);
            circle(imsparse, Point(cvRound(p2.x), cvRound(p2.y)), 1, color, -1);
        }
    }
    LEAVE();
}

bool isCenter (int n)
{
    return (n == 2 || n == 5 || n == 8 || n == 11);
}

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

bool isTopROI(Point2f p1, Rect rect)
{
    return p1.y < (rect.y + rect.height / 2);
}

bool cmp_flow_index(struct flow_index a, struct flow_index b)
{
    return (a.mag < b.mag);
}

flow_index findMedianVector(Rect rect, Mat& prv, Mat& next, Mat& next_copy)
{
    Point2f median_start, median_end;
    vector <Point2f> pts[2];
    vector <uchar> global_status;
    vector <float> global_err;
    vector <flow_index> global_flows;
    int density = 20, bound = 0, count = 0;
    //cout << rect1.x << "\t" << rect1.y << "\t" << rect1.width << "\t" << rect1.height << endl;
    
    //cout << "rect.width: " << rect.width << "\t rect.height: " << rect.height << endl;
    for(float i = bound; i < rect.width - bound; i += density)
    {
        for(float j = bound * 2; j < rect.height - bound; j += density)
        {
            //cout << "i: " << i + rect.x << "\tj: " << j + rect.y << endl;
            pts[0].push_back(Point2f(i + rect.x,j + rect.y));
            count++;
        }
    }
    //cout << "rect.x: " << rect.x << "\trect.y: " << rect.y << "\trect.width: " << rect.width << "\trect.height: " << rect.height << endl;
    //cout << "pts[0].size(): " << pts[0].size() << "\tpts[1].size(): " << pts[1].size() << endl;
    calcOpticalFlowPyrLK(prv, next, pts[0], pts[1], global_status, global_err, Size(64,64), 2);
   
    CvScalar color = CV_RGB(0, 255, 0);
    for(int k = 0; k < count; k++)
    {
        if(int(global_status[k]) && global_err[k]<optFErr)
        {
            const Point2f& p1=pts[0][k];
            const Point2f& p2=pts[1][k];
            //cout << p1.x << "\t\t" << p1.y << "\t\t\t\t" << p2.x << "\t\t" << p2.y << endl;

            Point start = Point(cvRound(p1.x), cvRound(p1.y));
            Point end = Point(cvRound(p2.x), cvRound(p2.y));
           
            if(SHOW_MEDIAN_FLOW)
            {
                line(next_copy, start, end, color);
                circle(next_copy, end, 1, color, -1);
            }
            //cout << norm(start-end) << endl;
            global_flows.push_back({norm(start-end),start,end});
        }
    }
    
    int n = global_flows.size() / 2;
    std::nth_element(global_flows.begin(), global_flows.begin() + n, global_flows.end(), cmp_flow_index);
    if(n == 0)
        return {0, Point(0,0), Point(0,0)};
    return global_flows[n];
}

float drawrect(Mat& prv, Mat& next, Mat& next_copy, Mat& imsparse,Rect rect,vector<Point2f> *points,vector<uchar> &status,vector<float> err,int winnum,bool perform_median_calc)
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
	int threshold = getThreshold(winnum);   //200 for centers, 80 for sides
	int num_detection_feats=0;

    /* ***** GLOBAL MOTION ESTIMATION *****
     * Global motion estimation using optical flow for a given ROI. Uses a sparse
     * grid of points passed through sparse LK OF, taking the median value to estimate
     * the global motion vector for an ROI. */
    float median_mag_top, median_ang_top, median_mag_bot, median_ang_bot;
    Point2f median_start_top, median_end_top, median_start_bot, median_end_bot;
    
    //if(perform_median_calc || winnum == 2)
    if(perform_median_calc || winnum == 2)
    {
        optFErr = oferr;
        flow_index median_flow_top = findMedianVector(Rect(rect.x, rect.y, rect.width, rect.height / 2), prv, next, next_copy);
        median_start_top = median_flow_top.start;
        median_end_top = median_flow_top.end;
        median_mag_top = norm(median_start_top - median_end_top);
        median_ang_top = fastAtan2((median_start_top.y - median_end_top.y),(median_start_top.x - median_end_top.x));

        flow_index median_flow_bot = findMedianVector(Rect(rect.x, rect.y + rect.height / 2, rect.width, rect.height / 2), prv, next, next_copy);
        median_start_bot = median_flow_bot.start;
        median_end_bot = median_flow_bot.end;
        median_mag_bot = norm(median_start_bot - median_end_bot);
        median_ang_bot = fastAtan2((median_start_bot.y - median_end_bot.y),(median_start_bot.x - median_end_bot.x));
        //cout << "\t\tmedian_mag_top - median_mag_bot = " << median_mag_top - median_mag_bot << "\tmedian_ang_top - median_ang_bot = " << median_ang_top - median_ang_bot << endl;
    }
    else
    {
        median_mag_top = 0;
        median_ang_top = 0;
        median_start_top = Point(0,0);
        median_end_top = Point(0,0);
        
        median_mag_top = 0;
        median_ang_top = 0;
        median_start_bot = Point(0,0);
        median_end_bot = Point(0,0);
        optFErr = 100;
    }

    //cout << "median_mag: " << median_mag << "\t median_ang: " << median_ang << endl;
    if(SHOW_MEDIAN_FLOW)
    {
        line(next_copy, median_start_top, median_end_top, CV_RGB(255, 0, 0), 2);
        circle(next_copy, median_end_top, 2, CV_RGB(255, 0, 0), -1);
    }
    /* ***** END GLOBAL MOTION ESTIMATION *****
     * median_start -> starting median point
     * median_end -> ending median point 
     */


	/* ***** FLOW CALCULATION *****
     */
    if(perform_median_calc || (winnum == 2 && median_mag_top > 0.25))
    {
        for(size_t y=0; y<points[0].size(); y++) 
        {
            const Point2f& p1=points[0][y];
            const Point2f& p2=points[1][y];
            //filter points for this window 
            if(isInsideWindow(p2, rect) && (status[y]) && err[y]<optFErr)
            {
                //cout << "SMALL ERROR: " << err[y] << "------------------------------" << endl;
                float point_mag = norm(p1-p2);
                float point_ang = fastAtan2((p1.y - p1.y), (p1.x - p2.x));
                //cout << "diff_mag = " << abs(point_mag - median_mag) << endl;
                //cout << "diff_ang = " << abs(point_ang - median_ang) << endl;

                // Remove random static (especially when stationary)
                if(abs(point_mag) < STATIC_NOISE_THOLD)
                {
                    line(next_copy,Point(cvRound(p1.x),cvRound(p1.y)),Point(cvRound(p2.x),cvRound(p2.y)), Scalar(255,0,0));
                    circle(next_copy, Point(cvRound(p2.x), cvRound(p2.y)), 5, Scalar(255,0,0), -1);
                }

                if(isTopROI(p1, rect))
                {
                    // Point is in a center ROI; larger magnitude difference, smaller angle difference
                    if(isCenter(winnum) && (abs(point_mag - median_mag_top) < CENTER_ROI_MAG_THOLD) && (abs(point_ang - median_ang_top)) < CENTER_ROI_ANG_THOLD)
                    {
                        //line(next_copy,Point(cvRound(p1.x),cvRound(p1.y)),Point(cvRound(p2.x),cvRound(p2.y)), Scalar(255,0,0));
                        circle(next_copy, Point(cvRound(p2.x), cvRound(p2.y)), 5, Scalar(255,0,0), -1);
                    }
                    

                    // Point is in a side ROI; smaller magnitude differrence, larger angle difference
                    else if((abs(point_mag - median_mag_top) < SIDE_ROI_MAG_THOLD) && (abs(point_ang - median_ang_top)) < SIDE_ROI_ANG_THOLD)
                    {
                        //line(next_copy,Point(cvRound(p1.x),cvRound(p1.y)),Point(cvRound(p2.x),cvRound(p2.y)), Scalar(255,0,0));
                        circle(next_copy, Point(cvRound(p2.x), cvRound(p2.y)), 5, Scalar(255,0,0), -1);
                    }

                    else if((winnum == 7) && (abs(point_mag - median_mag_top) < 4) && (abs(point_ang - median_ang_top)) < 45)
                    {
                        //line(next_copy,Point(cvRound(p1.x),cvRound(p1.y)),Point(cvRound(p2.x),cvRound(p2.y)), Scalar(0,0,255));
                        circle(next_copy, Point(cvRound(p2.x), cvRound(p2.y)), 5, Scalar(255,0,0), -1);
                    }
                }

                else
                {
                    if(isCenter(winnum) && (abs(point_mag - median_mag_bot) < CENTER_ROI_MAG_THOLD) && (abs(point_ang - median_ang_bot)) < CENTER_ROI_ANG_THOLD)
                    {
                        //line(next_copy,Point(cvRound(p1.x),cvRound(p1.y)),Point(cvRound(p2.x),cvRound(p2.y)), Scalar(255,0,0));
                        circle(next_copy, Point(cvRound(p2.x), cvRound(p2.y)), 5, Scalar(0,0,255), -1);
                    }
                    

                    // Point is in a side ROI; smaller magnitude differrence, larger angle difference
                    else if((abs(point_mag - median_mag_bot) < SIDE_ROI_MAG_THOLD) && (abs(point_ang - median_ang_bot)) < SIDE_ROI_ANG_THOLD)
                    {
                        //line(next_copy,Point(cvRound(p1.x),cvRound(p1.y)),Point(cvRound(p2.x),cvRound(p2.y)), Scalar(255,0,0));
                        circle(next_copy, Point(cvRound(p2.x), cvRound(p2.y)), 5, Scalar(0,0,255), -1);
                    }

                    else if((winnum == 7) && (abs(point_mag - median_mag_bot) < 4) && (abs(point_ang - median_ang_bot)) < 45)
                    {
                        //line(next_copy,Point(cvRound(p1.x),cvRound(p1.y)),Point(cvRound(p2.x),cvRound(p2.y)), Scalar(0,0,255));
                        circle(next_copy, Point(cvRound(p2.x), cvRound(p2.y)), 5, Scalar(0,0,255), -1);
                    }
                }
                

                //else (otherwise increment num_detection_feats, include point in interior_points, increase flow, and draw line + circle)
                //TODO: add else
                interior_points.push_back(points[1][y]);
                num_detection_feats++;
                flowx=flowx+(p2.x-p1.x);
                flowy=flowy+(p2.y-p1.y);
                line(next_copy,Point(cvRound(p1.x),cvRound(p1.y)),Point(cvRound(p2.x),cvRound(p2.y)), Scalar(0,255,0));
                circle(next_copy, Point(cvRound(p2.x), cvRound(p2.y)), 1, Scalar(0,255,0), -1);
            }
            else if(isInsideWindow(p2, rect) && (status[y]) && (err[y] < 100))
            {
                const Point2f& p1=points[0][y];
                const Point2f& p2=points[1][y];
                cout << "LARGE ERROR: " << err[y] << "+++++++++++++++++++++++++++++" << endl;
                //cout << "\tLENGTH: " << norm(p1-p2) << endl;
                line(next_copy,Point(cvRound(p1.x),cvRound(p1.y)),Point(cvRound(p2.x),cvRound(p2.y)), Scalar(255,0,255));
                circle(next_copy, Point(cvRound(p2.x), cvRound(p2.y)), 5, Scalar(255,0,255), -1);
            }
        }
    }
    else
    {
        for(unsigned int y=0;y<points[0].size();y++)
        {
            const Point2f& p1=points[0][y];
            const Point2f& p2=points[1][y];
            if(isInsideWindow(p1, rect)) 
            { //filter points for this window
                if(int(status[y]) && err[y]<optFErr) 
                {
                    interior_points.push_back(points[1][y]);
                    num_detection_feats++;
                    flowx=flowx+(p2.x-p1.x);
                    flowy=flowy+(p2.y-p1.y);
                    line(next_copy,Point(cvRound(p1.x),cvRound(p1.y)),Point(cvRound(p2.x),cvRound(p2.y)), Scalar(0,255,0));
                    circle(next_copy, Point(cvRound(p2.x), cvRound(p2.y)), 1, Scalar(0,255,0), -1);

                }
            }
	    }   
    }
	// calculate total vector flow
	flow = flowx*flowx + flowy*flowy;
	//cout<<"window"<<winnum<<"flow "<<flow<<endl;
    /* ***** END FLOW CALCULATION *****
     */


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
		calcOpticalFlowPyrLK(
            last_moving_image[winnum], prv,
            last_moving_features[winnum], pred_points,
            interior_status, interior_err);
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
		

        /* MODIFICATION. Uses nth_element to find median rather than sorting */
        /*std::sort(flow.begin(), flow.end());
		if (flow.size() > 0)
			medianFlow = flow[flow.size()/2];*/
        std::nth_element(flow.begin(), flow.begin() + flow.size()/2, flow.end());
		if (flow.size() > 0)
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
    //cout << "\t\t\t\t\tmedian_mag: " << median_mag << "\tmedian_ang: " << median_ang << endl;
    return median_mag_bot;
	LEAVE();
}

void findobst(Mat& prv, Mat& next, Mat& colnext, Mat& imsparse,vector<Point2f> *points,vector<uchar> &status,vector<float> err)
{
    ENTER();
    //imshow("colnext", colnext);
    //waitKey(0);
    float median_flow_magnitude = 0;
    bool perform_median_calc = true;
    median_flow_magnitude += drawrect(prv, next, colnext, imsparse,rect2,points,status,err,2,false);
    
    // Static camera; this check and calculation on the first camera can be replaced once car speed information is known
    if(median_flow_magnitude < 0.25)
    {
        perform_median_calc = false;
        cout << "%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% static camera %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%" << endl;
    }
    //cout << median_flow_magnitude << endl;
    
    drawrect(prv, next, colnext, imsparse,rect1,points,status,err,1,perform_median_calc);
    //drawrect(prv, next, colnext, imsparse,rect2,points,status,err,2,perform_median_calc);
    drawrect(prv, next, colnext, imsparse,rect3,points,status,err,3,perform_median_calc);

    drawrect(prv, next, colnext, imsparse,rect4,points,status,err,4,perform_median_calc);
    drawrect(prv, next, colnext, imsparse,rect5,points,status,err,5,perform_median_calc);
    drawrect(prv, next, colnext, imsparse,rect6,points,status,err,6,perform_median_calc);
    drawrect(prv, next, colnext, imsparse,rect7,points,status,err,7,perform_median_calc);
    drawrect(prv, next, colnext, imsparse,rect8,points,status,err,8,perform_median_calc);
    drawrect(prv, next, colnext, imsparse,rect9,points,status,err,9,perform_median_calc);
    drawrect(prv, next, colnext, imsparse,rect10,points,status,err,10,perform_median_calc);
    drawrect(prv, next, colnext, imsparse,rect11,points,status,err,11,perform_median_calc);
    drawrect(prv, next, colnext, imsparse,rect12,points,status,err,12,perform_median_calc);
    
    if(SHOW_MEDIAN_FLOW)
    {
        imshow("colnext", colnext);
        waitKey(0);
    }
    
    //imshow("colnext", colnext);
    //waitKey(0);

    
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
