/* This is the implementation of a sample task that demonstrates how to create a task. */

#include "sampleTask.h"

#define debugimage 1
#define TRACKING 1

//comment this to suppress tracking vectors in debugging output
#define TRACKING_DEBUG
#define RECTSTAT_SIZE (13)
#define oferr (100)
using namespace std;
using namespace task;
using namespace cv;

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
int rectstat[RECTSTAT_SIZE];

Mat colim;

/*The below function will draw the sparse optical flow points*/
void drawoptflowsparse(Mat& prv,Mat& next,Mat& imsparse,vector<Point2f> *points,vector<uchar> &status,vector<float> err, CvScalar color)
{
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

void drawrect(Mat& prv, Mat& imsparse,Rect rect,vector<Point2f> *points,vector<uchar> &status,vector<float> err,int winnum)
{
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
}

void findobst(Mat& prv, Mat& next,Mat& imsparse,vector<Point2f> *points,vector<uchar> &status,vector<float> err)
{
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
}
/**
 * @brief Creates an instance of the task, as required for Task control.
 *
 * @return A pointer to an instance of this task.
 *
 */
Task* Task::get(void)
{
    static SampleTask task(string("SampleTask"));
    return &task;
}

/**
 * @brief Default constructor of the Task, which sets defaults values.
 *
 */
SampleTask::SampleTask(const std::string taskName)
    :Task(taskName),
     outputInterface_(NULL),
     outputData_(),
     exampleParam_(0)
{
}

SampleTask::~SampleTask()
{
}

VideoCapture cap("/home/nvidia/Videos/mod_test.mp4");
Size S;
vector<Point2f> points[2];
vector<Point2f> h_points[2];
vector<uchar> status;
vector<float> err;
Mat prv, next, flow, colflow, imsparse, prvmul;
Mat binimg; //Used for storing the binary images
Mat featimg; //Used for getting the features
vector<KeyPoint> feats;
vector < vector<Point2i > > blobs;
unsigned int i=0;
int frame_id = 0;

OrbFeatureDetector detector(200);

void showorb(Mat& img, Mat& out, vector<KeyPoint>& pts)
{
    out=img.clone();
    detector.detect(img, pts);
    drawKeypoints(img, pts, out);
}

/**
 * @brief Initializes the Task to default readiness for operation.
 *
 * @return True for success, false for failure.
 *
 */
bool SampleTask::initialize(void)
{
    GET_WRITEONLY_INTERFACE(outputInterface_, SampleOutput, "SampleOutput", task::GetIntfFailOnErrors);

    taskParams_.get("ExampleParameter", exampleParam_, 1);

    outputData_.value=exampleParam_;


    S = Size((int) cap.get(CV_CAP_PROP_FRAME_WIDTH), (int) cap.get(CV_CAP_PROP_FRAME_HEIGHT));
    if(!(cap.read(prv)))
        return 0;
    cvtColor(prv, prv, CV_BGR2GRAY);

    return true;
}


/**
 * @brief Polls for received commands, and responds to them appropriately.
 *
 * @return True for a successful iteration, false for failure.
 *
 */
bool SampleTask::executive(void)
{
    //Mat img;
    //img = imread("/home/nvidia/drive-t186ref-linux/samples/nvmedia/MODcapture/cadillac.jpg", 1);
    //imshow("",img);
    //waitKey(1);
    cap.read(next);

    colim = next.clone();
    cvtColor(next, next, CV_BGR2GRAY);
    Mat diff = abs(next - prv);

    threshold(diff, binimg, 0, 255, THRESH_BINARY|THRESH_OTSU);

    prvmul=prv.mul(binimg);
    showorb(prvmul, featimg, feats);

    points[0].clear();
    points[1].clear();

    for( i = 0; i < feats.size(); i++ )
    {
        points[0].push_back(Point2f(feats[i].pt.x,feats[i].pt.y));
    }

    bool isSeqOpticalCalc = false;

    if(points[0].size() > 0)
    {
        isSeqOpticalCalc = true;
        calcOpticalFlowPyrLK(
                prv, next,
                points[0], points[1],
                status, err);

        drawoptflowsparse(prv, colim, imsparse, points, status, err, CV_RGB(0, 255, 0));
        findobst(prv, imsparse, imsparse, points, status, err);
    }

    double t_total_seq;
    if(points[0].size() > 0)
    {
        //string time = "frame: "+ to_string(frame_id) + " time (ms): " + to_string((int) t_total_seq);
        string time = "frame: time (ms): ";
        putText(imsparse, time, Point(30, 30), FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 255, 255), 2);
    
        if(debugimage)
            imshow("SparseFlow_Seq", imsparse);
    }

    if(debugimage)
    {
        char cKey = waitKey(3);
    }

    frame_id++;
    prv = next.clone();



    if (outputInterface_)
    {
		printf("write value: %d\n", outputData_.value);
        outputData_.value;
        outputInterface_->publish(outputData_);
        logger_.log_info("Writing %d", outputData_.value);
    }
    else
    {
		printf("interface error\n");
        logger_.log_fatal("The output interface is NULL.");
        return false;
    }
    return true;
}

/**
 * @brief Cleans up when task is told to terminate.
 *
 *
 */
void SampleTask::cleanup(void)
{
}
