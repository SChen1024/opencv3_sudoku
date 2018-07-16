/*
    文件：main.cpp
    功能：测试分割函数以及识别函数
    
*/


#include "..\include\include.h"



#define TESTCYLINDER    0   //测试杆面图像
    #define EFace1  "Pic\\E\\CFace1.bmp"
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


#define TESTWATERSHED 0  //测试watershed 算法
#define TESTDIVIMG    0  //测试切割图像
#define TESTRDIV      0   // 测试R面分割算法
#define TEST        1    //测试算法通道

const char *RFaceWindowsName = "测试图像显示窗口";
   
#define MaxWidth 430    //表示当前可见的窗口能够看到的图像最大值，
                            //根据当前图像进行适当调整

//气门尺寸定义
//气门总长度103mm 
//从小端面开始 G区15mm  F区15mm E区大概45mm 大端面直径28mm 小端面直径6mm
//G区锁夹槽 G2 总宽5.2mm 左侧G1 4.9mm 右侧G3 4.9mm
//最左侧B区1mm 大概锁夹槽宽度3x1.15+2x0.9；
const int MaxLongmm = 103;
const int GLongmm = 15;
const int FLongmm = 15; 
const int ELongmm = 44;  //大概




//暂时只加入 柱面处
vector<double> borderScales = { 
    0,
    0.0080, //BC 分界线比例
    0.09,   //D1 D2 分界线
    0.2816, //D2 E分界线
    0.7087, //E F 分界线
    0.8543, //FG 分界线
    1 };


//暂时只加入 弧面处
vector<double> RborderScales = {
    0,
    0.1308, //C D1 分界线比例
    0.3692,   //D1 D0 分界线
    0.6028, //D0 D2
    1 };    //D2 E分界线 


void testFunction( Mat &srcImg, Mat &dstImg, vector<double> &borderScales )
{


}

//快排函数
int comP( const void *a, const void *b )
{
    return ( *(int *) a - *(int *) b );
}


