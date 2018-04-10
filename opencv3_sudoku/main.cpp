/********************************************************************
文件: main.cpp
功能: 主函数操作
备注: 20180111 陈栓 设立文件
***********************************************************************/

#include "commonHead.h"

//常用图片
#define PIC_LENA	"../picture/Lena.jpg"
#define SRC_WIN_NAME "素描图变换"
#define DST_WIN_NAME "结果目标窗口"


/*======================================================================
函数: main
功能: 主函数操作
输入: 无
输出: 0
备注: 无
======================================================================*/



int main()
{
	//opencv 读取摄像头
	//videoCap();
	//ImageScanner barScanner;
	//barScanner.set_config(ZBAR_NONE, ZBAR_CFG_ENABLE, 1);

	Mat srcImg = imread(PIC_LENA);
	//imshow(SRC_WIN_NAME,srcImg);

    Mat grayImg, edgeImg,maskImg;
   
    const int MEDIAN_BLUR_FILTER_SIZE = 7;
    const int LAPLACIAN_FILTER_SIZE =5;
    const int EDGES_THRESHOLD = 80;

    //缩放为1半
    Size size = srcImg.size();
    Size smallSize;
    smallSize.width = size.width/2;
    smallSize.height = size.height /2;
    Mat smallImg = Mat(smallSize,CV_8UC3);
    resize(srcImg,smallImg,smallSize,0,0,INTER_LINEAR);

    //处理数据
    cvtColor(smallImg, grayImg, CV_BGR2GRAY);
    medianBlur(grayImg,grayImg,MEDIAN_BLUR_FILTER_SIZE);
    Laplacian(grayImg,edgeImg,CV_8U,LAPLACIAN_FILTER_SIZE);
    threshold(edgeImg,maskImg,EDGES_THRESHOLD,255,THRESH_BINARY_INV);

    //imshow(DST_WIN_NAME, edgeImg);



	waitKey(0);
    cvDestroyAllWindows();
	return 0;


}


