#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

static int sockfd = 0;
void sock_init(){
    sockfd = socket(AF_INET , SOCK_STREAM , 0);

    if (sockfd == -1){
        printf("Fail to create a socket.");
    }

    struct sockaddr_in info;
    bzero(&info,sizeof(info));
    info.sin_family = PF_INET;

    //localhost test
    info.sin_addr.s_addr = inet_addr("127.0.0.1");
    info.sin_port = htons(8700);

    int err = connect(sockfd,(struct sockaddr *)&info,sizeof(info));
    if(err==-1){
        printf("Connection error");
    }
}

int sendBuf(unsigned char* buf, unsigned int len)
{
    int sent_len = 0;
#ifdef TIME_MEASURE
    struct timespec tpstart;
#endif
    if (sockfd == 0){
	    sock_init();
    }

    //Send the buf to server
#ifdef TIME_MEASURE
    clock_gettime(CLOCK_MONOTONIC, &tpstart);
    printf("%ld %ld\n", tpstart.tv_sec, tpstart.tv_nsec);
#endif
    sent_len = send(sockfd,buf,len,0);
    if (sent_len < len)
        printf("socket sends less data (%d) than expected (%d)\n", sent_len, len);

    //close(sockfd);
    //sockfd = 0;
    return 0;
}
