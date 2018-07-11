#include <ros/ros.h>
#include <srvs/SerPortSignal.h>

#include <iostream>
#include <string>

bool signalRecvCB(srvs::SerPortSignal::Request &srv, srvs::SerPortSignal::Response &)
{
    std::cout<<srv.signal<<std::endl;

    return true;
}

int main(int argc,char **argv)
{
    ros::init(argc,argv,"ser_node");

    ros::NodeHandle nh;
    ros::ServiceServer send_res;

    send_res = nh.advertiseService("classify_res",&signalRecvCB);

    ros::spin();
    ros::waitForShutdown();

    return 0;
}
