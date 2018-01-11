/********************************************************************
	文件: main.cpp
	功能: 主函数操作
	备注: 20180111 陈栓 设立文件

***********************************************************************/

#include "commonHead.h"

/*======================================================================
	函数: main
	功能: 主函数操作
	输入: 无
	输出: 0
	备注: 无

======================================================================*/
int main()
{
	//读入摄像头内容
	VideoCapture capture(1);

	//灰度图,黑白图,边界图
	Mat grayFrame, binFrame, edgeFrame;

	//循环显示
	while (1)
	{ 
			Mat srcFrame;
			capture >> srcFrame;

			//转换为灰度图
			cvtColor(srcFrame, grayFrame, CV_BGR2GRAY);
			//转换为黑白图
			threshold(grayFrame, binFrame, 128, 255, CV_THRESH_OTSU);
			//模糊化
			blur(grayFrame, grayFrame, Size(3, 3));
			//转换为边界图
			Canny(grayFrame, edgeFrame, 0, 30, 3);
			
			imshow("srcFrame", srcFrame);
			imshow("grayFrame", grayFrame);
			imshow("binFrame", binFrame);
			imshow("edgeFrame", edgeFrame);

			waitKey(30);
	}
	capture.release();
	destroyAllWindows();
	return 0;
}
