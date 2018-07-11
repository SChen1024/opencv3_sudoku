/********************************************************************
文件: main.cpp
功能: 主函数操作
备注: 20180111 陈栓 设立文件
***********************************************************************/

#include "commonHead.h"

//常用图片
#define PIC	"../picture/001-2.bmp"
#define WINDOWS_NAME "多窗口显示"


/*======================================================================
函数: main
功能: 主函数操作
输入: 无
输出: 0
备注: 无
======================================================================*/


int main(void)
{
	//Mat srcImg = imread(PIC_LENA);
	//imshow(WINDOWS_NAME,srcImg);

    Mat srcImg = imread(PIC);
    Mat grayImg,dstImg;
    cv::cvtColor(srcImg,grayImg,CV_BGR2GRAY);

    pyrDown(grayImg,grayImg);
    pyrUp(grayImg,grayImg);

    cv::Canny(grayImg, grayImg, 50, 100, 3);

    cv::imshow(WINDOWS_NAME,grayImg);





	waitKey(0);
	return 0;
}



