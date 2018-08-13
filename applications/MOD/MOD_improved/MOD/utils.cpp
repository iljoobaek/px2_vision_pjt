#include <vector>
#include <utility>
#include <bitset>
#include <iostream>
#include <iomanip>
#include "utils.h"
#include "log.h"

static int log_level = LOG_INFO;

/**
 * the following APIs are used to evaluate the correctness of CUDA algo
 */
void download(const GpuMat& d_mat, vector<Point2f>& vec) {
    vec.resize(d_mat.cols);
    Mat mat(1, d_mat.cols, CV_32FC2, (void*)&vec[0]);
    d_mat.download(mat);
}

void download(const GpuMat& d_mat, vector<uchar>& vec) {
    vec.resize(d_mat.cols);
    Mat mat(1, d_mat.cols, CV_8UC1, (void*)&vec[0]);
    d_mat.download(mat);
}

void download(const GpuMat& d_mat, vector<float>& vec) {
    vec.resize(d_mat.cols);
    Mat mat(1, d_mat.cols, CV_32FC1, (void*)&vec[0]);
    d_mat.download(mat);
}

void upload(vector<Point2f>& vec, GpuMat& d_mat) {
    Mat mat(1, vec.size(), CV_32FC2, (void *)&vec[0]);
    d_mat.upload(mat);
}

bool isEqual(const Mat &a, const Mat &b, const char *msg) {
    Mat tmp;
    if (a.rows != b.rows || a.cols != b.cols) {
        ERR("Mat is not equal in size!");
        return false;
    }
    bitwise_xor(a, b, tmp);
    int cnt = countNonZero(tmp);
    if (cnt > 0) {
        ERR("Mat is not equal for %s, cnt: %d, per:%0.2f%%", msg, cnt, (float)cnt/a.rows*a.cols);
        return false;
    }
    return true;
}

bool isEqual(const vector<Point2f> &a, const vector<Point2f> &b, const char *msg) {
    if (a.size() != b.size()) {
        ERR("vector is not equal for %s, size of a: %ld, size of b: %ld", msg, a.size(), b.size());
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
        ERR("vector is not equal for %s, cnt: %d per: %0.2f%%", msg, cnt, (float)cnt/a.size()*100);
        return false;
    }
    return true;
}

bool isEqual(const vector<uchar> &a, const vector<uchar> &b, const char *msg) {
    if (a.size() != b.size()) {
        ERR("vector is not equal for %s, size of a: %ld, size of b: %ld", msg, a.size(), b.size());
        return false;
    }
    int cnt = 0;
    for (int i = 0; i < a.size(); i++) {
        // if the diff between seq and cuda is larger than 0.001
        if (a[i] != b[i]) {
            //printf("i: %d, a[i]: %d, b[i]: %d", i, a[i], b[i]);
            cnt++;
        }
    }
    if (cnt > 0) {
        ERR("vector is not equal for %s, cnt: %d, per: %0.2f%%", msg, cnt, (float)cnt/a.size());
        return false;
    }
    return true;
}

bool isEqual(const vector<float> &a, const vector<float> &b, const char *msg) {
    if (a.size() != b.size()) {
        ERR("vector is not equal for %s, size of a: %ld, size of b: %ld", msg, a.size(), b.size());
        return false;
    }
    int cnt = 0;
    for (int i = 0; i < a.size(); i++) {
        // if the diff between seq and cuda is larger than 0.001
        if (abs(a[i] - b[i]) > seq_cuda_err) {
            //ERR("i: %d, a[i]: %f, b[i]: %f", i, a[i], b[i]);
            cnt++;
        }
    }
    if (cnt > 0) {
        ERR("vector is not equal for %s, cnt: %d, per: %0.2f%%", msg, cnt, (float)cnt/a.size());
        return false;
    }
    return true;
}

/**
 * the time diff between ts2 and ts1
 * ts1 - early time
 * ts2 - later time
 */
double get_timediff(timespec &ts1, timespec &ts2) {
    double sec_diff = difftime(ts2.tv_sec, ts1.tv_sec);
    long nsec_diff = ts2.tv_nsec - ts1.tv_nsec;

    return sec_diff * 1000 + nsec_diff / 1000000;
}


/**
 * constructor of class Accuracy with output fileName and similiarity threshold specified
 */
Accuracy::Accuracy(const char *fileName, int _threshold) {
    ENTER();
    if (!fileName) {
        ERR("Invalid fileName specified!\n");
        return;
    }
    this->ofs = std::ofstream(fileName);
    this->threshold = _threshold;
    LEAVE(); 
}

/**
 * constructor of class Accuracy with output fileName specified
 * default threshold: 240 = 200 * 12 * 1% (1% error offset is allowed)
 */
Accuracy::Accuracy(const char *fileName) : Accuracy(fileName, 240){
}

Accuracy::~Accuracy() {
   
}

/**
 * fill the map with frame_id and rectstat data
 */
