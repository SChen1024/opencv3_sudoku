#pragma once
#pragma warning(disable:4996) //全部关掉文件读写函数不加s的错误

#include <iostream>  
#include <vector>
#include <string>
#include <io.h>
#include <time.h>


#include <opencv2/core/core.hpp>  
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>



#include "areadivider.h"    //杆面区域分割头文件
#include "scardetector.h"   //杆面伤痕识别

#include "areaRDiv.h"       //R面分割函数头文件
#include "WatershedProc.h"  //分水岭算法实现



using namespace cv;
using namespace std;


