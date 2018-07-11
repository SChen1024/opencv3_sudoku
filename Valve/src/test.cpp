#include <ros/ros.h>
#include <cv_bridge/cv_bridge.h>
#include <sensor_msgs/Image.h>
#include <sensor_msgs/image_encodings.h>
#include <std_msgs/String.h>

#include <srvs/Images.h>
#include <srvs/SerPortSignal.h>

#include <opencv2/opencv.hpp>
#include <string>

#include <areadivider.h>
#include <scardetector.h>

ros::ServiceClient send_res;
int img_count = 0;

void sendClassifyRes(const std::string &res)
{
    srvs::SerPortSignal srv;
    srv.request.signal = res;

    send_res.call(srv);
}

void intToFixedString(std::string &str,int num)
{
    std::string str_ = std::to_string(num);
    if(str_.size() == 1)
        str = "00"+std::to_string(num);
    if(str_.size() == 2)
        str = "0"+std::to_string(num);
	if (str_.size() == 3)
		str = std::to_string(num);
}

bool areaImgRecvCB(srvs::Images::Request &imgs, srvs::Images::Response &)
{
    cv::Mat img1,img2;

    cv_bridge::CvImagePtr img_buffer_ptr1 = cv_bridge::toCvCopy(imgs.img1, sensor_msgs::image_encodings::MONO8);
    img_buffer_ptr1->image.copyTo(img1);
    cv_bridge::CvImagePtr img_buffer_ptr2 = cv_bridge::toCvCopy(imgs.img2, sensor_msgs::image_encodings::MONO8);
    img_buffer_ptr2->image.copyTo(img2);

    cv::imwrite("/home/iris1105/catkin_ws/src/image_proc/test_images/areaImage1.png",img1);
    cv::imwrite("/home/iris1105/catkin_ws/src/image_proc/test_images/areaImage2.png",img2);

    return true;
}

bool lineImgRecvCB(srvs::Images::Request &imgs, srvs::Images::Response &)
{
    cv::Mat img;

    cv_bridge::CvImagePtr img_buffer_ptr1 = cv_bridge::toCvCopy(imgs.img1, sensor_msgs::image_encodings::MONO8);
    img_buffer_ptr1->image.copyTo(img);

    std::string res;
    {
        std::vector<double> scales{ 0.0063,0.0777,0.9045,0.9171 };

        AreaDivider ad;
        ScarDetector sd;

        std::vector<AreaDivider::Line> lines;
        std::vector<std::vector<cv::Point>> scars;

        cv::Mat dividedImg;
        if (ad.divide(img, scales, lines))
        {
                std::cout<<"Divided successfully!"<<std::endl;
                sd.detect(img, lines, scars);
                ad.drawLines(img, lines, dividedImg);
                for (auto i = scars.cbegin(); i != scars.cend(); ++i)
                {
                        cv::Rect r = cv::boundingRect(*i);
                        cv::Rect r_(cv::Point(r.x - 5, r.y - 5), cv::Point(r.x + r.width + 5, r.y + r.height + 5));

                        cv::rectangle(dividedImg, r_, cv::Scalar(0, 0, 255));
                }
        }
        else
        {
            sendClassifyRes("000000000000");
            std::cout<<res<<std::endl;

            return true;
        }
        int nums[4]{0};
        for(auto i = scars.cbegin();i!=scars.cend();++i)
        {
            int min_x = img.cols, max_x = 0;
            int min_y = img.rows, max_y = 0;
            for (auto j = i->cbegin(); j != i->cend(); ++j)
            {
                    min_x = min_x <= j->x ? min_x : j->x;
                    max_x = max_x >= j->x ? max_x : j->x;
                    min_y = min_y <= j->y ? min_y : j->y;
                    max_y = max_y >= j->y ? max_y : j->y;
            }
            int center = (min_x + max_x) / 2;

            if(center>=lines[0].l&&center<lines[1].l)
                ++nums[0];
            if(center>=lines[2].l&&center<lines[3].l)
                ++nums[1];
            if(center>=lines[3].l&&center<lines[8].l)
                ++nums[2];
            if(center>=lines[8].l&&center<=lines[9].l)
                ++nums[3];

        }

        std::string str1,str2,str3,str4;
        intToFixedString(str1,nums[0]);
        intToFixedString(str2,nums[1]);
        intToFixedString(str3,nums[2]);
        intToFixedString(str4,nums[3]);

        res = str1+str2+str3+str4;

        std::string file = "/home/ubuntu/catkin_ws/src/image_proc/test_images/"+std::to_string(img_count)+".jpg";
        ++img_count;
        cv::imwrite(file,dividedImg);
    }

    sendClassifyRes(res);
    std::cout<<res<<std::endl;

    return true;
}

int main(int argc,char **argv)
{
    ros::init(argc,argv,"image_proc_node");

    ros::NodeHandle nh;
    ros::Publisher error;

    ros::ServiceServer recvArea;
    ros::ServiceServer recvLine;

    error = nh.advertise<std_msgs::String>("image_proc_error",3);
    send_res = nh.serviceClient<srvs::SerPortSignal>("classify_res");
    recvArea = nh.advertiseService("send_area",&areaImgRecvCB);
    recvLine = nh.advertiseService("send_line",&lineImgRecvCB);

    ros::spin();

    ros::waitForShutdown();

    return 0;
}