int  main(void)
{
    //开始计时
    clock_t start, finish;
    double totaltime;
    start = clock();



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


#if TESTWATERSHED

    Mat srcRImg = imread(EFace0);

    WaterShedProc ws;
    Mat markers;
    ws.process( srcRImg, markers );

    Mat dstImg = srcRImg.clone();
    ws.drawMarkersLine( dstImg, markers );

    waitKey( 0 );


#endif



#if TESTDIVIMG




    AreaRDiv ard;
    String path = "Pic\\TestPic\\E\\";
    
    vector<string> files;
    vector<string> files_res;

    listFiles( path.c_str(), files );

    string suffix_ = "png";
    Mat dstImg;
    for (int i=0;i<files.size();i++)
    {

        cout << files[i] << endl;



        auto name_iter = std::find( files[i].crbegin(), files[i].crend(), '.' );
        std::string name = std::string( files[i].cbegin(), name_iter.base() );
        name += suffix_;
        //name = path + name;

        files_res.push_back(name);

        //cout << files_res[i] << endl;

        Mat srcRImg = imread( files[i] );

        ard.divideImg( srcRImg ,dstImg,borderScales);

        imwrite( name, dstImg );

    }

    cout<<"总计"<< files.size() <<"图片" << endl;








#endif



#if TESTRDIV

    AreaRDiv ard;
    std::string path = "Pic\\TestPic\\R";
    std::string respath = "Pic\\TestPic\\res";

    vector<string> files;
    vector<string> files_res;

    listFiles( path.c_str(), files );

    Mat dstImg;
    for (int i = 0;i<files.size();i++)
    {

        cout << files[i] << endl;

        string  newfile;
        renameFiles( files[i], newfile, respath );
        
        files_res.push_back( newfile );

        cout << files_res[i] << endl;

        Mat srcRImg = imread( files[i] );

        ard.RdivideImg( srcRImg, dstImg, RborderScales );

        imwrite( newfile, dstImg );

    }

    cout << "总计" << files.size() << "图片" << endl;





#endif

#if TEST

#define AREAB   0    //B
#define AREAD2  0    //D2
#define AREAE   0    // E
#define AREAF   0    //F 

#define AREAG   1    //G


    
    Mat srcRImg = imread( EFace0 );
    vector<Mat> cellImg;
    vector<AreaRDiv::Line> borderlines;
    Mat dstImg;

    AreaRDiv ard;
    ard.divideImg( srcRImg, dstImg, borderScales, borderlines );

    cutImage( srcRImg, borderlines, cellImg );

    //获取到各区域的图片
    //Mat AreaBImg, AreaD2Img, AreaEImg, AreaFImg, AreaGImg;
    //AreaBImg = cellImg[0];
    //AreaD2Img = cellImg[2];
    //AreaEImg = cellImg[3];
    //AreaFImg = cellImg[4];
    //AreaGImg = cellImg[5];

    //确定准确的边缘位置
    vector<AreaRDiv::Line> realLines;


#if AREAB
    /*==========================================================================*/
    /*        B区域处理方法***/
    
    
    Mat AreaBImg = cellImg[0];
    Mat grayBImg, edgeBImg, cannyBImg, dstBImg;

    cvtColor( AreaBImg, grayBImg, CV_BGR2GRAY );
    threshold( grayBImg, edgeBImg, 0, 255, CV_THRESH_OTSU );
    Canny( edgeBImg, cannyBImg, 30, 80, 3 );

    //对得到的两条轮廓统计横座标值，确定轮廓位置

    int line_l = 0, line_l_max = 0;
    int line_r = cannyBImg.cols, line_r_max = 0;
    for (int i = 5; i < 12; i++)
    {
        float avgl = 0,avgr=0;
        for (int j = 0;j < cannyBImg.rows;j++)
        {
            avgl += (float)cannyBImg.at<uchar>( j, i ) / cannyBImg.rows;
            avgr += (float) cannyBImg.at<uchar>( j, cannyBImg.cols-i ) / cannyBImg.rows;
        }
        //将canny左右座标作为最终座标确定
        if ((int) avgl > line_l_max)
        {
            line_l = i;
            line_l_max = avgl;
        }

        if ((int) avgr > line_r_max)
        {
            line_r = cannyBImg.cols - i;
            line_r_max = avgr;
        }
    }

    //确定真的边缘位置   最左侧两条线
    realLines.push_back( { borderlines[0].l - 5 + line_l ,1,false } );
    realLines.push_back( { borderlines[0].l - 5 + line_r ,1,false } );

    //D区分界线
    realLines.push_back( { borderlines[2].l  ,3,false } );

    // B区处理结束
    //------------------------------------------------------------
#endif




#if AREAD2
    //==============================================================
    //D2区域处理开始
    //Mat AreaD2Img = cellImg[4];
    Mat AreaD2Img = imread( "Pic\\E\\area\\D2\\D2.png" );
    Mat grayD2Img, edgeD2Img, cannyD2Img, dstD2Img;

    cv::cvtColor( AreaD2Img, grayD2Img, CV_BGR2GRAY );
    cv::blur( grayD2Img, grayD2Img, Size( 7, 7 ) );

    //cv::Mat element = cv::getStructuringElement( cv::MORPH_RECT, cv::Size( 7, 7 ) );
    //cv::morphologyEx( grayD2Img, grayD2Img, cv::MORPH_CLOSE, element );

    // 局部二值化


    cv::adaptiveThreshold( grayD2Img, edgeD2Img, 255, CV_ADAPTIVE_THRESH_MEAN_C, CV_THRESH_BINARY_INV, 55, 20 );

    //threshold( grayD2Img, edgeD2Img, 96, 255, CV_THRESH_OTSU );  //直接找到结果？

    Mat kernel = getStructuringElement( MORPH_RECT, Size( 3, 3 ) );
    cv::morphologyEx( edgeD2Img, edgeD2Img, MORPH_OPEN, kernel );
    cv::morphologyEx( edgeD2Img, edgeD2Img, MORPH_CLOSE, kernel );


    std::vector<std::vector<Point>> contours;
    std::vector<Vec4i> hierarchy;

    cv::findContours( edgeD2Img, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE, Point( 0, 0 ) );

    Mat contoursImg = AreaD2Img.clone();
    //判断得到的轮廓值是否有效
    std::vector<std::vector<Point>> contours_filtered;
    if (contours.size() != 0)
    {
        // 计算所有轮廓的面积和长度
        vector<Rect> box( contours.size() );
        for (auto i = 0; i <contours.size(); ++i)
        {
            double contour_area = cv::contourArea( contours[i] );
            //计算外框矩形
            box[i] = boundingRect( Mat( contours[i] ) );

            //drawContours( contoursImg, contours, i, Scalar( 0, 0, 255 ), 1, 8, hierarchy );
            //rectangle( contoursImg, box[i], Scalar( 0, 128, 0 ), 1 );
            //double contour_length = cv::arcLength( *i, false );

            float acc = (float) box[i].height / box[i].width;

            float boxCenterX = ( box[i].tl().x + box[i].br().x ) / 2;
            //Point boxCenter = Point( ( box[i].tl.x + box[i].br.x ) / 2, ( box[i].tl.y + box[i].br.y ) / 2 );


            //选出面积小于10的且中心距离左边缘超过30px的
            if (( contour_area >= 10 && boxCenterX > 30 ))
                contours_filtered.push_back( contours[i] );

            //cout << contour_area << "    " << contour_length << endl;
        }
    }


    //最终结论
    if (contours_filtered.size() != 0)
    {
        cout << "D2区伤痕数量为：" << contours_filtered.size() << endl;

        // 计算所有轮廓的面积和长度
        vector<Rect> box( contours_filtered.size() );
        for (int i = 0; i <contours_filtered.size(); ++i)
        {
            //计算外框矩形
            box[i] = boundingRect( Mat( contours_filtered[i] ) );
            box[i] = Rect( Point( box[i].tl().x - 5, box[i].tl().y - 5 ), Point( box[i].br().x + 5, box[i].br().y + 5 ) );

            rectangle( contoursImg, box[i], Scalar( 0, 0, 255 ), 1 );

        }
        imwrite( "Pic\\E\\area\\D2\\res_d2.png", contoursImg );
    }

    else
        cout << "D2区没有伤痕!" << endl;

    //DE 分界线
    realLines.push_back( { borderlines[3].l  ,2,false } );
    //D2区域结束
    //=================================================================
#endif

#if AREAE
    //==============================================================
    //E区域处理开始
    //Mat AreaEImg = cellImg[3];
    Mat AreaEImg = imread( "Pic\\E\\area\\E\\E.png" );
    Mat grayEImg, edgeEImg, cannyEImg, dstEImg;

    cvtColor( AreaEImg, grayEImg, CV_BGR2GRAY );
    blur( grayEImg, grayEImg, Size( 5, 5 ) );

    //cv::Mat element = cv::getStructuringElement( cv::MORPH_RECT, cv::Size( 7, 7 ) );
    //cv::morphologyEx( grayEImg, grayEImg, cv::MORPH_CLOSE, element );
    //threshold( grayEImg, edgeEImg, 128, 255, CV_THRESH_BINARY_INV );  //直接找到结果？
    adaptiveThreshold( grayEImg, edgeEImg, 255, CV_ADAPTIVE_THRESH_MEAN_C, CV_THRESH_BINARY_INV, 25, 15 );

    Mat kernel= getStructuringElement( MORPH_RECT, Size( 3, 3 ) );
    morphologyEx( edgeEImg, edgeEImg, MORPH_OPEN, kernel );
    morphologyEx( edgeEImg, edgeEImg, MORPH_CLOSE, kernel );
    morphologyEx( edgeEImg, edgeEImg, MORPH_CLOSE, kernel );


    //erode( edgeEImg, edgeEImg, kernel );
    //dilate( edgeEImg, edgeEImg, kernel );

   // Canny( grayEImg, cannyEImg, 96, 160, 3 );

   // Mat labelImg;
    //connectedComponents( edgeEImg, labelImg );


    std::vector<std::vector<Point>> contours;
    std::vector<Vec4i> hierarchy;

    findContours( edgeEImg, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_NONE, Point( 0, 0 ) );

    Mat contoursImg = AreaEImg.clone();
    //判断得到的轮廓值是否有效
    std::vector<std::vector<Point>> contours_filtered;
    if (contours.size() != 0)
    {
        // 计算所有轮廓的面积和长度
        vector<Rect> box( contours.size() );
        for (auto i =0; i <contours.size(); ++i)
        {
            double contour_area = cv::contourArea( contours[i] );
            //计算外框矩形
            box[i] = boundingRect( Mat( contours[i] ) );

            //drawContours( contoursImg, contours, i, Scalar( 0,0,255 ), 1, 8, hierarchy );

            //rectangle( contoursImg, box[i], Scalar(0,128,0), 1 );
            //double contour_length = cv::arcLength( *i, false );

            //float acc = (float) box[i].height / box[i].width;

            float boxCenterX = ( box[i].tl().x + box[i].br().x ) / 2;
            //Point boxCenter = Point( ( box[i].tl.x + box[i].br.x ) / 2, ( box[i].tl.y + box[i].br.y ) / 2 );

            //选出面积小于10的且中心距离左边缘超过30px的
            if (contour_area >= 10&&( boxCenterX >30))
                contours_filtered.push_back( contours[i] );

            //cout << contour_area << "    " << contour_length << endl;
        }
    }



    //最终结论
    if (contours_filtered.size() != 0)
    {
        cout << "E区伤痕数量为：" << contours_filtered.size() << endl;

        // 计算所有轮廓的面积和长度
        vector<Rect> box( contours_filtered.size() );
        for (int i = 0; i <contours_filtered.size(); ++i)
        {
            //计算外框矩形
            box[i] = boundingRect( Mat( contours_filtered[i] ) );
            box[i] = Rect( Point( box[i].tl().x - 5, box[i].tl().y - 5 ), Point( box[i].br().x + 5, box[i].br().y + 5 ) );

            rectangle( contoursImg, box[i], Scalar( 0, 0, 255 ), 1 );

        }
        imwrite( "Pic\\E\\area\\E\\res_E.png", contoursImg );

    }
    else
        cout << "E区没有伤痕!" << endl;


    //Mat labelImg;
   // Two_PassNew( edgeEImg, labelImg );


    //Sobel( grayEImg, edgeEImg, CV_8U, 0, 1, 5 );
    //Canny( edgeEImg, cannyEImg, 30, 80, 3 );


    //EF 分界线
    realLines.push_back( { borderlines[4].l  ,2,false } );
    //E区域结束
    //=================================================================
#endif // AREAE


#if AREAF
    //Mat AreaFImg = cellImg[4];
    Mat AreaFImg = imread( "Pic\\E\\area\\F\\F.png" );
    Mat grayFImg, edgeFImg, cannyFImg, dstFImg;

    cv::cvtColor( AreaFImg, grayFImg, CV_BGR2GRAY );
    cv::blur( grayFImg, grayFImg, Size( 5, 5 ) );

    //cv::Mat element = cv::getStructuringElement( cv::MORPH_RECT, cv::Size( 7, 7 ) );
    //cv::morphologyEx( grayFImg, grayFImg, cv::MORPH_CLOSE, element );

    // 局部二值化


    cv::adaptiveThreshold( grayFImg, edgeFImg, 255, CV_ADAPTIVE_THRESH_MEAN_C, CV_THRESH_BINARY_INV, 35, 12 );

    //threshold( grayFImg, edgeFImg, 96, 255, CV_THRESH_OTSU );  //直接找到结果？

    Mat kernel = getStructuringElement( MORPH_RECT, Size( 3, 3 ) );
    cv::morphologyEx( edgeFImg, edgeFImg, MORPH_OPEN, kernel );
    cv::morphologyEx( edgeFImg, edgeFImg, MORPH_CLOSE, kernel );


    std::vector<std::vector<Point>> contours;
    std::vector<Vec4i> hierarchy;

    cv::findContours( edgeFImg, contours, hierarchy,CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE, Point( 0, 0 ) );

    Mat contoursImg = AreaFImg.clone();
    //判断得到的轮廓值是否有效
    std::vector<std::vector<Point>> contours_filtered;
    if (contours.size() != 0)
    {
        // 计算所有轮廓的面积和长度
        vector<Rect> box( contours.size() );
        for (auto i = 0; i <contours.size(); ++i)
        {
            double contour_area = cv::contourArea( contours[i] );
            //计算外框矩形
            box[i] = boundingRect( Mat( contours[i] ) );

            //drawContours( contoursImg, contours, i, Scalar( 0, 0, 255 ), 1, 8, hierarchy );

            //rectangle( contoursImg, box[i], Scalar( 0, 128, 0 ), 1 );
            //double contour_length = cv::arcLength( *i, false );

            float acc = (float) box[i].height / box[i].width;

            float boxCenterX = ( box[i].tl().x + box[i].br().x ) / 2;
            //Point boxCenter = Point( ( box[i].tl.x + box[i].br.x ) / 2, ( box[i].tl.y + box[i].br.y ) / 2 );


            //选出面积小于10的且中心距离左边缘超过30px的
            if (contour_area >= 15 )
                contours_filtered.push_back( contours[i] );

            //cout << contour_area << "    " << contour_length << endl;
        }
    }


    //最终结论
    if (contours_filtered.size() != 0)
    {
        cout << "F区伤痕数量为：" << contours_filtered.size() << endl;

        // 计算所有轮廓的面积和长度
        vector<Rect> box( contours_filtered.size() );
        for (int i = 0; i <contours_filtered.size(); ++i)
        {
            //计算外框矩形
            box[i] = boundingRect( Mat( contours_filtered[i] ) );
            box[i] = Rect( Point( box[i].tl().x - 5, box[i].tl().y - 5 ), Point( box[i].br().x + 5, box[i].br().y + 5 ) );

            rectangle( contoursImg, box[i], Scalar( 0, 0, 255 ), 1 );

        }
        imwrite( "Pic\\E\\area\\F\\res_f.png",contoursImg );

    }

    else
        cout << "F区没有伤痕!" << endl;




#endif // AREAF




    //FG 分界线
    realLines.push_back( { borderlines[5].l  ,2,false } );


    ////绘制出来原始裁剪边线
    //dstBImg = cannyBImg.clone();
    //line( dstBImg, Point( 5, 0 ), Point( 5, dstBImg.rows - 1 ),Scalar(128));
    //line( dstBImg, Point( dstBImg.cols-6, 0 ), Point( dstBImg.cols - 6, dstBImg.rows - 1 ), Scalar( 128 ) );
 

 

#if AREAG
    //---- G 区域处理方案------------------------------------------


    //Mat AreaGImg = cellImg[5];
    Mat AreaGImg = imread( "Pic\\E\\area\\G\\G3.png" );
    Mat grayGImg, edgeGImg, cannyGImg, dstGImg;

    cvtColor( AreaGImg, grayGImg, CV_BGR2GRAY );

    cv::blur( grayGImg, grayGImg, Size( 5, 5 ) );

    threshold( grayGImg, edgeGImg, 0, 255, CV_THRESH_OTSU );

    Mat kernel = getStructuringElement( MORPH_CROSS, Size( 3, 3 ) );
   
    cv::morphologyEx( edgeGImg, edgeGImg, MORPH_CLOSE, kernel );

    Canny( edgeGImg, cannyGImg, 30, 100, 3 );

    ////对得到的两条轮廓统计横座标值，确定轮廓位置
    ////缩小计算长度  可以
    //vector<int> avgX;
    //for (int i = 0; i <cannyGImg.cols; i++)
    //{
    //    float avg=0;
    //    for (int j = 0;j < cannyGImg.rows;j++)
    //    {
    //        avg += (float) cannyGImg.at<uchar>( j, i ) / cannyGImg.rows;
    //    }
    //    avgX.push_back((int)avg );
    //}
    //
    ////取出适宜长度的内容  80-200   得到的位置是乱序的不影响使用
    //vector<int> avgXl;
    //avgXl.insert( avgXl.begin(), avgX.begin() + 80, avgX.begin() + 200 );
    //int pos[7];
    //for (int i = 0;i < 6;i++)
    //{
    //    auto maxl = max_element( avgXl.begin(),avgXl.end() );
    //    pos[i] = distance( avgXl.begin(), maxl );
    //    avgXl[pos[i]] = 0;              //置零      
    //    pos[i] += 80;
    //}
    //
    ////末尾最后一个边界值
    //vector<int> avgXr;
    //avgXr.insert( avgXr.begin(), avgX.begin() + 270, avgX.end() );
    //auto maxr = max_element( avgXr.begin(), avgXr.end() );
    //pos[6] = distance( avgXr.begin(), maxr );
    //pos[6] += 270;

    //qsort( pos, 7, sizeof( pos[0] ), comP );


    std::vector<AreaRDiv::Line> lines;
    //对于得到边缘的图像的每一列进行计数， 当一列的白色点数目大于300时便认为检测到直线
    for (auto i = 0; i < cannyGImg.cols; ++i)
    {
        int c = 0;
        for (auto j = 0; j < cannyGImg.rows; ++j)
            c = ( cannyGImg.at<unsigned char>( j, i ) == 255 ) ? c + 1 : c;

        //d对于大于300个白点的列进行 加入line中
        if (c > 400)
            lines.push_back( { i,c / double( cannyGImg.rows ),true } );
    }

    //line 至少7条线 如果少了 报错
    if (lines.size() < 7)
    {
        cout << "G区无法读取到7条边缘， 请检查图片！" << endl;
        return false;
    }
    //对于对于相邻的两条线进行取大值处理
    for (int i = 0; i < lines.size()-1; )
    {
        if (fabs( lines[i].l - lines[i + 1].l ) < 5)
        {
            if (lines[i].acc >= lines[i + 1].acc)
                lines.erase( lines.cbegin() + i + 1 );
            else
                lines.erase( lines.cbegin() + i );
        }
        else
            ++i;
    }

    std::vector<AreaRDiv::Line> lines_filter;
    //判断lines的范围 根据范围确定7条线
    for (int i = 0; i < lines.size(); i++)
    {
        int l = lines[i].l;
        
        //在槽的范围内的线 不做考虑
        //准确定位线的位置之后，定位都很精确
        if (( l > 103 && l < 117 )
            || ( l > 140 && l < 153 )
            || ( l > 177 && l < 189 ))
            continue;
        else
            lines_filter.push_back( lines[i] );
    }



    ////绘出最终的线
    //for (int i = 0; i < lines_filter.size();i++ )
    //{
    //    line( cannyGImg, Point( lines_filter[i].l, 0 ), Point( lines_filter[i].l, cannyGImg.rows - 1 ), Scalar( 128 ) );
    //}


    //判断线的个数，过多则返回失败
    if (lines_filter.size() == 7)
    {
        for (int i = 0;i < 7;i++)
        {
            realLines.push_back( { borderlines[5].l - 5 + lines_filter[i].l ,1,false } );
        }
    }
    else
       cout<<"G区检测到的线过多, 请检查"<<endl;

    
    //在开始的地方插入一个量
    lines_filter.insert( lines_filter.begin(), { 5,1,false } );

    vector<Mat> GcellImg;
    cutImage( AreaGImg, lines_filter, GcellImg );


    
#endif



    //绘制出各条线
    ard.drawLines( srcRImg, realLines, dstImg );

    ////创建并绘制水平投影图像
    //cv::Mat projImg( 255, cannyGImg.cols, CV_8U, cv::Scalar( 255 ) );

    //for (int i = 0; i < cannyGImg.cols; ++i)
    //{
    //    cv::line( projImg, cv::Point( i, 128 - avgX[i] ), cv::Point( i, 128 ), cv::Scalar::all( 0 ) );
    //}

    //Mat tmp, tmp2;
    //bitwise_not( projImg, tmp );
    //resize( tmp, tmp2, cannyGImg.size() );
    //addWeighted( tmp2, 0.3, cannyGImg, 0.7, 0, dstImg );


    //realLines.push_back( { borderlines[0].l - 5 + line_l ,2,false } );
    //realLines.push_back( { borderlines[1].l - 5 + line_r ,2,false } );

    ////绘制出来原始裁剪边线
    //dstGImg = cannyGImg.clone();
    //line( dstGImg, Point( 5, 0 ), Point( 5, dstGImg.rows - 1 ), Scalar( 128 ) );
    //line( dstGImg, Point( dstGImg.cols - 6, 0 ), Point( dstGImg.cols - 6, dstGImg.rows - 1 ), Scalar( 128 ) );




    waitKey( 0 );

        





#endif



    finish = clock();
    totaltime = (double) ( finish - start ) / CLOCKS_PER_SEC;
    cout << "\n此程序的运行时间为" << totaltime << "秒" << endl;

    getchar();
    getchar();


    return 0;
}



