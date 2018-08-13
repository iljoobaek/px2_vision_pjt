/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   features.h
 * Author: KSH
 *
 * Created on December 30, 2016, 12:24 AM
 */

#ifndef FEATURES_H
#define FEATURES_H

using namespace cv;
using namespace std;

#ifdef __cplusplus
extern "C" {
#endif
    
/*
 * The below set of functions compute different feature points for a given image
 * and plot those locations on the image and return it in the form of out image.
 */
    
    
void showsift(Mat& img,
              Mat& out,
              vector<KeyPoint> &pts);

void showorb(Mat& img,
              Mat& out,
              vector<KeyPoint> &pts);

void showorb_cuda(Mat& img,
              Mat& out,
              vector<KeyPoint> &pts);

void showsurf(Mat& img,
              Mat& out,
              vector<KeyPoint> &pts);

void showfast(Mat& img,
              Mat& out,
              vector<KeyPoint> &pts);

void showhog(Mat& img,
              Mat& out,
              vector<float> &pts);

/*
 * For a given set of HOG feature points from an image, what this function does
 * is to compute the minimum difference with respect to the set of HOG features
 * computed during initialization from the set of road images.
 * What we are trying to do is, if the input HOG points were from a road, we
 * would ideally get a very good match with one of the HOG features and we would
 * be able to discard that window saying it belonged to a road.
 */

int comparehog(vector<float> &pts);



#ifdef __cplusplus
}
#endif



#endif /* FEATURES_H */

