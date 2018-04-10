/********************************************************************
文件: function.cpp
功能: 实现基本函数操作
备注:	20180111  设立文件
20180111 videoCap函数
**********************************************************************/

#include "commonHead.h"
/*
函数: videoCap
功能: 实现摄像头抓取图像并做简单处理]
输入: 无
输出: 0
备注: 无
*/
int videoCap(void)
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

/*
函数: imgChangeSave
功能: 实现读取图片做简单处理之后储存下来.
输入: 无
输出: 0
备注: 无
*/

int imgChangeSave(void)
{

	Mat srcImg = imread("../picture/Lena.jpg", CV_LOAD_IMAGE_ANYCOLOR);
	imshow("原始图像", srcImg);
	Mat grayImg, binImg;
	cvtColor(srcImg, grayImg, CV_BGR2GRAY);
	threshold(grayImg, binImg, 128, 255, CV_THRESH_BINARY);
	imshow("黑白图像", binImg);
	imwrite("../picture/02_binImg.jpg", binImg);
	waitKey(0);

	return 0;
}

/*
函数: imgChangeSave
功能: 实现读取图片做简单处理之后储存下来.
输入: 无
输出: 0
备注: 无
*/
int creatAlphaMat(Mat &mat)
{
	for (int i = 0; i < mat.rows; ++i)
	{
		for (int j = 0; j < mat.cols; ++j)
		{
			Vec4b&rgba = mat.at<Vec4b>(i, j);
			rgba[0] = UCHAR_MAX;
			rgba[1] = saturate_cast<uchar>((float(mat.cols - j)) / ((float)mat.cols)*UCHAR_MAX);
			rgba[2] = saturate_cast<uchar>((float(mat.rows - i)) / ((float)mat.rows)*UCHAR_MAX);
			rgba[3] = saturate_cast<uchar>(0.5 * (rgba[1] + rgba[2]));
		}
	}
	return 0;
}

int creatSaveImg(void)
{
	Mat mat(480, 640, CV_8UC4);
	creatAlphaMat(mat);

	vector<int>compression_params;
	compression_params.push_back(IMWRITE_PNG_COMPRESSION);
	compression_params.push_back(9);

	try {
		imwrite("pic/透明Alpha值图.png", mat, compression_params);
		imshow("生成的png图片", mat);
		waitKey(0);
	}
	catch (runtime_error& ex) {
		fprintf(stderr, "图像转换出错:%s\n", ex.what());
		return 1;
	}
	return 0;
}





