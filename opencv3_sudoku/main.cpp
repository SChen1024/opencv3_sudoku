/********************************************************************
文件: main.cpp
功能: 主函数操作
备注: 20180111 陈栓 设立文件
***********************************************************************/

#include "commonHead.h"

//常用图片
#define PIC_LENA	"../picture/Lena.jpg"

#define BAR1		"../picture/qrcode/barcode1.png"
#define BAR2		"../picture/qrcode/barcode2.png"

#define WINDOWS_NAME "条形码识别"


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
	ImageScanner barScanner;
	barScanner.set_config(ZBAR_NONE, ZBAR_CFG_ENABLE, 1);

	Mat srcImg = imread(BAR1);
	imshow(WINDOWS_NAME,srcImg);
	Mat grayImg;
	cvtColor(srcImg, grayImg, CV_BGR2GRAY);
	int width = grayImg.cols;
	int height = grayImg.rows;
	uchar* raw = (uchar*)grayImg.data;
	Image zbarImg(width, height, "Y800", raw, width*height);
	barScanner.scan(zbarImg);
	Image::SymbolIterator symbol = zbarImg.symbol_begin();
	if (zbarImg.symbol_begin() == zbarImg.symbol_end())
	{
		cout << "查询条码失败,请检查图片!" << endl;
	}
	for (; symbol != zbarImg.symbol_end(); ++symbol);
	{
		cout << "类型:" << endl << symbol->get_type_name() << endl << endl;
		cout << "条码:" << endl << symbol->get_data() << endl << endl;
	}

	zbarImg.set_data(NULL, 0);



	
	
	//imwrite("pic/05_mouseBox.png", tmpImg);

	waitKey(0);
	return 0;


}