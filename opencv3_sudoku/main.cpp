/********************************************************************
	文件: main.cpp
	功能: 主函数操作
	备注: 20180111 陈栓 设立文件

***********************************************************************/

#include "commonHead.h"

//常用的图形四副图
#define PIC_LENA	"../picture/Lena.jpg"
#define PIC_AIRPLANE "../picture/Airplane.jpg"
#define PIC_FRUIT	"../picture/Fruits.jpg"
#define PIC_BABOON	"../picture/Baboon.jpg"


#define WINDOWS_NAME "鼠标座标获取"

Rect gRect;
bool gDrawingBox = false;
RNG gRNG(12345);
void drawRectangle(cv::Mat& img, cv::Rect box);
void mouseHandle(int event, int x, int y, int flags, void* param);



void mouseHandle(int event, int x, int y, int flags, void* param)
{
	Mat& Img = *(cv::Mat*)param;
	switch (event)
	{
		//鼠标移动记录座标变化
		case EVENT_MOUSEMOVE:
		{
			if (gDrawingBox)
			{
				gRect.width = x - gRect.x;
				gRect.height = y - gRect.y;
			}
		}	break;

		//左键按下 记录起始点
		case EVENT_LBUTTONDOWN:
		{
			gDrawingBox = true;
			gRect = Rect(x, y, 0, 0);
		} break;

		//左键松开 画图
		case EVENT_LBUTTONUP:
		{
			gDrawingBox = false;
			if (gRect.width < 0)
			{
				gRect.x += gRect.width;
				gRect.width *= -1;
			}
			if (gRect.height < 0)
			{
				gRect.y += gRect.height;
				gRect.height *= -1;
			}

			drawRectangle(Img, gRect);

		} break;

		default:
			break;
	}
}

void drawRectangle(cv::Mat& img, cv::Rect box)
{
	rectangle(img, box.tl(), box.br(), Scalar(gRNG.uniform(0, 255), gRNG.uniform(0, 255), gRNG.uniform(0, 255)));
}





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
	//图像二值化并保存
	//imgChangeSave();

	//Mat srcImg = imread("../picture/Lena.jpg",CV_LOAD_IMAGE_ANYCOLOR);
	//imshow("原始图像",srcImg);

	gRect = Rect(-1, -1, 0, 0);
	Mat srcImg(600, 800, CV_8UC3), tmpImg;
	srcImg.copyTo(tmpImg);
	srcImg = Scalar::all(0);

	namedWindow(WINDOWS_NAME);
	setMouseCallback(WINDOWS_NAME, mouseHandle, (void*)&srcImg);

	while (1)
	{
		srcImg.copyTo(tmpImg);
		if (gDrawingBox)
			drawRectangle(tmpImg, gRect);
		imshow(WINDOWS_NAME, tmpImg);
		if (waitKey(10) == 27) break;
	}

	imwrite("pic/05_mouseBox.png", tmpImg);
	return 0;


}

