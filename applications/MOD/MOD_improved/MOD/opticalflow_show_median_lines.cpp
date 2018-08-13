#include <opencv2/opencv.hpp>
#include <opencv2/gpu/gpu.hpp>
#include "opencv2/video/tracking.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/video/tracking.hpp"
#include <opencv2/nonfree/features2d.hpp>

#include <iostream>
#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <ctype.h>
#include "opticalflow.h"
#include "features.h"
#include "morph.h"
#include "utils.h"
#include "log.h"
#include <unistd.h>

#include <time.h>

using namespace cv;
using namespace std;

static int log_level = LOG_ERR;
#define dense 2         //set this to one if we want to compute dense correspondences
#define calcfeat 1      //set this to one if we want to calculate features in every frame
#define debugon 0       //Sets some print statements enabling debug
#define enableseq    1
#define correctcheck 0  // must set enableseq to 1
// select one of the below two
#define imagepreprocess 0 // pre-precess the video to images
#define imageprocess 0  // set this to 1 if we want to process from images
#define videoprocess 1  // set this to 1 if you want to process directly from video 

#define savevideo 0     //set this to one if we want to write the output to a video file
#define saveimage 0     //set this to one if we want to write the output to a image file
#define samplefreq 1   //the freq of doing sampling

#define debugimage 0

extern int rectstat[RECTSTAT_SIZE];
extern Rect rect1;
extern Rect rect2;
extern Rect rect3;
extern Rect rect4;
extern Rect rect5;
extern Rect rect6;
extern Rect rect7;
extern Rect rect8;
extern Rect rect9;
extern Rect rect10;
extern Rect rect11;
extern Rect rect12;

vector<Mat> HogFeats;//Global variable for storing the computed HOG features of 200 road images

//extern Rect Rect2;//In this window we have to detect the static object as well.

/*
 * We will compute the HoG descriptors for all the set of road images once during
 * the initialization itself.
 */


Mat colim, colnext;
struct timespec tpstart;
struct timespec tpend;
float diff_sec, diff_nsec;
int num_f;
float sum_time, max_time;

