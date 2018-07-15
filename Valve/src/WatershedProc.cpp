#include "..\include\include.h"

WaterShedProc::WaterShedProc()
{
}

WaterShedProc::~WaterShedProc()
{
}

//将图像markers 转换成为整型
void WaterShedProc::setMarkers( const Mat & markerImage )
{
    markerImage.convertTo( markers, CV_32S );
}

//将需要处理的图片进行前处理 
void WaterShedProc::preProc(const Mat &srcImg, Mat &dstImg)
{
    Mat grayImg, edgeImg;
    
    if (srcImg.channels() != 1)
    {
        cvtColor( srcImg, grayImg, CV_BGR2GRAY );
    }

    GaussianBlur( grayImg, grayImg, Size( 5, 5 ), 0 );

    //二值化之后取反 将需要获取的部分置为白色
    threshold( grayImg, edgeImg, 0, 255, CV_THRESH_OTSU );
    bitwise_not( edgeImg, dstImg );

}

//执行watershed 程序生成markers
void WaterShedProc::process( const Mat &srcImg ,Mat &markers )
{
    //前处理得到边缘图像
    Mat edgeImg;
    preProc( srcImg, edgeImg );

    //三次膨胀得到图片背景的确定区域，此区域为不需要的部分
    Mat sure_bg, sure_fr;
    Mat kernel = getStructuringElement( MORPH_RECT, Size( 3, 3 ) );

    dilate( edgeImg, sure_bg, kernel, Point( -1, -1 ), 3 );

    //距离变换生成距离图像，二值化得到区域核心 生成种子
    Mat distImg;
    distanceTransform( edgeImg, distImg, DIST_C, 3 );
    distImg.convertTo( distImg, CV_8U );
    
    //距离边界超过1的都算内部
    threshold( distImg, sure_fr, 1, 255, CV_THRESH_BINARY );

    Mat unknownImg;
    subtract( sure_bg, sure_fr, unknownImg );

    connectedComponents( sure_fr, markers );

    //将markers 中背景的部分全部设置为1 避免影响灌水
    markers = markers + 1;

    //将markers中unknown的部分置为0 进行下一步灌水
    for (int i = 0;i < unknownImg.rows;i++)
    {
        unsigned char *data = unknownImg.ptr<unsigned char>( i );
        int *dataM = markers.ptr< int>( i );
        for (int j = 0;j < unknownImg.cols;j++)
        {
            if (data[j] == 255)
            {
                dataM[j] = 0;
            }
        }
    }

    watershed( srcImg, markers );


}

void WaterShedProc::drawMarkersLine( cv::Mat &dstImg, const cv::Mat &markers)
{
   
    //将marker中的区域绘制在原图上
    for (auto i = 0;i < markers.rows;i++)
    {

        for (auto j = 0;j < markers.cols;j++)
        {
            if (markers.at<int>( i, j ) == -1)
            {
                //dataM[j] =255;
                dstImg.at<cv::Vec3b>( i, j ) = Vec3b( 0, 0, 255 );
            }
        }
    }

}

Vec3b RandomColor( int value )//生成随机颜色函数
{
    value = value % 255;  //生成0~255的随机数  
    RNG rng;
    int aa = rng.uniform( 0, value );
    int bb = rng.uniform( 0, value );
    int cc = rng.uniform( 0, value );
    return Vec3b( aa, bb, cc );
}

void WaterShedProc::drawMarkersCon( cv::Mat &dstImg, const cv::Mat &markers )
{
    //对每一个区域进行颜色填充
    Mat PerspectiveImage = Mat::zeros( dstImg.size(), CV_8UC3 );
    for (int i = 0;i<markers.rows;i++)
    {
        for (int j = 0;j<markers.cols;j++)
        {
            int index = markers.at<int>( i, j );
            if (markers.at<int>( i, j ) == -1)
            {
                PerspectiveImage.at<Vec3b>( i, j ) = Vec3b( 255, 255, 255 );
            }
            else
            {
                PerspectiveImage.at<Vec3b>( i, j ) = RandomColor( index );
            }
        }
    }

    //分割并填充颜色的结果跟原始图像融合
    addWeighted( dstImg, 0.4, PerspectiveImage, 0.6, 0, dstImg );

}







