bool
Accuracy::fillFrameState(unsigned int frame_id, const int *rectstat) {
    ENTER();
    if (!rectstat) {
        ERR("Invalid rectstate specified!\n");
        return false;
    }
    if (frame_id < 0) {
        ERR("Invalid frame_id %u specified!\n", frame_id);
        return false;
    }

    unsigned int state = 0;
    for (int i = 1; i < RECTSTAT_SIZE; i++) {
        if (rectstat[i]) {
            state |= (0x1 << (i-1));
        }
    }
    map.push_back(std::make_pair(frame_id, state));
    LEAVE();
    return true;
}

/**
 * generate the report and output to the file specified in the constructor
 */
void
Accuracy::generateAccuracyReport() {
    ENTER();
    if (!this->ofs.is_open()) {
        ERR("ofs is not ready!\n");
        return;
    }

    this->ofs << "frame_id" << "\t" << "rectstat (12 bits, the left top rectangle is the lowest bit)\n";
    for (const auto &item : map) {
	for (auto i=0; i<12; i++) {
		this->ofs << item.first 
				<< "\t" 
				<< i
				<< "\t"
				<< bool(item.second&(1<<i))
				<< "\n";
		//this->ofs << item.first << "\t" << item.second << "\n";
	}
    }
    this->ofs.close();
    LEAVE();
}

/**
 * check if the accuracy specified is similiar with this object
 */
bool 
Accuracy::isSimiliarAccuracy(Accuracy *accuracy) {
    if (accuracy->map.size() != this->map.size()) {
        ERR("Invalid accuracy specified, the size is different!\n");
        return false;
    }
    int n = accuracy->map.size();
    int cnt = 0;
    for (int i = 0; i < n; i++) {
        int a = accuracy->map[i].second;
        int b = this->map[i].second;

        for (int j = 0; j < RECTSTAT_SIZE - 1; j++) {
            if ((a & (0x1 << j)) != (b & (0x1 << j))) {
                cnt++;
                if (cnt > this->threshold) {
                    return false;
                }
            }
        }
    }
    return true;
}

/**
 * constructor for Performance class
 */
Performance::Performance(const char *fileName, int _infoSize) {
    ENTER();
    if (!fileName) {
        ERR("Invalid fileName specified!\n");
        return;
    }
    if (_infoSize <= 0) {
        ERR("Invalid info size specified!\n");
        return;
    }
    this->ofs = std::ofstream(fileName);
    this->infoSize = _infoSize;
    LEAVE(); 
}


/**
 * default constructor for Performance class
 * currently, 7 categories of statics info:
 * - diff, wise multiple
 * - threshold & orb
 * - opticalflow calc, opticalflow draw, find moving object
 * - sum
 */
Performance::Performance(const char *fileName) : Performance(fileName, 7) {
}

Performance::~Performance() {
}

/**
 * fill frame info to the map
 */
bool
Performance::fillFrameInfo(unsigned int frame_id, std::vector<double> &info) {
    if (frame_id < 0) {
        ERR("Invalid frame_id %u specified!\n", frame_id);
        return false;
    }
    if (info.size() != this->infoSize) {
        ERR("Invalid info size specified, should be same as infoSize %u!\n", infoSize);
        return false;
    }
    this->map.push_back(std::make_pair(frame_id, info));
    return true;
}

/**
 * generate the performance report and write to the specified file
 */
void 
Performance::generatePerfReport() {
    ENTER();
    if (!this->ofs.is_open()) {
        ERR("ofs is not ready!\n");
        return;
    }
    vector<double> averages(this->infoSize, 0);
    vector<double> sums(this->infoSize, 0);
    vector<double> sds(this->infoSize, 0); //standard deviation
    // calc sums
    for (int i = 0; i < this->map.size(); i++) {
        auto p = map[i].second;
        for (int j = 0; j < this->infoSize; j++) {
            sums[j] += p[j];
        }
    }
    // calc average
    int number = this->map[this->map.size()-1].first + 1; // start from 0
    for (int j = 0; j < this->infoSize; j++) {
        averages[j] = sums[j] / number;
    }
    // calc variance
    for (int i = 0; i < this->map.size(); i++) {
        auto p = map[i].second;
        for (int j = 0; j < this->infoSize; j++) {
            double diff = abs(p[j] - averages[j]);
            sds[j] += diff * diff;
        }
    }
    for (int j = 0; j < this->infoSize; j++) {
        sds[j] = sqrt(sds[j] / number);
    }

    this->ofs << "frameNum" << "\t" << "diff" << "\t\t\t" << "wise" << "\t\t\t" << "threshold" << "\t\t" << "opt_calc" << "\t\t" << "opt_draw" << "\t\t" << "mod" 
        << "\t\t\t\t" << "sum\n";

    this->ofs << number << "\t";
    for (int j = 0; j < this->infoSize; j++) {
            this->ofs << "\t\t" << std::fixed << std::setprecision(2) << averages[j] << "+-" << sds[j];
    }
    this->ofs << "\n";
    this->ofs.close();

    LEAVE();
}

void 
Performance::comparePerfReport(Performance *perf) {
    //TODO
    return;
}
