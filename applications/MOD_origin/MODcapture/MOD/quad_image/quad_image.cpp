#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <iostream>
using namespace cv;
using namespace std;
int main( int argc, char** argv )
{
    Mat img1 = imread("img1.png");
    Mat img2 = imread("img2.png");
    Mat img3 = imread("img3.png");
    Mat img4 = imread("img4.png");

    
    Mat top_img, bot_img;
    Mat img;
    hconcat(img1, img2, top_img);
    hconcat(img3, img4, bot_img);
    vconcat(top_img, bot_img, img);
    imshow("img", img);
    imwrite("img.png", img);
    waitKey(0);
    return 0;
}
