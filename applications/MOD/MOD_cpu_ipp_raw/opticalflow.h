/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   opticalflow.h
 * Author: KSH
 *
 * Created on December 29, 2016, 2:18 AM
 */

#ifndef OPTICALFLOW_H
#define OPTICALFLOW_H

using namespace cv;
using namespace std;

#ifdef __cplusplus
extern "C" {
#endif

extern struct timespec tpstart;
extern struct timespec tpend;
extern int num_f;
extern float diff_sec, diff_nsec, sum_time, max_time;

/*
 Draws the dense optical flow vectors on the image
 */
void drawOptFlowMap (const Mat& flow, Mat& cflowmap, int step, const Scalar& color);

/*
 Draws the sparse optical flow vectors on the image
 */
void drawoptflowsparse(Mat& prv,Mat& next,Mat& imsparse,vector<Point2f> *points,vector<uchar> &status,vector<float> err, CvScalar color);

/*
 This function is used to identify moving obstacles in the pre-defined regions 
 of interest.
 This simple computes the sum of the optical flow values of the points falling 
 inside the ROI and says that there is a moving obstacle present if the 
 magnitude of the resulting sum is greater than a certain threshold. However,
 only those optical flow values are considered which have returned a less
 error.
 This in turn calls drawrect() function where all the computations happped
 */
void findobst(Mat& flow, Mat& cflowmap,Mat& imsparse,vector<Point2f> *points,vector<uchar> &status,vector<float> err);

/*
 This function is not used in our pipeline
 */
void findstatobst(Mat& diff,Mat& next);

/*
 This function is used to determine if there is a static object in the 
 pre-defined region of interest.
 For a given optical flow values, this selects only those points whose optical
 flow values is less than a particular threshold and pass it to the 
 extractstaticwindowsrefined() function where all of the further computations
 happen.
 */
void findstaticobst(Mat& flow, Mat& cflowmap,Mat& imsparse,vector<Point2f> *points,vector<uchar> &status,vector<float> err);




#ifdef __cplusplus
}
#endif

#endif /* OPTICALFLOW_H */