int main( int argc, char** argv )
{
    const int MAX_COUNT = 100;
    vector<Point2f> points[2];
    vector<Point2f> h_points[2];
    vector<uchar> status;
    vector<float> err;
    Accuracy *acc;
    // host variables
    Mat prv, next, flow, colflow, imsparse, prvmul;
    Mat binimg; //Used for storing the binary images
    Mat featimg; //Used for getting the features
    vector<KeyPoint> feats;
    vector < vector<Point2i> > blobs;
    unsigned int i=0;
    int frame_id = 0;


    /*Initializing our system first*/
    cout<<"Started"<<endl;
    //VideoWriter outputVideo_orig;
    //VideoWriter outputVideo_seq;
    //VideoWriter outputVideo_seq_thresh;
    //VideoWriter outputVideo_seq_diff;
    VideoWriter outputVideo_background_blobs;

    if (accuracy_measure) {
    	acc = new Accuracy("result/accuracy_no_tracking.txt");
    }

    //VideoCapture cap("data/mod_test.mp4");
    //VideoCapture cap("data/urban1.mp4");
    //VideoCapture cap("data/forbes_pull_up2.mp4");
    //VideoCapture cap("data/urban_end_moving.mp4");
    VideoCapture cap("data/forbes_877.mp4");
    //VideoCapture cap("data/urban_begin_moving.mp4");
    cap.read(prv);
    //VideoCapture cap1("prv");

    Size S = Size((int) cap.get(CV_CAP_PROP_FRAME_WIDTH), (int) cap.get(CV_CAP_PROP_FRAME_HEIGHT));//Input Size
    //outputVideo_seq_thresh.open("result/outputVideo_thresh_after.avi", CV_FOURCC('M','J','P','G'), cap.get(CV_CAP_PROP_FPS), S, true);
    //outputVideo_seq_diff.open("result/outputVideo_diff.avi", CV_FOURCC('M','J','P','G'), cap.get(CV_CAP_PROP_FPS), S, true);
    //outputVideo_orig.open("result/outputVideo_orig.avi", CV_FOURCC('M','J','P','G'), cap.get(CV_CAP_PROP_FPS), S, true);
    outputVideo_background_blobs.open("result/background_blobs.avi", CV_FOURCC('M','J','P','G'), cap.get(CV_CAP_PROP_FPS), S, true);
    //if(savevideo) {

        //Size S = Size(1280, 720);//Input Size
        /*We will save the video here, change the name accordingly*/
        //outputVideo_seq.open("result/outputVideo_tracking_2018-02-09.avi", CV_FOURCC('M','J','P','G'), cap.get(CV_CAP_PROP_FPS), S, true);
        //if (!outputVideo_seq.isOpened()) {
        //    cout  << "Could not open the output video for write: "  << endl;
        //    return -1;
        //}
    //}


    //processing start   
    if(!(cap.read(prv)))
        return 0;//returns if the video is over

    /*Gray Scale conversion*/
    /*Gray Scale to Binary Image conversion*/
    //imshow("prv", prv);    
    //imshow("binary", binimg);
    //imshow("h_prv", h_prv);
    cvtColor(prv, prv, CV_BGR2GRAY);
    threshold(prv, binimg, 100, 0, THRESH_BINARY);

    //Computing the good features to track at the beginning
    // FIXME 

    while(true)
    {
        if(!(cap.read(next)))
            break;
        if(next.empty())
            break;

	    //outputVideo_orig.write(next);

        // if not the frame should be processed, then skip
        if (frame_id % samplefreq != 0 || frame_id < 0) {
            prv = next.clone();
            cvtColor(prv, prv, CV_BGR2GRAY);
            frame_id++;
            continue;
        }
	    printf("\n\n=============%d=============\n", frame_id);
	
        /*Storing the color version of the image for future use*/
        colim = next.clone();
        colnext = next.clone();
        cvtColor(next, next, CV_BGR2GRAY);
        

        /* ***** START TIMING ***** */
        clock_gettime(CLOCK_MONOTONIC, &tpstart);
        //printf("%ld %ld\n", tpstart.tv_sec, tpstart.tv_nsec);
        /* ***** START TIMING ***** */
        
        /* ***** END TIMING ***** */ 
        clock_gettime(CLOCK_MONOTONIC, &tpend);
        //printf("%ld %ld\n", tpend.tv_sec, tpend.tv_nsec);

        diff_sec = tpend.tv_sec - tpstart.tv_sec;
        diff_nsec = tpend.tv_nsec - tpstart.tv_nsec; 
        float total_nsec = diff_sec * 1000000000 + diff_nsec;
        if(total_nsec > max_time)
            max_time = total_nsec;
        sum_time += total_nsec;
        num_f++;
        /* ***** END TIMING ***** */






	    /***** Background subtraction step. First take difference *****/
	    /* Difference between the previous frame and the current frame */
        Mat diff = abs(next - prv); 
        /*Thresholding the diff image to obtain a binary image. Then we take a binary threshold w/ OTSU threshold */
        threshold(diff, binimg, 0, 255, THRESH_BINARY|THRESH_OTSU);
	
	    Mat diff_save, binimg_save;

        cvtColor(diff, diff_save, CV_GRAY2BGR);		//saved img for the binary background subtraction
        cvtColor(binimg, binimg_save, CV_GRAY2BGR);	//saved img for after the thresholding step

        //outputVideo_seq_diff.write(diff_save);
        //outputVideo_seq_thresh.write(binimg_save);

        if(debugon)
            cout<< "Calculated Binary image"<<endl;

        prvmul=prv.mul(binimg);	//multiplies prv (gray) img element-wise by the binary background subtraction img
        if(debugon)
            cout<<"Completed element wise multiplication"<<endl;
	    /***** End of background subtraction step *****/


	    /**** Features extraction step. First use ORB feature extraction, storing in feats *****/
        showorb(prvmul, featimg, feats);//Computing features on this images.
        
	    /*Clearing out the points to store new points*/
        points[0].clear();
        points[1].clear();

        /* points[0] will now contain the locations at which the ORB key point 
           descriptors were identified*/
        for(i=0;i<feats.size();i++) 
        {
            points[0].push_back(Point2f(feats[i].pt.x,feats[i].pt.y));
        }

        if(debugon) {
            cout<<"Computed ORB features"<<prv.size()<<" "<<next.size()<<endl;
            cout<<points[0].size()<<" "<<points[1].size()<<endl;
            cout<<"The size of the images passed are"<<prv.size()<<" "<<next.size()<<endl;
        }

        bool isSeqOpticalCalc = false;

	    /***** Perform object tracking using Optical Flow (Pyramid LK) *****/
        if (points[0].size() > 0) 
	    {
            isSeqOpticalCalc = true;
            calcOpticalFlowPyrLK(
                    prv, next, 
                    points[0], points[1],
                    status, err, Size(64,64), 2);//, TermCriteria(TermCriteria::COUNT+TermCriteria::EPS, 30, 0.01), 8, 0.01);
                    //additional params: Size winSize, int maxLevel
            if(debugon)
                cout<<"Computed sparse optical flow features"<<endl;

            /* Draws the optical flow with the drawoptflowsparse function */
            drawoptflowsparse(prv, colim, imsparse, points, status, err, CV_RGB(0, 255, 0));

            //imshow("imsparse", imsparse);
            findobst(prv,next,colnext,imsparse,points,status,err);
            outputVideo_background_blobs.write(colnext);
            if (accuracy_measure) {
                    acc->fillFrameState(frame_id, rectstat);
        }

        //findstatobst(diff,next);
        if(debugon)
            cout<<"Located obstacles"<<endl;
        }
        double t_total_seq;
        if (points[0].size() > 0) {
            string time = "frame: "+ to_string(frame_id) + " time (ms): " + to_string((int)t_total_seq);
            putText(imsparse, time, Point(30, 30), FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 255, 255), 2);
            if(debugimage)
                imshow("SparseFlow_Seq",imsparse);
            /* Saving the results in the form of a video*/
            //if(savevideo)
                //outputVideo_seq.write(imsparse);
        }

        //FIXME add static object detector here
	    Mat view;

        if(debugon)
            cout<<"Calculated Binary image"<<endl;
        if(debugon)
            cout<<"Completed element wise multiplication"<<endl;

        char cKey = waitKey(3);
        if ( cKey == 27) // ESC
            break;
        else if (cKey == 32) // Space
        {
            while(1)
            {
                if(waitKey(33) >= 0)
                    break;
            }
        }
   
        // update the frame_id for accuracy analysis
        frame_id++;
        /* Current frame is stored into the previous frame and the new frame is
           read in the loop*/
        prv = next.clone();
        PERF("\n");

    } //end of while (true)
    if (accuracy_measure) {
            acc->generateAccuracyReport();
    }
    cout << sum_time << " ns \t" << num_f << " frames \t" << sum_time / num_f  << " ns \t" << sum_time / num_f / 1000000 << " ms" << endl;
    cout << max_time << " ns \t" << max_time / 1000000 << " ms" << endl;


    return 0;
}
