#include "..\include\include.h"

AreaRDiv::AreaRDiv()
{

}

AreaRDiv::~AreaRDiv()
{

}

// 分割区域
bool AreaRDiv::divide(const cv::Mat &img, const std::vector<double> &scales, std::vector<Line> &lines)const
{

    return true;
}




void AreaRDiv::drawLines(const cv::Mat &img, const std::vector<Line> &lines, cv::Mat &imgOut)const


{
    cv::Mat img_rgb = img.clone();
    if (img.channels() == 1)
        cv::cvtColor( img, img_rgb, CV_GRAY2BGR );

    //根据acc值进行画不同颜色的线 小于等于1 绿线/   2 蓝线     其他 红线
    for (auto i : lines)
    {
        if (i.acc <= 1)
            cv::line( img_rgb, cv::Point( i.l, 0 ), cv::Point( i.l, img_rgb.rows - 1 ), cv::Scalar( 0, 255, 0 ) );
        else if (i.acc == 2)
            cv::line( img_rgb, cv::Point( i.l, 0 ), cv::Point( i.l, img_rgb.rows - 1 ), cv::Scalar( 255, 0, 0 ) );
        else
            cv::line( img_rgb, cv::Point( i.l, 0 ), cv::Point( i.l, img_rgb.rows - 1 ), cv::Scalar( 0, 0, 255 ) );

    }
    imgOut = img_rgb;
}



//图像预处理
bool AreaRDiv::preProc(const cv::Mat &srcImg, cv::Mat &dstImg) 
{
    Mat grayImg;
    if (srcImg.channels() != 1)
    {
        cvtColor(srcImg,grayImg,CV_BGR2GRAY);
    }

    //Canny(grayImg,dstImg,50,100,3);
    GaussianBlur(grayImg, grayImg, Size(5, 5), 1, 0);
    //缩放插值图像
    pyrDown(grayImg, grayImg);
    pyrUp(grayImg, grayImg);



    Mat cannyImg;
    Canny(grayImg, cannyImg, 10, 80, 3); //实验参数，测试用

   // cv::dilate(cannyImg, cannyImg, cv::Mat());
   // cv::erode(cannyImg, cannyImg, cv::Mat());



    dstImg = cannyImg.clone();

   // waitKey(0);
    return true;
}



void AreaRDiv::detectLines(const cv::Mat &cannyImg, std::vector<Line> &lines)const
{

     //对于得到边缘的图像的每一列进行计数， 当一列的白色点数目大于300时便认为检测到直线
    for (auto i = 0; i < cannyImg.cols; ++i)
    {
        int c = 0;
        for (auto j = 0; j < cannyImg.rows; ++j)
            c = (cannyImg.at<unsigned char>(j, i) == 255) ? c + 1 : c;

        //d对于大于300个白点的列进行 加入line中
        if (c > 300)
            lines.push_back({ i,c / double(cannyImg.rows),true });
    }

}

//将检测到的边缘线进行滤波
bool AreaRDiv::filterLines(const std::vector<Line> &lines_in, std::vector<Line> &lines_out, const std::vector<double> &scales)const
{
    //如果两条线之间间距小于5 则擦除较短的那条
    std::vector<Line> lines = lines_in;
    for (int i = 0; i < lines.size() - 1; )
    {
        if (fabs(lines[i].l - lines[i + 1].l) < 5)
        {
            if (lines[i].acc >= lines[i + 1].acc)
                lines.erase(lines.cbegin() + i + 1);
            else
                lines.erase(lines.cbegin() + i);
        }
        else
            ++i;
    }

    //将边缘线加入滤波后lines中
    //for (auto  it = lines.cbegin(); it < lines.cend(); it++)
        //lines_out.push_back(*it);
    //暂时只把最左侧的线画出来
    lines_out.push_back(*lines.cbegin());

    for (auto i = 0;i < scales.size();i++)
    {
        lines_out.push_back({ lines_out[0].l + (int)scales[i],
            1,
            false });
    }


    return true;
}


