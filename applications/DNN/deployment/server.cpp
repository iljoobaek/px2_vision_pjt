#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <opencv2/opencv.hpp>
#include "opencv2/video/tracking.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/video/tracking.hpp"
#include <opencv2/opencv.hpp>
#include "opencv2/video/tracking.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/video/tracking.hpp"
#include <opencv2/opencv.hpp>

#define HEIGHT 1208
//#define WIDTH 3840 //2 aggregate
#define WIDTH 7680

using namespace std;

static int sockfd = 0;
static int forClientSockfd = 0;
static struct sockaddr_in serverInfo, clientInfo;
static socklen_t addrlen;

extern "C" int getImgBuffer(unsigned char* inputBuffer, unsigned int width, unsigned int height)
{
    FILE * pFile;
    if (sockfd == 0){
    	sockfd = socket(AF_INET , SOCK_STREAM , 0);

    	if (sockfd == -1){
        	printf("Fail to create a socket.");
    	}

    	addrlen = sizeof(clientInfo);
    	bzero(&serverInfo,sizeof(serverInfo));

    	serverInfo.sin_family = PF_INET;
    	serverInfo.sin_addr.s_addr = INADDR_ANY;
    	serverInfo.sin_port = htons(8700);
    	bind(sockfd,(struct sockaddr *)&serverInfo,sizeof(serverInfo));
    	listen(sockfd,200);
        forClientSockfd = accept(sockfd,(struct sockaddr*) &clientInfo, &addrlen);
    }

    //cout << "Finished connecting commands" << endl;

    //while(1){
        //printf("Get one 123\n");
        //memset(inputBuffer, 0, 4746240);
        int recv_len = recv(forClientSockfd,inputBuffer,4746240 * 2,MSG_WAITALL);//width*height,0);
    //cout << "Finished recv" << endl;
        //struct timespec tpstart;
        //clock_gettime(CLOCK_MONOTONIC,&tpstart);
        //printf("get buffer %ld %ld\n", tpstart.tv_sec, tpstart.tv_nsec);
        
        //printf("%d\n", recv_len);
/*
        pFile = fopen ("test" , "w");
        fwrite(inputBuffer, 1, 4746240, pFile);
        fclose(pFile);
*/
        //printf("get %d\n");
        //printf("Get one 234\n");

/*
	cv::Mat Image;
    cv::Mat bayer16BitMat(HEIGHT, WIDTH, CV_16UC1, inputBuffer);//two memory copy?
    //if (bayer16BitMat.empty()){
    //    printf("empty image\n");
    //}
    //else {
    //    imwrite("test.png", bayer16BitMat);
    //}
	 //Convert thfe Bayer data from 16-bit to to 8-bit
    cv::Mat bayer8BitMat;
    //cv::Mat bayer8BitMat = bayer16BitMat.clone();
	// The 3rd parameter here scales the data by 1/16 so that it fits in 8 bits.
	//bayer8BitMat.convertTo(bayer8BitMat, CV_8UC1,0.015625);
	bayer16BitMat.convertTo(bayer8BitMat, CV_8UC1,0.0625);
	// Convert the Bayer data to 8-bit RGB
	//cv::Mat rgb8BitMat(1208, 1920, CV_8UC1);
	cv::cvtColor(bayer8BitMat, Image, CV_BayerGR2RGB);
    cv::resize(Image, Image, cv::Size(640, 360));
	//cv::resize(rgb8BitMat, Image,  cv::Size(1920,1208));
	cv::imshow( "imageName", Image); 
    //clock_gettime(CLOCK_MONOTONIC,&tpstart);
    //printf("imshow %ld %ld\n", tpstart.tv_sec, tpstart.tv_nsec);
	cv::waitKey(1);
	//
*/
        //printf("Get one\n");
        //close(forClientSockfd);
    //}
    return 0;
}


