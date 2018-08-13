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
extern "C" {
#include "eglconsumer_main.h"
}
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

#define savevideo 1     //set this to one if we want to write the output to a video file
#define saveimage 0     //set this to one if we want to write the output to a image file
#define samplefreq 1   //the freq of doing sampling

#define debugimage 1
#define stop_early 0
#define stop_early_iterations 1000

#define HEIGHT_NEW 720
#define WIDTH_NEW 1280
#define HEIGHT 604
#define WIDTH 960

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

struct timespec tpstart;
struct timespec tpend;
float diff_sec, diff_nsec;
int num_f;
float sum_time, max_time;

struct timespec tp_start_10;
struct timespec tp_now_10;
int frame_ctr;
float fps;
float total_nsec;


vector<Mat> HogFeats;//Global variable for storing the computed HOG features of 200 road images

//extern Rect Rect2;//In this window we have to detect the static object as well.

/*
 * We will compute the HoG descriptors for all the set of road images once during
 * the initialization itself.
 */

//Converts the Mat from px2 into the appropriate form
void cvt_mod(Mat &prv_temp, Mat &prv)
{
    // The 3rd parameter here scales the data by 1/16 so that it fits in 8 bits.
    if (prv_temp.empty()){
        cout << "prv_temp empty" << endl;
    }
    
	cvtColor(prv_temp, prv, CV_RGBA2BGR);
    resize(prv, prv, Size(2560,360));
}

gpu::GpuMat colim;

void download_p2f(const gpu::GpuMat& d_mat, vector<Point2f>& vec)
{
    vec.resize(d_mat.cols);
    Mat mat(1, d_mat.cols, CV_32FC2, (void*)&vec[0]);
    d_mat.download(mat);
}

void download_c(const gpu::GpuMat& d_mat, vector<uchar>& vec)
{
    vec.resize(d_mat.cols);
    Mat mat(1, d_mat.cols, CV_8UC1, (void*)&vec[0]);
    d_mat.download(mat);
}

void download_f(const gpu::GpuMat& d_mat, vector<float>& vec)
{
    vec.resize(d_mat.cols);
    Mat mat(1, d_mat.cols, CV_32FC1, (void*)&vec[0]);
    d_mat.download(mat);
}