//绘制scars
void AreaRDiv::drawScars(const cv::Mat &img, const std::vector<std::vector<cv::Point>> &scars, cv::Mat &img_out)
{
    cv::Mat img_rgb = img.clone();
    if (img.channels() == 1)
        cv::cvtColor(img, img_rgb, CV_GRAY2BGR);

    //绘制轮廓
    vector<Rect> bound(scars.size());
    for (int i = 0; i < scars.size(); i++)
    {
        //drawContours(img_rgb, scars, i, Scalar(0, 255, 255), 1, 8);

        bound[i] = boundingRect(Mat(scars[i]));
        rectangle(img_rgb, Point(bound[i].tl().x-5, bound[i].tl().y -5), Point(bound[i].br().x + 5, bound[i].br().y + 5), Scalar(0, 0, 255), 1, 8, 0);
        

    }


    //for (auto i = scars.cbegin(); i != scars.cend(); ++i)
    //    for (auto j = i->cbegin(); j != i->cend(); ++j)
    //        img_rgb.at<cv::Vec3b>(j->y, j->x) = { 0,0,255 };

    img_out = img_rgb.clone();


}

std::vector<std::vector<Point>> contours;
std::vector<Vec4i> hierarchy;


void AreaRDiv::detectScars(const cv::Mat &img, std::vector<std::vector<cv::Point>> &scars, const std::vector<AreaRDiv::Line> &lines)const
{
    //查找到粗轮廓 采集到的最外层轮廓，保留轮廓的所有信息
    findContours(img, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE, Point(0, 0));

    /*
    Mat drawImg = Mat::zeros(img.size(), CV_8UC3);
    for (auto i = 0; i <contours.size(); i++)
    {

        Scalar color = Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));
        //drawContours(drawImg, contours, i, color, CV_FILLED,8, hierarchy, 0, Point());

        cout << contourArea(contours[i]) << "   " << arcLength(contours[i],false) << endl;

    }

    */

    // 计算所有轮廓的面积和长度
    for (auto i = contours.cbegin(); i != contours.cend(); ++i)
    {
        double contour_area = cv::contourArea(*i);
        double contour_length = cv::arcLength(*i, false);

        //伤痕面积面积大于10 或者长度在10-1000范围内内
        if (contour_area > 10 || (contour_length>=10 && contour_length<=500))
            scars.push_back(*i);

        //cout << contour_area << "    " << contour_length << endl;


    }

 
}


void AreaRDiv::filterScars(const cv::Mat &img, const std::vector<std::vector<cv::Point>> &scars_in, std::vector<std::vector<cv::Point>> &scars_out, const std::vector<AreaRDiv::Line> &lines)const
{


    //对每一个边缘绘出最小矩形 对于最小矩形的长宽进行比值得到
    //vector<Scar> scars_filter(scars_in.size());
    vector<RotatedRect> box(scars_in.size());
    for (auto i = 0;i <scars_in.size();i++)
    {
        box[i] = minAreaRect(Mat (scars_in[i]));

        //在边缘宽度不为0 的情况下
        //cout << box[i].size.width << "   "<< box[i].size.height <<  "   "<< (box[i].size.height + 0.001) / (box[i].size.width + 0.001)<< endl;
        if((box[i].size.height != 0) || (box[i].size.height != 0))
            if (((box[i].size.height+0.001) / (box[i].size.width+0.001)) <20)
                scars_out.push_back(scars_in[i]);


        //if ((box[i].center.x > lines[0].l + 30))
        //{
        //    continue;
        //}
        //else
        //{
        //    bool edgesC(scars_in[i].size());
        //    const unsigned char *data = img.ptr<unsigned char>(i);
        //    vector<cv::Point> edgeContours;

        //    for (auto j = scars_in[i].cbegin; j < scars_in[i].cend;j++)
        //    {
        //        if (j->x - lines[0].l > 3)
        //            edgesC[j] = true;
        //        else
        //            edgesC[j] = false;
        //    }

        //    for (auto j =0; j < scars_in[i].size();j++)
        //    {
        //        
        //    }

        //}



    }


    //观察左侧边缘处的缺陷 将图像左侧边线的附近30px 宽度选出来
    Mat imgROI = img(Range(0, img.rows), Range(lines[0].l - 30, lines[0].l + 30));



    //for (auto it = scars_in.cbegin();it < scars_in.cend();it++)
    //{

    //    

    //    for (auto j = it->cbegin();j < it->cend();j++)
    //    {

    //    }

    //}

    







}


