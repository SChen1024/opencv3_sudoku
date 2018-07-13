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
        cv::cvtColor(img, img_rgb, CV_GRAY2BGR);

    for (auto i : lines)
        if (i.aut)
            cv::line(img_rgb, cv::Point(i.l, 0), cv::Point(i.l, img_rgb.rows - 1), cv::Scalar(0, 255, 0));
        else
            cv::line(img_rgb, cv::Point(i.l, 0), cv::Point(i.l, img_rgb.rows - 1), cv::Scalar(255, 0, 0));

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



    for (auto it = scars_in.cbegin();it < scars_in.cend();it++)
    {

        

        for (auto j = it->cbegin();j < it->cend();j++)
        {

        }

    }

    







}


