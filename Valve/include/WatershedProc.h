#pragma once


#ifndef __WATERSHEDPROC_H__
#define __WATERSHEDPROC_H__


class WaterShedProc
{
    private:
        cv::Mat markers;

    public:
        WaterShedProc();
        ~WaterShedProc();
        
        void setMarkers( const cv::Mat& markerImage );
        
        void preProc(const cv::Mat &, cv::Mat &);
        void process(const cv::Mat &, cv::Mat & );

        void drawMarkersLine( cv::Mat &dstImg, const cv::Mat &markers );
        void drawMarkersCon( cv::Mat &dstImg, const cv::Mat &markers );


};




#endif // !__WATERSHEDPROC_H__