//杆面图像区域划分
void AreaRDiv::divideImg( Mat &srcImg, Mat &dstImg, vector<double> &borderScales, vector<AreaRDiv::Line> &borderlines)
{



    Mat grayImg, edgeImg, cannyImg;


    cvtColor( srcImg, grayImg, CV_BGR2GRAY );

    blur( grayImg, grayImg, Size( 7, 7 ) );


    //grayImg.convertTo( grayImg, CV_16S );

    //Sobel( grayImg, edgeImg, CV_16S, 1,0,5 );
    //Laplacian( grayImg, edgeImg, CV_32F, 5 );

    //统计列向的平均数，可以分开计算 计算最前面和最后面的头尾 减少大部分运算量
    vector<int> sumY{};
    //double *sumY = new double[grayImg.cols];
    for (int i = 0;i < grayImg.cols;++i)
    {
        double avg = 0;
        for (int j = 0;j < grayImg.rows;j++)
        {
            //cout << grayImg.at<uchar>( j, i )<<"  "<<i<< endl;
            avg += (double) grayImg.at<uchar>( j, i ) / grayImg.rows;
        }
        sumY.push_back( (int)avg );
        //sumY[i] = sum;
    }



    //计算横向的梯度
    vector<int> gradX{};
    gradX.push_back( 0 );
    for (int i = 1;i < grayImg.cols;i++)
    {
        gradX.push_back( 10 * ( sumY[i] - sumY[i - 1] ) );

        //cout << gradX[i] << endl;
    }

    //遍历得到图像梯度的两个峰值，得到两个边缘的值  左侧选 0-150 右侧选1900-2048
    int max = 0, max_l = 0;
    for (int i = 0;i < 150;i++)
    {
        if (gradX[i] > max)
        {
            max = gradX[i];
            max_l = i;
        }
    }
    //cout << max << "  " << max_l << endl;


    //从第一个峰值开始找到30px 内最小值 低谷
    int min1 = 100, min1_l = 0;
    for (int i = max_l;i < max + 300;i++)
    {
        if (gradX[i] < min1)
        {
            min1 = gradX[i];
            min1_l = i;
        }
    }
    //cout << min1 << "  " << min1_l << endl;


    //逆向找到第一个峰值  E面找低估
    int min = 128, min_l = 0;
    for (int i = grayImg.cols - 1;i >grayImg.cols - 121;i--)
    {
        if (gradX[i] < min)
        {
            min = gradX[i];
            min_l = i;
        }
    }
    //cout << min << "  " << min_l << endl;

    

    max_l = max_l - 2;
    //最左侧边线 手动
    borderlines.push_back( { max_l, 1, false } );

    //最第二条边线 手动
    borderlines.push_back( { min1_l, 1, false } );

    int maxWidthpx = abs( min_l - max_l );

    //划分区域边线
    for (int i = 2;i < borderScales.size() - 1;i++)
    {
        borderlines.push_back( { max_l + (int) ( borderScales[i] * maxWidthpx ), 1, false } );
    }
    //最右侧边线
    borderlines.push_back( { min_l, 1, false } );

    AreaRDiv ard;

    //ard.drawLines( srcImg, borderlines, dstImg );

/*
    //创建并绘制水平投影图像
    cv::Mat projImg( 255, grayImg.cols, CV_8U, cv::Scalar( 255 ) );

    for (int i = 0; i < grayImg.cols; ++i)
    {
    cv::line( projImg, cv::Point( i, 128 - gradX[i] ), cv::Point( i, 128 ), cv::Scalar::all( 0 ) );
    }

    Mat tmp, tmp2;
    bitwise_not( projImg, tmp );
    resize( tmp, tmp, grayImg.size() );
    addWeighted( tmp, 0.3, grayImg, 0.7, 0, dstImg );

    //imwrite( "Pic\\R\\Test_grad.png", tmp2 );


    

    //edgeImg.convertTo( edgeImg, CV_8U );
    //threshold( edgeImg, edgeImg, 100, 255, CV_THRESH_OTSU );


    //Sobel( grayImg, dstImg, CV_8U, 1, 0, 5 );
    //Canny( grayImg, cannyImg, 30, 60, 3 );

    vector<Mat> cellImg;
    cutImage( srcRImg,borderlines, cellImg );

    Mat AreaBImg, AreaD2Img, AreaEImg, AreaFImg, AreaGImg;
    AreaBImg = cellImg[0];
    AreaD2Img = cellImg[2];
    AreaEImg = cellImg[3];
    AreaFImg = cellImg[4];
    AreaGImg = cellImg[5];

    */
}

