#include <iostream>
#include <vector>
#include <ctime>
#include <stdio.h>

#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/video/video.hpp"
#include "opencv2/gpu/gpu.hpp"

using namespace std;
using namespace cv;
using namespace cv::gpu;

double
get_timediff(timespec &ts1, timespec &ts2) {
    double sec_diff = difftime(ts2.tv_sec, ts1.tv_sec);
    long nsec_diff = ts2.tv_nsec - ts1.tv_nsec;

    return sec_diff * 1000 + nsec_diff / 1000000;
}

static void download(const GpuMat& d_mat, vector<Point2f>& vec)
{
    vec.resize(d_mat.cols);
    Mat mat(1, d_mat.cols, CV_32FC2, (void*)&vec[0]);
    d_mat.download(mat);
}

static void download(const GpuMat& d_mat, vector<uchar>& vec)
{
    vec.resize(d_mat.cols);
    Mat mat(1, d_mat.cols, CV_8UC1, (void*)&vec[0]);
    d_mat.download(mat);
}

static void download(const GpuMat& d_mat, vector<float>& vec)
{
    vec.resize(d_mat.cols);
    Mat mat(1, d_mat.cols, CV_32FC1, (void*)&vec[0]);
    d_mat.download(mat);
}

static bool isEqual(const Mat &a, const Mat &b, const char *msg) {
    Mat tmp;
    bitwise_xor(a, b, tmp);
    int cnt = countNonZero(tmp);
    if (cnt > 0) {
        printf("Mat is not equal for %s, cnt: %d\n", msg, cnt);
        return false;
    } else {
        //INFO("Mat is equal for %s\n", msg);
        return true;
    }
}

#define seq_cuda_err 0.001       // error allowed between seq and cuda 
static bool isEqual(const vector<Point2f> &a, const vector<Point2f> &b, const char *msg) {
    if (a.size() != b.size()) {
        printf("vector is not equal for %s, size of a: %ld, size of b: %ld\n", msg, a.size(), b.size());
        return false;
    }
    int cnt = 0;
    for (int i = 0; i < a.size(); i++) {
#if 0
        if (a[i].x != b[i].x || a[i].y != b[i].y) {
            printf("i: %d, a[i].x: %f, b[i].x: %f, a[i].y: %f, b[i].y: %f\n",
                    i, a[i].x, b[i].x, a[i].y, b[i].y);
            cnt++;
        }
#else
        if (abs(a[i].x - b[i].x) / a[i].x > seq_cuda_err || 
                abs(a[i].y - b[i].y) / a[i].y > seq_cuda_err) {
            //printf("i: %d, a[i].x: %f, b[i].x: %f, a[i].y: %f, b[i].y: %f\n",
            //        i, a[i].x, b[i].x, a[i].y, b[i].y);
            cnt++;
        }
#endif
    }
    if (cnt > 0) {
        printf("vector is not equal for %s, cnt: %d per: %0.2f%%\n", msg, cnt, (float)cnt/a.size()*100);
        return false;
    }
    return true;
}

static bool isEqual(const vector<uchar> &a, const vector<uchar> &b, const char *msg) {
    if (a.size() != b.size()) {
        printf("vector is not equal for %s, size of a: %ld, size of b: %ld\n", msg, a.size(), b.size());
        return false;
    }
    int cnt = 0;
    for (int i = 0; i < a.size(); i++) {
        if (a[i] != b[i]) {
            //printf("i: %d, a[i]: %d, b[i]: %d\n", i, a[i], b[i]);
            cnt++;
        }
    }
    if (cnt > 0) {
        printf("vector is not equal for %s, cnt: %d, per: %0.2f%%\n", msg, cnt, (float)cnt/a.size());
        return false;
    }
    return true;
}

static bool isEqual(const vector<float> &a, const vector<float> &b, const char *msg) {
    if (a.size() != b.size()) {
        printf("vector is not equal for %s, size of a: %ld, size of b: %ld\n", msg, a.size(), b.size());
        return false;
    }
    int cnt = 0;
    for (int i = 0; i < a.size(); i++) {
        if (a[i] != b[i]) {
            cnt++;
        }
    }
    if (cnt > 0) {
        printf("vector is not equal for %s, cnt: %d, per: %0.2f%%\n", msg, cnt, (float)cnt/a.size());
        return false;
    }
    return true;
}


