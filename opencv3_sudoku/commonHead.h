/********************************************************************
	文件: commonHead.h
	功能: 常用头文件定义区域
	备注: 20180111 设立文件
***********************************************************************/
#pragma once
#pragma warning(disable:4996) //全部关掉文件读写函数不加s的错误

#include<iostream>  
#include <opencv2/core/core.hpp>  
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>



using namespace cv;

using namespace std;
//宏定义区


//函数声明
int videoCap(void);
int imgChangeSave(void);
int creatAlphaMat(Mat &mat);
int creatSaveImg(void);