//弧面区域划分
void AreaRDiv::RdivideImg( Mat &srcImg, Mat &dstImg, vector<double> &RborderScales )
{

    //Mat srcImg = imread( RFace0 );
    //Mat dstImg;


    //imshow( "src", srcRImg );
    Mat grayImg, edgeImg, cannyImg;


    cvtColor( srcImg, grayImg, CV_BGR2GRAY );

    blur( grayImg, grayImg, Size( 5, 5 ) );


    //统计列向的平均数，
    vector<double> sumY{};
    for (int i = 0;i < grayImg.cols;++i)
    {
        double sum = 0;
        for (int j = 0;j < grayImg.rows;j++)
        {
            sum += (double) grayImg.at<uchar>( j, i ) / grayImg.rows;
        }
        sumY.push_back( sum );
    }

    //计算横向的梯度
    vector<int> gradX{};
    gradX.push_back( 0 );
    for (int i = 1;i < grayImg.cols;i++)
    {
        gradX.push_back( (int) 10 * ( sumY[i] - sumY[i - 1] ) );
        //cout << gradX[i] << endl;
    }

    //遍历得到图像梯度的两个峰值，得到两个边缘的值  左侧选 0-180 右侧选倒数250
    int max = 0, max_l = 0;  //最左侧边缘，最第二条边缘
    for (int i = 0;i < 180;i++)
    {
        if (gradX[i] > max)
        {
            max = gradX[i];
            max_l = i;
        }


    }
    //cout << max << "  " << max_l << endl;
    //从第一个峰值开始找到100px 内最小值 低谷
    int min = 100, min_l = 0;
    for (int i = max_l;i < max + 100;i++)
    {
        if (gradX[i] < min)
        {
            min = gradX[i];
            min_l = i;
        }
    }
    //cout << min << "  " << min_l << endl;

    //逆向找到第一个峰值  R 面处是找到峰值
    int max2 = 0, max2_l = 0;
    for (int i = grayImg.cols - 1;i >grayImg.cols - 251;i--)
    {
        //再次找到一个峰值
        if (gradX[i] > max2)
        {
            max2 = gradX[i];
            max2_l = i;
        }
    }
    //cout << max2 << "  " << max2_l << endl;

    vector<AreaRDiv::Line> Rborderlines;

    //最左侧边线 手动左移动2px  右侧右移1px
    max_l -= 0;
    max2_l += 0;

    Rborderlines.push_back( { max_l, 1, false } );
    Rborderlines.push_back( { min_l, 1, false } );

    int maxWidthpx = abs( max2_l - max_l );

    //划分区域边线
    for (int i = 2;i < RborderScales.size() - 1;i++)
    {
        Rborderlines.push_back( { max_l + (int) ( RborderScales[i] * maxWidthpx ), 1, false } );
    }
    //最右侧边线
    Rborderlines.push_back( { max2_l, 1, false } );

    //AreaRDiv ard;

    //ard.drawLines( srcImg, Rborderlines, dstImg );


    //创建并绘制水平投影图像
    cv::Mat projImg( 255, grayImg.cols, CV_8U, cv::Scalar( 255 ) );

    for (int i = 0; i < grayImg.cols; ++i)
    {
        cv::line( projImg, cv::Point( i, 128 - gradX[i] ), cv::Point( i, 128 ), cv::Scalar::all( 0 ) );
    }

    Mat tmp, tmp2;
    bitwise_not( projImg, tmp );
    resize( tmp, tmp, grayImg.size() );
    addWeighted( tmp, 0.3, grayImg, 0.7, 0, dstImg );
    //imwrite( "Pic\\R\\Test_grad.png", tmp2 );



    //edgeImg.convertTo( edgeImg, CV_8U );
    //threshold( edgeImg, edgeImg, 100, 255, CV_THRESH_OTSU );


    //Sobel( grayImg, dstImg, CV_8U, 1, 0, 5 );
    //Canny( grayImg, cannyImg, 30, 60, 3 );

    AreaRDiv ard;
    ard.drawLines( srcImg, Rborderlines, dstImg );

    //vector<Mat> cellImg;
    //cutImage( srcImg,Rborderlines, cellImg );

    //Mat AreaCImg, AreaD1Img, AreaD0Img, AreaD2Img;
    //AreaCImg = cellImg[0];
    //AreaD1Img = cellImg[1];
    //AreaD0Img = cellImg[2];
    //AreaD2Img = cellImg[3];

}









