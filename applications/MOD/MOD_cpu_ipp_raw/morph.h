/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   morph.h
 * Author: KSH
 *
 * Created on January 2, 2017, 9:24 AM
 */

#ifndef MORPH_H
#define MORPH_H

using namespace cv;
using namespace std;

#ifdef __cplusplus
extern "C" {
#endif
    /*
     Wrapper to erode the input image by taking erosion_size as input parameter
     * by default it uses an ellipse window 
     */
    void imgerode(Mat& img,
                  Mat& out,
                  int erosion_size);
    
    /*
    *Wrapper to dilate the input image by taking erosion_size as input parameter
     * by default it uses an ellipse window.
     */
    void imgdilate(Mat& img,
                   Mat& out,
                   int erosion_size);
    
    
    /*
     Given a set of points, this function draws windows around these points of
     size 96x64 and returns them in the output image.
     */
    void extractwindows(Mat& img,Mat& out,vector<Point2f> *points);
    
    /*
     This is essentially a connected components algorithm. Given an input binary
     image, it returns a matrix labels which, at every pixel has the component 
     label to which that particular pixel belongs to. It also returns blobs
     which contains the pixels belonging to a particular component i at blobs[i]
     */
    void FindBlobs(const Mat &binary,Mat &labels, vector < vector<Point2i> > &blobs);
    
    /*
     Given the blobs in the image identified with the FindBlobs() function
     along with the interest points, we extract the rectangle windows around
     every point and merge the windows belonging to the same blob by using the
     union and filtering out the false positives by passing the HOG feature 
     points to  the comparehog() function
     */
    void extractwindowsrefined(Mat& img,Mat& blob, Mat& out, vector<Point2f> *points, vector < vector<Point2i> > &blobs);
    
    /*
     Given the blobs in the image identified with the FindBlobs() function
     along with the interest points, we extract the rectangle windows around
     every point and merge the windows belonging to the same blob by using the
     union and filtering out the false positives by passing the HOG feature 
     points to  the comparehog() function.
     Also, we want to identify only static objects in the image hence, for the
     windows which pass the HOG test, we compute the motion by summing the pixel
     in the diff image and if it is greater than a threshold, we discard it.
     */
    void extractstaticwindowsrefined(Mat& img,Mat& diff,Mat& blob, Mat& out, vector<Point2f> *points, vector < vector<Point2i> > &blobs);
    
    /*
     We don;t use this function in our pipeline but this is similar to  
     extractwindowsrefined() function 
     */
    void extractwindowsclose(Mat& img,Mat &blob, Mat& out, vector<Point2f> *points, vector < vector<Point2i> > &blobs);
    
#ifdef __cplusplus
}
#endif



#endif /* MORPH_H */

