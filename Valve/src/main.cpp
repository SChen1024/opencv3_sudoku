/*
    文件：main.cpp
    功能：测试分割函数以及识别函数
    
*/


#include "..\include\include.h"



#define TESTCYLINDER    0   //测试杆面图像
    #define CFace1  "Pic\\CylinderFace1.bmp"
    #define EFace1  "Pic\\E\\CFace1.bmp"  
    #define ERESULT  "Pic\\E\\res_CFace1.png"


#define TESTBFACE       0   //测试大端面图像
    #define BigFace1 "Pic\\BigFace1.bmp"
    #define BigFace2 "Pic\\BigFace2.bmp"


#define TESTRFACE       1   //R面 弧面 锥面测试
    #define RFace1  "Pic\\RFace1.bmp"   
    #define RFace2  "Pic\\RFace2.bmp"  
    #define RFace0  "Pic\\Pick\\R\\074-3.bmp"  
    #define RESULT  "Pic\\Pick\\R\\res074-3.png"


    const char *RFaceWindowsName = "测试图像显示窗口";
   
    #define MaxWidth 430    //表示当前可见的窗口能够看到的图像最大值，
                            //根据当前图像进行适当调整

    using namespace cv;

int  main(void)
{
#if TESTCYLINDER
    Mat srcImg = imread(EFace0);
    //载入后先显示
    //namedWindow("src");
    //imshow("src", srcImg);
    std::vector<double> scales{0.0068,0.0777,0.9045,0.9176};
    std::vector<AreaDivider::Line> lines;

    vector<vector<Point>> scars;


    AreaDivider ad;
    ScarDetector sd;
   
    Mat dividerImg;
    ad.divide(srcImg, scales, lines);
    {
        sd.detect(srcImg, lines, scars);
        ad.drawLines(srcImg, lines, dividerImg);

        for (auto i = scars.cbegin();i != scars.cend();++i)
        {
            Rect r = boundingRect(*i);
            Rect r_(Point(r.x - 5, r.y - 5), Point(r.x + r.width + 5, r.y + r.height + 5));
            rectangle(dividerImg, r_, Scalar(0, 0, 255));
        }
    }

    //ad.drawLines(srcImg,lines,dividerImg);

    //imshow("dst",dividerImg);
    imwrite(ERESULT, dividerImg);
    cv::waitKey(0);
#endif

#if TESTBFACE
    Mat bigFaceImg = imread(BigFace1);
    
    Mat grayImg,edgeImg;
    
    if(bigFaceImg.channels() == 3)
    {
        cvtColor(bigFaceImg, grayImg, CV_BGR2GRAY);
    }

#endif

#if TESTRFACE
    

    Mat srcRImg = imread(EFace1);

    //将图像纵向拉伸
    Mat transImg;
    //resize(srcRImg, transImg, Size(srcRImg.cols, srcRImg.rows * 4), 0, 0, INTER_LINEAR);

    Mat cannyImg,dstImg;

    AreaRDiv ard;
    ard.preProc(transImg, cannyImg);

    std::vector<AreaRDiv::Line> lines, linesFilter;
    ard.detectLines(cannyImg, lines);




    //根据图像暂定画出四条线，分别为，锥面边线，亮带左侧，亮带右侧，总宽度
    //根据视图上的能见宽度为430px 锥面分界线55px  最大检测范围380px
    //std::vector<double> scales_l={ 55.0,171.0,260.0,400.0};

    std::vector<double> scales_l = { (55/430.0)*MaxWidth, (380 / 430.0)*MaxWidth };

    std::vector<double> scales{ 0.0063,0.0777,0.9045,0.9017 };

    ard.filterLines(lines, linesFilter, scales_l);

    ard.drawLines(transImg,linesFilter, dstImg);


    //识别出来canny 的边缘  得到轮廓，进行过滤得到最终结果
    vector<vector<Point>> scars , scarsFilter;
    ard.detectScars(cannyImg, scars, linesFilter);

    ard.filterScars(cannyImg, scars, scarsFilter, linesFilter);
    
    ard.drawScars(dstImg, scarsFilter, dstImg);

    //将图像缩放为原始宽高比
    resize(dstImg, dstImg, Size(dstImg.cols, dstImg.rows / 2), 0, 0,INTER_LANCZOS4);




    //namedWindow(RFaceWindowsName, CV_WINDOW_AUTOSIZE);
    //imshow(RFaceWindowsName, dstImg);
    imwrite(RESULT, dstImg);
    //imwrite("Pic\\test\\canny_img.png", cannyImg);

    waitKey(0);


#endif



    return 0;
}