//目录中的所有图片(到每一级目录)
void listFiles(const char * dir, vector<string>& files)
{
    char dirNew[200];
    strcpy(dirNew, dir);
    strcat(dirNew, "\\*.*");    // 在目录后面加上"\\*.*"进行第一次搜索
    intptr_t handle;
    _finddata_t findData;
    handle = _findfirst(dirNew, &findData);
    if (handle == -1)        // 检查是否成功
        return;
    do
    {
        if (findData.attrib & _A_SUBDIR)
        {
            if (strcmp(findData.name, ".") == 0 || strcmp(findData.name, "..") == 0)
                continue;
            //cout << findData.name << "\t<dir>\n";
            // 在目录后面加上"\\"和搜索到的目录名进行下一次搜索
            strcpy(dirNew, dir);
            strcat(dirNew, "\\");
            strcat(dirNew, findData.name);
            listFiles(dirNew, files);
        }
        else
            files.push_back(string(dir).append("\\").append(findData.name));
        //cout << findData.name << "\t" << findData.size << " bytes.\n";
    } while (_findnext(handle, &findData) == 0);
    _findclose(handle);    // 关闭搜索句柄
}


void renameFiles( string &filespath, string &newname,string &newpath  )
{


    //定位出来文件名
    int pos = filespath.find_last_of( '\\' );
    int pos_ = filespath.find( '.' );
    string path( filespath.substr( 0, pos + 1 ) );   // 路径
    string name( filespath.substr( pos + 1, pos_- pos-1 ) ); //文件名
    string exte( filespath.substr( pos_+1, 3 ) ); //拓展名

    string suffix = "png";

    //设定文件名格式
    newname = newpath+"\\" +"res_" + name +"."+ suffix;

   // files_rename = 



    ////字符型路径分割
    //char drive[_MAX_DRIVE];
    //char dir[_MAX_DIR];
    //char fname[_MAX_FNAME];
    //char ext[_MAX_EXT];
    //_splitpath( filespath, drive, dir, fname, ext );


}





//将图像划分成为五个单元
void cutImage( cv::Mat &srcImg, std::vector<AreaRDiv::Line> &lines, std::vector<cv::Mat> &cellImg )
{
    int height = srcImg.rows;


    ////设置ROI总会在第五个区域失效无法进行下一步
    //for (int i = 0;i < lines.size()-1;i++)
    //{

    //    Mat roi_img(srcImg, Rect( lines[i].l - 5, 0, lines[i+1].l- lines[i].l + 10, height ) );

    //    cellImg.push_back( roi_img );
    //}

    Mat roi_img;

    for (int i = 0;i < lines.size()-3;i++)
    {

        Rect rect( lines[i].l - 5, 0, lines[i + 1].l - lines[i].l + 10, height );
        //在第四个总是出问题  
        //应该是roi区域变小导致无法处理
        srcImg( rect ).copyTo( roi_img );
        cellImg.push_back( roi_img );
        //imshow( "roi_img", roi_img );

    }

    

    // 强制将数据推出， 解决之前的bug
    Mat roi_img4;
    Rect rect4( lines[4].l - 5, 0, lines[5].l - lines[4].l + 10, height );
    //在第四个总是出问题
    srcImg( rect4 ).copyTo( roi_img4 );
    cellImg.push_back( roi_img4 );

    Mat roi_img5;
    Rect rect5( lines[5].l - 5, 0, lines[6].l - lines[5].l + 10, height );
    //在第四个总是出问题
    srcImg( rect5 ).copyTo( roi_img5 );
    cellImg.push_back( roi_img5 );


}


