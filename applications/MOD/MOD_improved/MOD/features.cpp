#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/nonfree/features2d.hpp>
#include <opencv2/gpu/gpu.hpp>
#include <iostream>
#include <vector>
#include <cmath>
#include <ctype.h>
#include "features.h"

using namespace cv;
using namespace std;

extern vector<Mat> HogFeats;//Declaring as extern to access it here.
#define roadthresh 1300//Less than this means it is road only

void showsift(Mat& img,Mat& out,vector<KeyPoint> &pts)
{
    SiftFeatureDetector detector(2000);//The input parameter specifies the number of features to compute
    out=img.clone();
    detector.detect(img, pts);
    drawKeypoints(img, pts, out);
}

void showorb2(Mat& img,Mat& out,vector<KeyPoint> &pts, int x_os, int y_os)
{
    OrbFeatureDetector detector(75);//The input parameter specifies the number of features to compute
    //OrbFeatureDetector detector(500);//The input parameter specifies the number of features to compute
    out=img.clone();
    detector.detect(img, pts);
    drawKeypoints(img, pts, out);
    if(x_os != 0 || y_os != 0)
    {
        for(int i=0;i<pts.size();i++) 
        {
            pts[i].pt.x += x_os;
            pts[i].pt.y += y_os;
        }
    }
}

void showorb(Mat& img,Mat& out,vector<KeyPoint> &pts)
{
    //GoodFeaturesToTrackDetector detector(200);//The input parameter specifies the number of features to compute
    OrbFeatureDetector detector(200);//The input parameter specifies the number of features to compute
    out=img.clone();
    detector.detect(img, pts);
    drawKeypoints(img, pts, out);
}

void showorb_cuda(Mat& img, Mat& out, vector<KeyPoint> &pts) {
    cv::gpu::GpuMat d_img(img);
    cv::gpu::GpuMat gpudsc;
    out=img.clone();
    cv::gpu::ORB_GPU orb(2000);

    orb(d_img, cv::gpu::GpuMat(), pts, gpudsc);

    //drawKeypoints(img, pts, out);
}

void showsurf(Mat& img,Mat& out,vector<KeyPoint> &pts)
{
    SurfFeatureDetector detector(2000);//The input parameter specifies the number of features to compute
    out=img.clone();
    detector.detect(img, pts);
    drawKeypoints(img, pts, out);
}

void showfast(Mat& img,Mat& out,vector<KeyPoint> &pts)
{
    FastFeatureDetector detector(20);//The input parameter specifies the number of features to compute
    out=img.clone();
    detector.detect(img, pts);
    drawKeypoints(img, pts, out);
}

void showhog(Mat& img,Mat& out,vector<float> &pts)
{
    HOGDescriptor hog(Size(96,64), Size(8,8), Size(4,4), Size(4,4), 9);
    resize(img, img, Size(96, 64));
    hog.compute(img,pts,Size(0,0), Size(0,0));
    Mat A(pts.size(),1,CV_32FC1);
    memcpy(A.data,pts.data(),pts.size()*sizeof(float));
    out=A.clone();
}

int comparehog(vector<float> &pts)
{
    Mat A(pts.size(),1,CV_32FC1);
    double diff;
    double count=10000000;
    for(int i=1;i<=200;i++)
    {
        Mat C = A-HogFeats[i-1];
        C = C.mul(C);
        sqrt(C, C);
        Scalar rr = sum(C);
        diff=rr(0);
        count=min(count,diff);
        if(count < roadthresh)
            return 0;
    }
    if(count > roadthresh)
        return 1;
    else
        return 0;
}