static void drawArrows(Mat& frame, const vector<Point2f>& prevPts, const vector<Point2f>& nextPts, const vector<uchar>& status, Scalar line_color = Scalar(0, 0, 255))
{
    for (size_t i = 0; i < prevPts.size(); ++i)
    {
        if (status[i])
        {
            int line_thickness = 1;

            Point p = prevPts[i];
            Point q = nextPts[i];

            double angle = atan2((double) p.y - q.y, (double) p.x - q.x);

            double hypotenuse = sqrt( (double)(p.y - q.y)*(p.y - q.y) + (double)(p.x - q.x)*(p.x - q.x) );

            if (hypotenuse < 1.0)
                continue;

            // Here we lengthen the arrow by a factor of three.
            q.x = (int) (p.x - 3 * hypotenuse * cos(angle));
            q.y = (int) (p.y - 3 * hypotenuse * sin(angle));

            // Now we draw the main line of the arrow.
            line(frame, p, q, line_color, line_thickness);

            // Now draw the tips of the arrow. I do some scaling so that the
            // tips look proportional to the main line of the arrow.

            p.x = (int) (q.x + 9 * cos(angle + CV_PI / 4));
            p.y = (int) (q.y + 9 * sin(angle + CV_PI / 4));
            line(frame, p, q, line_color, line_thickness);

            p.x = (int) (q.x + 9 * cos(angle - CV_PI / 4));
            p.y = (int) (q.y + 9 * sin(angle - CV_PI / 4));
            line(frame, p, q, line_color, line_thickness);
        }
    }
}

template <typename T> inline T clamp (T x, T a, T b)
{
    return ((x) > (a) ? ((x) < (b) ? (x) : (b)) : (a));
}

template <typename T> inline T mapValue(T x, T a, T b, T c, T d)
{
    x = clamp(x, a, b);
    return c + (d - c) * (x - a) / (b - a);
}

static void getFlowField(const Mat& u, const Mat& v, Mat& flowField)
{
    float maxDisplacement = 1.0f;

    for (int i = 0; i < u.rows; ++i)
    {
        const float* ptr_u = u.ptr<float>(i);
        const float* ptr_v = v.ptr<float>(i);

        for (int j = 0; j < u.cols; ++j)
        {
            float d = max(fabsf(ptr_u[j]), fabsf(ptr_v[j]));

            if (d > maxDisplacement)
                maxDisplacement = d;
        }
    }

    flowField.create(u.size(), CV_8UC4);

    for (int i = 0; i < flowField.rows; ++i)
    {
        const float* ptr_u = u.ptr<float>(i);
        const float* ptr_v = v.ptr<float>(i);


        Vec4b* row = flowField.ptr<Vec4b>(i);

        for (int j = 0; j < flowField.cols; ++j)
        {
            row[j][0] = 0;
            row[j][1] = static_cast<unsigned char> (mapValue (-ptr_v[j], -maxDisplacement, maxDisplacement, 0.0f, 255.0f));
            row[j][2] = static_cast<unsigned char> (mapValue ( ptr_u[j], -maxDisplacement, maxDisplacement, 0.0f, 255.0f));
            row[j][3] = 255;
        }
    }
}

