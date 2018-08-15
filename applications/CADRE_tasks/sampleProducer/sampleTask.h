/* This is a sample task that demonstrates how to create a task. */

#ifndef _SAMPLETASK_H_
#define _SAMPLETASK_H_

#include <string>
#include <task/Task.h>
#include <interfaces/Sample/Output/Abstract.h>

#include "opencv2/core/core.hpp"
#include "opencv2/core/mat.hpp"
#include "opencv2/gpu/gpu.hpp"
#include "opencv2/video/tracking.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/video/tracking.hpp"

//extern "C" {
//#include "eglconsumer_main.h"
//}
//#include "nvm_consumer.h"


#include <iostream>
#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <ctype.h>
#include <unistd.h>
#include <time.h>

#include <ctime>
#include <fstream>
#include <string.h>




class SampleTask: public task::Task
{
 public:
    SampleTask(const std::string taskName);
    virtual ~SampleTask();
    
    virtual bool initialize(void);
    virtual bool executive(void);
    virtual void cleanup(void);

 protected:
    SampleOutput * outputInterface_;
    SampleMessage outputData_;
    int exampleParam_;
};

#endif
