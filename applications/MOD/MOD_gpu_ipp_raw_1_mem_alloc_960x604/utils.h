#ifndef __UTILS_H__
#define __UTILS_H__

#include <ctime>
#include <fstream>
#include <opencv2/opencv.hpp>
#include <opencv2/gpu/gpu.hpp>

using namespace cv;
using namespace cv::gpu;

#define RECTSTAT_SIZE       (13)
//set this to 1 if we want to enable performance measure
#define perf_measure        (0)
// set this to 1 if we want to enable accuracy measure
#define accuracy_measure    (0)
// error allowed between seq and cuda 
#define seq_cuda_err        (0.001)   

void download(const GpuMat& d_mat, vector<Point2f>& vec);
void download(const GpuMat& d_mat, vector<uchar>& vec);
void download(const GpuMat& d_mat, vector<float>& vec);
void upload(vector<Point2f>& vec, GpuMat& d_mat);
bool isEqual(const Mat &a, const Mat &b, const char *msg);
bool isEqual(const vector<Point2f> &a, const vector<Point2f> &b, const char *msg);
bool isEqual(const vector<uchar> &a, const vector<uchar> &b, const char *msg);
bool isEqual(const vector<float> &a, const vector<float> &b, const char *msg);

double get_timediff(timespec &ts1, timespec &ts2);

class Accuracy {
    private:
        // a map used to record the state of 12 rectangles
        // key - the frame id
        // value - the lower 12 bits to represent the state of the 12 rectangles
        std::vector<std::pair<unsigned int, unsigned int> > map;
        std::ofstream ofs;
        int threshold;
    public:
        Accuracy(const char *fileName);
        Accuracy(const char *fileName, int _threshold);
        ~Accuracy();
        bool fillFrameState(unsigned int frame_id, const int *rectstat);
        void generateAccuracyReport();
        bool isSimiliarAccuracy(Accuracy *accuracy);
};

class Performance {
    private:
        // a map used to record the statics of time used for MOV processing
        // key - the frame id
        // value - the time spent for one subset of the whole processing
        std::vector<std::pair<unsigned int, std::vector<double> > > map;
        std::ofstream ofs;
        // diff, wise multiple
        // threshold & orb
        // opticalflow calc, opticalflow draw, find moving object
        // sum
        int infoSize;

    public:
        Performance(const char *fileName);
        Performance(const char *fileName, int infoSize);
        ~Performance();
        bool fillFrameInfo(unsigned int frame_id, std::vector<double> &info);
        void generatePerfReport();
        void comparePerfReport(Performance *perf);
};

#endif
