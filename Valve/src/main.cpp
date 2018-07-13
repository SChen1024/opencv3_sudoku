/*
    文件：main.cpp
    功能：测试分割函数以及识别函数
    
*/


#include "..\include\include.h"



#define TESTCYLINDER    0   //测试杆面图像
    #define CFace1  "Pic\\CylinderFace1.bmp"
    #define EFace0  "Pic\\E\\CFace0.bmp"  
    #define ERESULT  "Pic\\E\\res_CFace0.png"


#define TESTBFACE       0   //测试大端面图像
    #define BigFace1 "Pic\\BigFace1.bmp"
    #define BigFace2 "Pic\\BigFace2.bmp"


#define TESTRFACE       0   //R面 弧面 锥面测试
    #define RFace1  "Pic\\R\\RFace1.bmp"   
    #define RFace2  "Pic\\R\\RFace2.bmp"  
    #define RFace0  "Pic\\R\\RFace0.bmp"  
    #define RESULT  "Pic\\Pick\\R\\res074-3.png"

#define TEST        1    //测试算法通道




    const char *RFaceWindowsName = "测试图像显示窗口";
   
    #define MaxWidth 430    //表示当前可见的窗口能够看到的图像最大值，
                            //根据当前图像进行适当调整

    using namespace cv;




    Vec3b RandomColor(int value)//生成随机颜色函数
    {
        value = value % 255;  //生成0~255的随机数  
        RNG rng;
        int aa = rng.uniform(0, value);
        int bb = rng.uniform(0, value);
        int cc = rng.uniform(0, value);
        return Vec3b(aa, bb, cc);
    }








int  main(void)
{
#if TESTCYLINDER
    Mat srcImg = imread(EFace1);
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
    Mat transImg=srcRImg.clone();
    resize(srcRImg, transImg, Size(srcRImg.cols, srcRImg.rows * 4), 0, 0, INTER_LINEAR);

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


#if TEST


    Mat srcRImg = imread(RFace1);

    //将图像纵向拉伸
    Mat transImg = srcRImg.clone();
    //resize(srcRImg, transImg, Size(srcRImg.cols, srcRImg.rows * 4), 0, 0, INTER_LINEAR);

    Mat grayImg,edgeImg,cannyImg, dstImg;

    AreaRDiv ard;
    //ard.preProc(transImg, cannyImg);
    cvtColor(transImg, grayImg,CV_BGR2GRAY);
    //GaussianBlur(grayImg, grayImg, Size(3,3), 0);


    grayImg.convertTo(grayImg, CV_32F);
    //Sobel(grayImg, grayImg, CV_8U, 1,0);
    Sobel(grayImg, grayImg, CV_32F, 0, 1);
    convertScaleAbs(grayImg, dstImg);



    /////////////////////////////////////////////////////////

    threshold(grayImg,edgeImg,0,255,CV_THRESH_OTSU);
    //bitwise_not(edgeImg, edgeImg);

    Mat kernel = getStructuringElement(MORPH_RECT, Size(3,3));

    //膨胀之后得到背景
    Mat sur_bg, sur_fr;
    //Mat dilateImg=edgeImg.clone();

    dilate(edgeImg, sur_bg, kernel, Point(-1,-1),3);


    Mat distImg;
    distanceTransform(edgeImg, distImg,DIST_L1, 3);

    distImg.convertTo(distImg, CV_8U);

    threshold(distImg,sur_fr, 0, 255, CV_THRESH_BINARY);

    Mat unknownImg;
    subtract(sur_bg, sur_fr, unknownImg);

    Mat markers(unknownImg.size(),CV_8UC1);

    connectedComponents(sur_fr, markers);

    markers = markers + 1;

    //markers[unknownImg == 255] = 0;

    for (int  i = 0;i < unknownImg.rows;i++)
    {
        unsigned char *data = unknownImg.ptr<unsigned char>(i);
        int *dataM = markers.ptr< int>(i);
        for (int  j = 0;j < unknownImg.cols;j++)
        {
            if (data[j] == 255)
            {
                dataM[j] = 0;
            }
        }
    }
    watershed(srcRImg, markers);
    //markers.convertTo(markers, CV_8U);



    for (auto i = 0;i < markers.rows;i++)
    {
        unsigned char *data = markers.ptr< unsigned char>(i);
        //unsigned char *dataM = grayImg.ptr<unsigned char>(i);
        for (auto j = 0;j < markers.cols;j++)
        {
            if (data[j] == -1)
            {
                //dataM[j] =255;
                srcRImg.at<cv::Vec3b>(j, i) = { 0,0,255 };
            }
        }
    }






    //add(sur_bg,Mat::all(1),)














    //namedWindow(RFaceWindowsName, CV_WINDOW_AUTOSIZE);
    //imshow(RFaceWindowsName, dstImg);
    //imwrite(RESULT, dstImg);
    //imwrite("Pic\\test\\canny_img.png", cannyImg);

    waitKey(0);


#endif



    return 0;
}



