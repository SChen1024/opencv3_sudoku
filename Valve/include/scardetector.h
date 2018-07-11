#ifndef SCAR_DETECTOR_H_
#define SCAR_DETECTOR_H_


class ScarDetector
{
public:
	void detect(const cv::Mat &img, const std::vector<AreaDivider::Line> &lines, std::vector<std::vector<cv::Point>> &scars)const;
	void showScars(const cv::Mat &img, const std::vector<std::vector<cv::Point>> &scars)const;

private:
	void detectScars(const cv::Mat &, std::vector<std::vector<cv::Point>> &, const std::vector<AreaDivider::Line> &)const;
	void filterScars(const cv::Mat &, const std::vector<std::vector<cv::Point>> &, std::vector<std::vector<cv::Point>> &, const std::vector<AreaDivider::Line> &)const;

	bool contourIsEdge(const cv::Mat &, const std::vector<cv::Point> &, const cv::Point &)const;
};
#endif
