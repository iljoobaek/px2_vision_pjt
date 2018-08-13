#include <stdio.h>
#include <opencv2/opencv.hpp>
#include <fstream>
#include <iostream>

using namespace cv;
using namespace std;

int main(int argc, char** argv )
{
#if 0
    if ( argc != 2 )
    {
        printf("usage: DisplayImage.out <Image_Path>\n");
        return -1;
    }

    Mat image;
    image = imread( argv[1], 1 );

    if ( !image.data )
    {
        printf("No image data \n");
        return -1;
    }
    namedWindow("Display Image", WINDOW_AUTOSIZE );
    imshow("Display Image", image);

    waitKey(0);
#endif

#if 1
    Mat image(200, 200, CV_8UC3, Scalar(0));
    RotatedRect rRect = RotatedRect(Point2f(100,100), Size2f(100,50), 30);

    Point2f vertices[4];
    rRect.points(vertices);
    for (int i = 0; i < 4; i++)
        line(image, vertices[i], vertices[(i+1)%4], Scalar(0,255,0));

    Rect brect = rRect.boundingRect();
    rectangle(image, brect, Scalar(255,0,0));
    imshow("rectangles1", image);
    rectangle(image, brect, Scalar(0,0,255));
    imshow("rectangles2", image);
    rectangle(image, brect, Scalar(255,255,0));
    imshow("rectangles3", image);
    waitKey(0);
#endif

#if 0
    char str[10];

    //Creates an instance of ofstream, and opens example.txt
    ofstream a_file ( "example.txt" );
    // Outputs to example.txt through a_file
    a_file<<"This text will now be inside of example.txt";
    // Close the file stream explicitly
    a_file.close();
    //Opens for reading the file
    ifstream b_file ( "example.txt" );
    //Reads one string from the file
    b_file>> str;
    //Should output 'this'
    cout<< str <<"\n";
    cin.get();    // wait for a keypress
    // b_file is closed implicitly here
#endif
    return 0;
}
