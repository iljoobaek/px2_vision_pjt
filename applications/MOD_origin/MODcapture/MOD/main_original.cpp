#include <opencv2/opencv.hpp>
#include <opencv2/gpu/gpu.hpp>
#include "opencv2/video/tracking.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/video/tracking.hpp"
#include <opencv2/nonfree/features2d.hpp>

#include <iostream>
#include <ctype.h>
#include "opticalflow.h"
#include "features.h"
#include "morph.h"
#include "utils.h"
#include "log.h"

using namespace cv;
using namespace std;
//using namespace cv::gpu;

static int log_level = LOG_INFO;
#define dense 2         //set this to one if we want to compute dense correspondences
#define savevideo 0     //set this to one if we want to write the output to a video file
#define calcfeat 1      //set this to one if we want to calculate features in every frame
#define debugon 1       //Sets some print statements enabling debug

extern int rectstat[RECTSTAT_SIZE];
vector<Mat> HogFeats;//Global variable for storing the computed HOG features of 200 road images

//extern Rect Rect2;//In this window we have to detect the static object as well.
/*
 * We will compute the HoG descriptors for all the set of road images once during
 * the initialization itself.
 */
void init()
{
    ENTER();
    cout<<"Started Initialization"<<endl;
    Mat src1;
    Mat gray1;
    vector<float> ders1;
    for(int i=1;i<=200;i++)
    {
        stringstream  num1;
        num1<<i;
        string str1 = string("data/Roads/") +num1.str()+ ".png";
        src1 = imread(str1);
        cv::HOGDescriptor hog(Size(96,64), Size(8,8), Size(4,4), Size(4,4), 9);
        cvtColor(src1, gray1, CV_BGR2GRAY);
        resize(gray1, gray1, Size(96, 64));
        hog.compute(gray1,ders1,Size(0,0), Size(0,0));
        Mat A(ders1.size(),1,CV_32FC1);
        memcpy(A.data,ders1.data(),ders1.size()*sizeof(float));
        HogFeats.push_back(A);
        //HogFeats[i]=A.clone();   
    }
    LEAVE();
}