//将G区图像单独划分
void GcutImage( cv::Mat &srcImg, std::vector<AreaRDiv::Line> &lines, std::vector<cv::Mat> &cellImg )
{
    int height = srcImg.rows;

    //设置ROI总会在第五个区域失效无法进行下一步
    for (int i = 0;i < lines.size()-1;i++)
    {

        Mat roi_img(srcImg, Rect( lines[i].l - 5, 0, lines[i+1].l- lines[i].l + 10, height ) );

        cellImg.push_back( roi_img );
    }

}



//------------------------------【两步法新改进版】----------------------------------------------
// 两步法求连通域  可见链接 
// https://blog.csdn.net/qq_37059483/article/details/78018539 
// https://blog.csdn.net/icvpr/article/details/10259577
// 对二值图像进行连通区域标记,从1开始标号
void  Two_PassNew( const Mat &bwImg, Mat &labImg )
{
    assert( bwImg.type() == CV_8UC1 );
    labImg.create( bwImg.size(), CV_32SC1 );   //bwImg.convertTo( labImg, CV_32SC1 );
    labImg = Scalar( 0 );
    labImg.setTo( Scalar( 1 ), bwImg );
    assert( labImg.isContinuous() );
    const int Rows = bwImg.rows - 1, Cols = bwImg.cols - 1;
    int label = 1;
    vector<int> labelSet;
    labelSet.push_back( 0 );
    labelSet.push_back( 1 );
    //the first pass
    int *data_prev = (int*) labImg.data;   //0-th row : int* data_prev = labImg.ptr<int>(i-1);
    int *data_cur = (int*) ( labImg.data + labImg.step ); //1-st row : int* data_cur = labImg.ptr<int>(i);
    for (int i = 1; i < Rows; i++)
    {
        data_cur++;
        data_prev++;
        for (int j = 1; j<Cols; j++, data_cur++, data_prev++)
        {
            if (*data_cur != 1)
                continue;
            int left = *( data_cur - 1 );
            int up = *data_prev;
            int neighborLabels[2];
            int cnt = 0;
            if (left>1)
                neighborLabels[cnt++] = left;
            if (up > 1)
                neighborLabels[cnt++] = up;
            if (!cnt)
            {
                labelSet.push_back( ++label );
                labelSet[label] = label;
                *data_cur = label;
                continue;
            }
            int smallestLabel = neighborLabels[0];
            if (cnt == 2 && neighborLabels[1]<smallestLabel)
                smallestLabel = neighborLabels[1];
            *data_cur = smallestLabel;
            // 保存最小等价表
            for (int k = 0; k<cnt; k++)
            {
                int tempLabel = neighborLabels[k];
                int& oldSmallestLabel = labelSet[tempLabel];  //这里的&不是取地址符号,而是引用符号
                if (oldSmallestLabel > smallestLabel)
                {
                    labelSet[oldSmallestLabel] = smallestLabel;
                    oldSmallestLabel = smallestLabel;
                }
                else if (oldSmallestLabel<smallestLabel)
                    labelSet[smallestLabel] = oldSmallestLabel;
            }
        }
        data_cur++;
        data_prev++;
    }
    //更新等价队列表,将最小标号给重复区域
    for (size_t i = 2; i < labelSet.size(); i++)
    {
        int curLabel = labelSet[i];
        int prelabel = labelSet[curLabel];
        while (prelabel != curLabel)
        {
            curLabel = prelabel;
            prelabel = labelSet[prelabel];
        }
        labelSet[i] = curLabel;
    }
    //second pass
    data_cur = (int*) labImg.data;
    for (int i = 0; i < Rows; i++)
    {
        for (int j = 0; j < bwImg.cols - 1; j++, data_cur++)
            *data_cur = labelSet[*data_cur];
        data_cur++;
    }
}