int main( int argc, char** argv )
{
    cout << "# CUDA-enabled devices: " << gpu::getCudaEnabledDeviceCount() << endl;
    gpu::setDevice(0);
    cout << "Current device in use: " << gpu::getDevice << endl;
    num_f = 0;
    sum_time = 0;
    max_time = 0;
    const int MAX_COUNT = 100;
    vector<Point2f> points[2];
    vector<Point2f> h_points[2];
    vector<uchar> status;
    vector<float> err;
    Accuracy *acc;
    // host variables
    gpu::GpuMat prv, next, flow, colflow, prvmul;
    Mat imsparse;
    gpu::GpuMat featimg; //Used for getting the features
    vector<KeyPoint> feats;
    vector < vector<Point2i > > blobs;
    unsigned int i=0;
    int frame_id = 0;


    /*Initializing our system first*/
    cout<<"Started"<<endl;


	if (accuracy_measure) {
		acc = new Accuracy("result/accuracy_no_tracking.txt");
	}



    //VideoCapture cap("data/mod_test.mp4");
    //VideoCapture cap("data/urban2.mp4");
        //Size S = Size((int) cap.get(CV_CAP_PROP_FRAME_WIDTH),    
        //        (int) cap.get(CV_CAP_PROP_FRAME_HEIGHT));//Input Size

    //Mat prv_cap(HEIGHT_NEW, WIDTH_NEW, CV_8UC1);
    //Mat next_cap(HEIGHT_NEW, WIDTH_NEW, CV_8UC1);
    Mat matInputBuffer(HEIGHT,WIDTH,CV_8UC4);
    unsigned char* inputBuffer = matInputBuffer.data;
    Mat matInputBuffer1(HEIGHT,WIDTH,CV_8UC4);
    unsigned char* inputBuffer1 = matInputBuffer1.data;
    Mat matInputBuffer2(HEIGHT,WIDTH,CV_8UC4);
    unsigned char* inputBuffer2 = matInputBuffer2.data;
    Mat matInputBuffer3(HEIGHT,WIDTH,CV_8UC4);
    unsigned char* inputBuffer3 = matInputBuffer3.data;
    eglconsumer_main();

    if(savevideo) {

        //Size S = Size(1280, 720);//Input Size
        /*We will save the video here, change the name accordingly*/
        //outputVideo_seq.open("result/outputVideo_tracking_2018-02-09.avi", CV_FOURCC('M','J','P','G'), cap.get(CV_CAP_PROP_FPS), S, true);
        /*if (!outputVideo_seq.isOpened()) {
            cout  << "Could not open the output video for write: "  << endl;
            return -1;
        }*/
    }

    sleep(1);
    gpu::GpuMat prv_temp, prv_temp_full;
    getImgBuffer(&inputBuffer, &inputBuffer1, &inputBuffer2, &inputBuffer3);
    vector <Mat> matrices = {matInputBuffer, matInputBuffer1, matInputBuffer2, matInputBuffer3};
    Mat allBuffer;
    hconcat(matrices, allBuffer);
    gpu::GpuMat prv_temp_cap(allBuffer);
    //prv_temp_cap.upload(allBuffer);
    gpu::cvtColor(prv_temp_cap, prv_temp_full,CV_RGBA2BGR); 
    //cvt_mod(matInputBuffer,prv_cap);
    gpu::resize(prv_temp_full, prv_temp, Size(2560,360));
    //gpu::GpuMat prv_temp;
    //prv_temp.upload(prv_cap);

    gpu::cvtColor(prv_temp, prv, CV_BGR2GRAY);
    //gpu::threshold(prv, binimg, 100, 0, THRESH_BINARY);
    //imshow("binimg", Mat(binimg));
    //waitKey(0);
    

    //Computing the good features to track at the beginning
    // FIXME 

    gpu::GpuMat next_temp, next_temp_full, next_temp_cap;
    gpu::GpuMat diff, sub_diff;
    gpu::GpuMat binimg, desc, mask;
    gpu::GpuMat feats_GpuMat, d_points, d_status, d_err;
    Mat binimg_mat, prv_t, colim_t;

    clock_gettime(CLOCK_MONOTONIC, &tp_start_10);
    while(true)
    {
        frame_ctr++;
        if(frame_ctr == 10)
        {
            clock_gettime(CLOCK_MONOTONIC, &tp_now_10);
            diff_sec = tp_now_10.tv_sec - tp_start_10.tv_sec;
            diff_nsec = tp_now_10.tv_nsec - tp_start_10.tv_nsec;
            total_nsec = diff_sec * 1000000000 + diff_nsec;
            fps = 10 / (total_nsec / 1000000000);
            cout << "FPS: " << fps << endl;
            frame_ctr = 0;
            clock_gettime(CLOCK_MONOTONIC, &tp_start_10);
        }
        if(stop_early && frame_id == stop_early_iterations)
            break;


        //gpu::GpuMat next_temp, next_temp_full;

        getImgBuffer(&inputBuffer, &inputBuffer1, &inputBuffer2, &inputBuffer3);

        vector <Mat> matrices = {matInputBuffer, matInputBuffer1, matInputBuffer2, matInputBuffer3};

        Mat allBuffer;
        hconcat(matrices, allBuffer);

        //imshow("", Mat(allBuffer));
        //waitKey(0);
        //gpu::GpuMat next_temp_cap(allBuffer);
        next_temp_cap.upload(allBuffer);
        
        /* ***** START TIMING ***** */
        clock_gettime(CLOCK_MONOTONIC, &tpstart);
        //printf("%ld %ld\n", tpstart.tv_sec, tpstart.tv_nsec);
        /* ***** START TIMING ***** */

        gpu::cvtColor(next_temp_cap, next_temp_full,CV_RGBA2BGR); 

        /* ***** END TIMING ***** */ 
        clock_gettime(CLOCK_MONOTONIC, &tpend);
        //printf("%ld %ld\n", tpend.tv_sec, tpend.tv_nsec);
		/*std::sort(flow.begin(), flow.end());
		if (flow.size() > 0)
			medianFlow = flow[flow.size()/2];
        */
        diff_sec = tpend.tv_sec - tpstart.tv_sec;
        diff_nsec = tpend.tv_nsec - tpstart.tv_nsec; 
        total_nsec = diff_sec * 1000000000 + diff_nsec;
        if(frame_id > 50)
        {
            if(total_nsec > max_time)
                max_time = total_nsec;
            sum_time += total_nsec;
            num_f++;
        }
        /* ***** END TIMING ***** */
        
        gpu::resize(next_temp_full, next_temp, Size(2560,360));
        if(next_temp.empty())
            break;

        if(frame_id == 500)
            imwrite("500_e2e.png", Mat(next_temp));
        if(frame_id == 200)
            imwrite("200_e2e.png", Mat(next_temp));

        //Mat imsparse_t;


	//outputVideo_orig.write(next);

        // if not the frame should be processed, then skip
        if (frame_id % samplefreq != 0 || frame_id < 0) {
            prv_temp = next_temp.clone();
            gpu::cvtColor(prv_temp, prv, CV_BGR2GRAY);
            frame_id++;
            continue;
        }
	printf("\n\n=============%d=============\n", frame_id);
	
	//cin.get();

        /*Storing the color version of the image for future use*/
        colim = next_temp.clone();
        gpu::cvtColor(next_temp, next, CV_BGR2GRAY);
        /*Difference between the previous frame and the current frame*/
        //Mat diff;
        //gpu::GpuMat diff, sub_diff;
        gpu::subtract(next, prv, sub_diff);
        gpu::abs(sub_diff, diff);

        /*Thresholding the diff image to obtain a binary image*/
        //Mat binimg_mat;

        threshold(Mat(diff), binimg_mat, 0, 255, THRESH_BINARY|THRESH_OTSU);
        //gpu::GpuMat binimg(binimg_mat); //Used for storing the binary images
        binimg.upload(binimg_mat);        

        //gpu::GpuMat diff_save, binimg_save;

        //cvtColor(diff, diff_save, CV_GRAY2BGR);
        //cvtColor(binimg, binimg_save, CV_GRAY2BGR);

        //outputVideo_seq_diff.write(diff_save);
        //outputVideo_seq_thresh.write(binimg_save);
        //imshow("threshold", binimg);
        if(debugon)
            cout<< "Calculated Binary image"<<endl;
        //prvmul=prv.mul(binimg);
        gpu::multiply(prv, binimg, prvmul);

        if(debugon)
            cout<<"Completed element wise multiplication"<<endl;
        //showorb_cuda_gpu(prvmul, featimg, feats);//Computing features on this images.
        gpu::ORB_GPU orb(200);
        //gpu::GpuMat desc;
        //gpu::GpuMat mask;
        orb(prvmul,mask,feats);
        
        vector<Point2f> feats_2f;
        KeyPoint kp;
        kp.convert(feats, feats_2f);
        
        Mat mat_temp(1, feats_2f.size(), CV_32FC2, (void*)&feats_2f[0]);
        //cout << "mat_temp: " << mat_temp.size() << "\t mat_temp: " << mat_temp.type() << endl;

        //cout << feats_2f.size() << endl;

        //gpu::GpuMat feats_GpuMat(mat_temp);
        feats_GpuMat.upload(mat_temp);

        //orb.downloadKeyPoints(feats_GpuMat, feats);

        //cout << feats_GpuMat.type() << endl;
        //cout << "feats_GpuMat: " << feats_GpuMat.size() << endl;
        //cout << feats.size() << endl;

        /*Clearing out the points to store new points*/
        points[0].clear();
        points[1].clear();
        /* points[0] will now contain the locations at which the ORB key point 
           descriptors were identified*/
        for(i=0;i<feats.size();i++) {
            points[0].push_back(Point2f(feats[i].pt.x,feats[i].pt.y));
        }
        
        //if(debugon) {
        //    cout<<"Computed ORB features"<<prv.size()<<" "<<next.size()<<endl;
        //    cout<<points[0].size()<<" "<<points[1].size()<<endl;
        //    cout<<"The size of the images passed are"<<prv.size()<<" "<<next.size()<<endl;
        //}

        bool isSeqOpticalCalc = false;

        //cout << points[0].size() << endl;

        //if (feats_GpuMat.size() > 0) {
        if (1) {
            isSeqOpticalCalc = true;
            gpu::PyrLKOpticalFlow d_pyrLK;
            //gpu::GpuMat d_points;
            //gpu::GpuMat d_status;
            //gpu::GpuMat d_err;
            d_pyrLK.sparse(
                    prv, next, 
                    feats_GpuMat, d_points,
                    d_status, &d_err);
            prv.download(prv_t);
            colim.download(colim_t);
            //calcOpticalFlowPyrLK(prv_t, Mat(next), points[0], points[1], status, err);


            if(debugon)
                cout<<"Computed sparse optical flow features"<<endl;


            /*
            d_points.download(points);
            d_status.download(status);
            d_err.download(err);
            */
            download_p2f(feats_GpuMat, points[0]);
            download_p2f(d_points, points[1]);
            download_c(d_status, status);
            download_f(d_err, err);

            drawoptflowsparse(prv_t, colim_t, imsparse, points, status, err, CV_RGB(0, 255, 0));
            findobst(prv_t, imsparse,imsparse,points,status,err);

            if (accuracy_measure) {
                    acc->fillFrameState(frame_id, rectstat);
            }


            //findstatobst(diff,next);
            if(debugon)
                cout<<"Located obstacles"<<endl;
        }
        double t_total_seq = total_nsec / 1000000;
        if (points[0].size() > 0) {
            string time = "frame: "+ to_string(frame_id) + " time (ms): " + to_string((int)t_total_seq);
            putText(imsparse, time, Point(30, 30), FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 255, 255), 2);

            if(debugimage)
                imshow("SparseFlow_Seq",Mat(imsparse));

            /* Saving the results in the form of a video*/
            //if(savevideo)
                //outputVideo_seq.write(imsparse);
        }

        //FIXME add static object detector here
	    //gpu::GpuMat view;


        if(debugon)
            cout<<"Calculated Binary image"<<endl;
        if(debugon)
            cout<<"Completed element wise multiplication"<<endl;

        
        if(debugimage)
        {
            char cKey = waitKey(2);
            if ( cKey == 27) // ESC
                break;
        }
        /*else if (cKey == 32) // Space
        {
            while(1)
            {
                if(waitKey(33) >= 0)
                    break;
            }
        }*/
   
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
    free(inputBuffer);
    return 0;
}