int main(int argc, const char* argv[])
{
    const char* keys =
        "{ h            | help           | false | print help message }"
        "{ l            | left           |       | specify left image }"
        "{ r            | right          |       | specify right image }"
        "{ gray         | gray           | false | use grayscale sources [PyrLK Sparse] }"
        "{ win_size     | win_size       | 21    | specify windows size [PyrLK] }"
        "{ max_level    | max_level      | 3     | specify max level [PyrLK] }"
        "{ iters        | iters          | 30    | specify iterations count [PyrLK] }"
        "{ points       | points         | 4000  | specify points count [GoodFeatureToTrack] }"
        "{ min_dist     | min_dist       | 0     | specify minimal distance between points [GoodFeatureToTrack] }";

    CommandLineParser cmd(argc, argv, keys);

    if (cmd.get<bool>("help"))
    {
        cout << "Usage: pyrlk_optical_flow [options]" << endl;
        cout << "Avaible options:" << endl;
        cmd.printParams();
        return 0;
    }

    string fname0 = cmd.get<string>("left");
    string fname1 = cmd.get<string>("right");

    if (fname0.empty() || fname1.empty())
    {
        cerr << "Missing input file names" << endl;
        return -1;
    }

    bool useGray = cmd.get<bool>("gray");
    int winSize = cmd.get<int>("win_size");
    int maxLevel = cmd.get<int>("max_level");
    int iters = cmd.get<int>("iters");
    int points = cmd.get<int>("points");
    double minDist = cmd.get<double>("min_dist");

    Mat frame0 = imread(fname0);
    Mat frame1 = imread(fname1);

    if (frame0.empty() || frame1.empty())
    {
        cout << "Can't load input images" << endl;
        return -1;
    }

    namedWindow("PyrLK [Sparse]", WINDOW_NORMAL);
    namedWindow("PyrLK [Dense] Flow Field", WINDOW_NORMAL);

    cout << "Image size : " << frame0.cols << " x " << frame0.rows << endl;
    cout << "Points count : " << points << endl;
    cout << endl;

    Mat frame0Gray;
    cvtColor(frame0, frame0Gray, COLOR_BGR2GRAY);
    Mat frame1Gray;
    cvtColor(frame1, frame1Gray, COLOR_BGR2GRAY);

    // goodFeaturesToTrack
    // seq
    vector<Point2f> prevPts;
    vector<Point2f> nextPts;
    goodFeaturesToTrack(frame0Gray, prevPts, points, 0.01, 10, Mat());
    
    // cuda
    GoodFeaturesToTrackDetector_GPU detector(points, 0.01, 10);
    GpuMat d_frame0Gray(frame0Gray);
    GpuMat d_prevPts;
    detector(d_frame0Gray, d_prevPts);
   
    // check prevPts between seq and cuda
    vector<Point2f> checkPrevPts;
    download(d_prevPts, checkPrevPts);
    isEqual(prevPts, checkPrevPts, "checkPrevPts");
  
    // Sparse
    // seq
    struct timespec one, two, three;
    
    vector<uchar> status;
    vector<float> err;
    clock_gettime(CLOCK_REALTIME, &one);
    calcOpticalFlowPyrLK(
            useGray ? frame0Gray : frame0,
            useGray ? frame1Gray : frame1,
            prevPts,
            nextPts,
            status,
            err);
    clock_gettime(CLOCK_REALTIME, &two);

    GpuMat d_frame0(frame0);
    GpuMat d_frame1(frame1);
    GpuMat d_frame1Gray(frame1Gray);
    GpuMat d_nextPts;
    GpuMat d_status;
    GpuMat d_err;
    
    // cuda
    PyrLKOpticalFlow d_pyrLK;
    d_pyrLK.winSize.width = winSize;
    d_pyrLK.winSize.height = winSize;
    d_pyrLK.maxLevel = maxLevel;
    d_pyrLK.iters = iters;
    d_pyrLK.sparse(useGray ? d_frame0Gray : d_frame0, 
                   useGray ? d_frame1Gray : d_frame1, 
                   d_prevPts, 
                   d_nextPts, 
                   d_status,
                   &d_err);
    clock_gettime(CLOCK_REALTIME, &three);
    
    double t_seq = get_timediff(one, two);
    double t_cuda = get_timediff(two, three);
    double t_total = t_seq + t_cuda;
    printf("time for seq: %0.2f ms, per: %0.2f%%\n", t_seq, t_seq/t_total*100);
    printf("time for cuda: %0.2f ms, per: %0.2f%%\n", t_cuda, t_cuda/t_total*100);

    // Draw arrows
    vector<Point2f> h_prevPts;
    vector<Point2f> h_nextPts;
    vector<uchar> h_status;
    vector<float> h_err;
    download(d_prevPts, h_prevPts);
    download(d_nextPts, h_nextPts);
    download(d_status, h_status);
    download(d_err, h_err);

    // check
    isEqual(h_prevPts, prevPts, "PrevPts");
    isEqual(h_nextPts, nextPts, "nextPts");
    isEqual(h_status, status, "status");
    isEqual(h_err, err, "err");
    
    drawArrows(frame0, prevPts, nextPts, status, Scalar(255, 0, 0));
    imshow("PyrLK [Sparse]", frame0);

    // Dense
    GpuMat d_u;
    GpuMat d_v;

    d_pyrLK.dense(d_frame0Gray, d_frame1Gray, d_u, d_v);

    // Draw flow field

    Mat flowField;
    getFlowField(Mat(d_u), Mat(d_v), flowField);

    imshow("PyrLK [Dense] Flow Field", flowField);

    waitKey();

    return 0;
}