int main( int argc, char** argv )
{
    const int MAX_COUNT = 5000;
    vector<Point2f> points[2];
    vector<uchar> status;
    vector<float> err;
    // host variables
    Mat prv, next, flow, colflow, imsparse, colim, diff, prvmul;
    // corresponding device variables
    gpu::GpuMat d_prv, d_next, d_flow, d_colflow, d_imsparse, d_colim, d_diff, d_prvmul;
    Mat binimg; //Used for storing the binary images
    Mat featimg; //Used for getting the features
    gpu::GpuMat d_binimg;
    gpu::GpuMat d_featimg;
    vector<KeyPoint> feats;
    vector < vector<Point2i > > blobs;
    unsigned int i=0;
    int frame_id = 0;
    Accuracy *acc_seq;
   
    int deviceCount = gpu::getCudaEnabledDeviceCount();
    if (!deviceCount) {
        INFO("GPU features are not supported for this platform!\n");
    } else {
        INFO("GPU features are supported for this platform!\n");
    }

    if (accuracy_measure) {
        acc_seq = new Accuracy("accuracy_seq.txt");
    }
    /*Initializing our system first*/
    init();
    cout<<"Started"<<endl;
    //VideoCapture cap("data/20150422155330.mp4");
    VideoCapture cap("data/20170630_test.mp4");
    Size S = Size((int) cap.get(CV_CAP_PROP_FRAME_WIDTH),    
            (int) cap.get(CV_CAP_PROP_FRAME_HEIGHT));//Input Size
    VideoWriter outputVideo;
    //int ex = static_cast<int>(cap.get(CV_CAP_PROP_FOURCC));
    if(savevideo)
    {
        /*We will save the video here, change the name accordingly*/
        if(dense)
            outputVideo.open("Track3.avi" , CV_FOURCC('M','J','P','G'), cap.get(CV_CAP_PROP_FPS),S, true);
        else
            outputVideo.open("StatMov5.avi" , 1, cap.get(CV_CAP_PROP_FPS),S, true);
        //Reading the first frame from the video into the previous frame
        if (!outputVideo.isOpened())
        {
            cout  << "Could not open the output video for write: "  << endl;
            return -1;
        }
    }
    if(!(cap.read(prv)))
        return 0;//returns if the video is over
    /*Gray Scale conversion*/
    cvtColor(prv, prv, CV_BGR2GRAY);
    /*Gray Scale to Binary Image conversion*/
    threshold(prv, binimg, 100, 0, THRESH_BINARY);
    //imshow("prv", prv);    
    //imshow("binary", binimg);
    //Computing the good features to track at the beginning 
    goodFeaturesToTrack(prv, points[0], MAX_COUNT, 0.01, 10, Mat(), 3, 0, 0.04);

    while(true)
    {
        struct timespec one, two, three, four, five, six, seven, eight, nine, ten, eleven, twelve, thirteen;
        if (perf_measure) {
            clock_gettime(CLOCK_REALTIME, &one);
        }

        /*Reading the next frame and returns if there is nothing to be read*/
        if(!(cap.read(next)))
            break;
        if(next.empty())
            break;
        /*Storing the color version of the image for future use*/
        colim=next.clone();
        cvtColor(next, next, CV_BGR2GRAY);
        /*Difference between the previous frame and the current frame*/
        diff=abs(next-prv);
        //imshow("Difference",diff);
        /*
         *By default dense will be set to 2 and this should not be changed
         */
#if dense==0        
        threshold(diff, binimg, 50, 1, THRESH_BINARY);//Will have very low
        //threshold because already it is difference of images
        //imshow("BinaryImage", binimg);
        Scharr(diff, gradnext, CV_64F, 0, 1, 3);
        imshow("GradientImage",gradnext);
        //threshold(gradnext, binimg, 50, 255, THRESH_BINARY);//Will have very low
        //imgerode(binimg, binimg, 1);
        imgdilate(binimg, binimg, 2);
        //imgerode(binimg, binimg, 4);
        imshow("MorphedImage",binimg);
        FindBlobs(binimg,blobimg, blobs);
        cout<<"The number of blobs detected in this frame is"<<blobs.size()<<endl;
        /*We will now calculate some features useful for identifying the obstacles*/
        if(calcfeat)
        {
            //showorb(next, featimg,feats);
            showorb(diff, featimg,feats);//Computing features on the diff of images.
            points[0].clear();
            for(i=0;i<feats.size();i++)
            {
                points[0].push_back(Point2f(feats[i].pt.x,feats[i].pt.y));
            }
            cout << points[0].size() << endl;
            /*Done calculating the features*/
        }
        //blobs.clear();
        extractwindowsrefined(next,blobimg,winimg,points,blobs);
        //extractwindows(next,winimg,points);
        imshow("WindowsImage",winimg);

        calcOpticalFlowPyrLK(
                prv, next, // 2 consecutive images
                points[0], // input point positions in first im
                points[1], // output point positions in the 2nd
                status,    // tracking success
                err      // tracking error
                );
        //cout<<"Optical Flow computed for one frame"<<endl;
        drawoptflowsparse(prv,colim,imsparse,points);
        imshow("SparseFlow",imsparse);
        outputVideo.write(winimg);
        if (waitKey(5) >= 0) 
            break;
        swap(points[1], points[0]);
#endif

        //The dense flow calculation takes a lot of time since it matches every
        //pixel in one image to other image.
#if dense==1
        calcOpticalFlowFarneback(prv, next, flow, 0.5, 1, 5, 3, 5, 1.2, 0);
        cvtColor(prv, colflow, CV_GRAY2BGR);
        drawOptFlowMap(flow, colflow, 20, CV_RGB(0, 255, 0));
        cout<<colflow.size()<<endl;      
        findobst(flow, colflow,colflow,points,status,err);
        imshow("DenseFlow",colflow);
        outputVideo.write(colflow);
        if (waitKey(5) >= 0) 
            break;
#endif
#if dense==2
        /*Thresholding the diff image to obtain a binary image*/
        threshold(diff, binimg, 200, 255, THRESH_BINARY);
        //imshow("Binary Image",binimg);
        if(debugon)
            cout<<"Calculated Binary image"<<endl;
        if (perf_measure) {
            clock_gettime(CLOCK_REALTIME, &two);
        }

        /*Doing element wise multiplication, the idea behind this is that the 
          interest points would get concentrated around the moving backgrounds
          which is desired ideally. This way, I was also able to eliminate a lot
          of unnecessary interest points on the roads too*/
        //imshow("mul: ", prv);
        prvmul=prv.mul(binimg);
        //imshow("prvmul: ", prvmul);
        /*
         *debugon is just a flag to print some statements to debug if something
         * isn't working as expected. Further print statements could also be
         * added under this flag.
         */
        if(debugon)
            cout<<"Completed element wise multiplication"<<endl;
        if (perf_measure) {
            clock_gettime(CLOCK_REALTIME, &three);
        }

        //imshow("PreviousImage",prv);
        showorb(prvmul, featimg, feats);//Computing features on this images.
        /*Clearing out the points to store new points*/
        points[0].clear();
        points[1].clear();
        /*
           points[0] will now contain the locations at which the ORB key point 
           descriptors were identified
           */
        for(i=0;i<feats.size();i++)
        {
            points[0].push_back(Point2f(feats[i].pt.x,feats[i].pt.y));
        }
        if(debugon)
        {
            cout<<"Computed ORB features"<<prv.size()<<" "<<next.size()<<endl;
            cout<<points[0].size()<<" "<<points[1].size()<<endl;
            cout<<"The size of the images passed are"<<prv.size()<<" "<<next.size()<<endl;
        }
        if (perf_measure) {
            clock_gettime(CLOCK_REALTIME, &four);
        }

        /*
           Proceed only if at least one ORB descriptor was identified, if not
           skip this frame.
           */
        if(points[0].size()>0)
        {
            /*
             *We are now calculating the optical flow values only at the 
             locations where the ORB key points were found. This way we need
             not compute it everywhere thus reducing the computational 
             complexity drastically.
             The corresponding points in the next frame are stored in 
             the points[1] array.
             *  */
            calcOpticalFlowPyrLK(
                    prv, next, // 2 consecutive images
                    points[0], // input point positions in first im
                    points[1], // output point positions in the 2nd
                    status,    // tracking success
                    err      // tracking error
                    );
            if(debugon)
                cout<<"Computed sparse optical flow features"<<endl;
            /*
             *We will now draw the optical flow values on the frame and store
             *it in a new image called imsparse
             */
            drawoptflowsparse(prv,colim,imsparse,points,status,err);
            /*
               We will now call the findobst function which returns the 
               imsparse image with regions of interest in either red color or
               blue color. Red indicates moving obstacle found in that region of
               interest whereas blue indicates no moving obstacle was found in 
               that region of interest.
               */
            if (perf_measure) {
                clock_gettime(CLOCK_REALTIME, &seven);
            }
            findobst(prv, imsparse,imsparse,points,status,err);
            if (accuracy_measure) {
                acc_seq->fillFrameState(frame_id, rectstat);
            }
            if (perf_measure) {
                clock_gettime(CLOCK_REALTIME, &eight);
            }
            //findstatobst(diff,next);
            imshow("SparseFlow",imsparse);
            if(debugon)
                cout<<"Located obstacles"<<endl;

            if(debugon)
                cout<<"Wrote into the video"<<endl;
            //swap(points[1], points[0]);
        }
        if (perf_measure) {
            clock_gettime(CLOCK_REALTIME, &five);
        }

#if 0
        /*
           We will now detect the static objects in the images
           */
        showorb(next, featimg,feats);//Computing features on the current frame.
        if (perf_measure) {
            clock_gettime(CLOCK_REALTIME, &thirteen);
        }
        /*
           Clearing the points
           */
        points[0].clear();
        points[1].clear();
        /*
           Storing the locations of the ORB key point descriptors in the points[0]
           array
           */
        //INFO("feats size: %d\n", feats.size()); // feats size 2000
        for(i=0;i<feats.size();i++)
        {
            points[0].push_back(Point2f(feats[i].pt.x,feats[i].pt.y));
        }

        if(points[0].size()>0)
        {
            /*Computing optical flow on these points*/
            if (perf_measure) {
                clock_gettime(CLOCK_REALTIME, &eleven);
            }
            calcOpticalFlowPyrLK(
                    prv, next, // 2 consecutive images
                    points[0], // input point positions in first im
                    points[1], // output point positions in the 2nd
                    status,    // tracking success
                    err      // tracking error
                    );
            if (perf_measure) {
                clock_gettime(CLOCK_REALTIME, &twelve);
            }
            if(debugon)
                cout<<"Computed sparse optical flow features"<<endl;
            //drawoptflowsparse(prv,colim,imsparse,points,status,err);
            /*
               Calling the find static obstacle function which return a red 
               window if only moving obstacle is detected in that ROI, yellow 
               window if only static object is detected in that ROI, cyan 
               window if both static and moving objects are detected in that 
               ROI */
            if (perf_measure) {
                clock_gettime(CLOCK_REALTIME, &nine);
            }
            findstaticobst(prv, colim,imsparse,points,status,err);
            if (perf_measure) {
                clock_gettime(CLOCK_REALTIME, &ten); 
            }
            imshow("StaticObjects",imsparse);
            if(debugon)
                cout<<"Located obstacles"<<endl;
            if(debugon)
                cout<<"Wrote into the video"<<endl;
            //swap(points[1], points[0]);
        }
        if (perf_measure) {
            clock_gettime(CLOCK_REALTIME, &six);
        }
#endif

        /* Saving the results in the form of a video*/
        if(savevideo)
            outputVideo.write(imsparse);

        char cKey = waitKey(5);
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
#endif
        /* Current frame is stored into the previous frame and the new frame is
           read in the loop*/
        prv = next.clone();

        if (perf_measure) {
            double t_diff_binary = get_timediff(one, two);
            double t_wise_mul = get_timediff(two, three);
            double t_ORB = get_timediff(three, four);
            double t_moving_obj = get_timediff(four, five);
            double t_total_moving_obj = t_diff_binary + t_wise_mul + t_ORB + t_moving_obj;
            
            double t_total = t_total_moving_obj;
            PERF("time for diff and binary image: %0.2f ms, per: %0.2f%%\n", t_diff_binary, t_diff_binary/t_total*100);
            PERF("time for wise mul: %0.2f ms, per: %0.2f%%\n", t_wise_mul, t_wise_mul/t_total*100);
            PERF("time for ORB: %0.2f ms, per: %0.2f%%\n", t_ORB, t_ORB/t_total*100);
            PERF("time for find moving obj: %0.2f ms, per: %0.2f%%\n", t_moving_obj, t_moving_obj/t_total*100);
#if 0 
            double t_static_obj = get_timediff(five, six);
            double t_findobst = get_timediff(seven, eight);
            double t_findstaticobst = get_timediff(nine, ten);
            double t_calc_opticalflow = get_timediff(eleven, twelve);
            double t_showorb = get_timediff(five, thirteen);
            double t_total_static_obj = t_static_obj + t_findobst + t_findstaticobst + t_calc_opticalflow + t_showorb;
            
            PERF("  -> time for findobst func: %0.2f ms, per: %0.2f%%\n", t_findobst, t_findobst/t_total*100);
            PERF("time for find static obj: %0.2f ms, per: %0.2f%%\n", t_static_obj, t_static_obj/t_total*100);
            PERF("  -> time for findstaticobst func: %0.2f ms, per: %0.2f%%\n", t_findstaticobst, t_findstaticobst/t_total*100);
            PERF("  -> time for calcOpticalFlowPyrLK func: %0.2f ms, per: %0.2f%%\n", t_calc_opticalflow, t_calc_opticalflow/t_total*100);
            PERF("  -> time for showorb func: %0.2f ms, per: %0.2f%%\n\n", t_showorb,  t_showorb/t_total*100);
#endif      
            //FIXME
            PERF("total time for one frame: %.2f ms\n\n", t_total);
        }
        // update the frame_id for accuracy analysis
        frame_id++;
    } //end of while (true)
    if (accuracy_measure) {
        acc_seq->generateReport();
        //FIXME
        if (!acc_seq->isSimiliarAccuracy(acc_seq)) {
            ERR("not accuracy enough for cuda version!\n");
        } else {
            INFO("accuracy of cuda version is within the threshold 1%%\n");
        }
    }

    return 0;
}
