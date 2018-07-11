#ifndef AREA_DIVIDER_H_
#define AREA_DIVIDER_H_


class AreaDivider
{
public:
	class Line
	{
	public:
		int l;      //Œª÷√  localtion
		double acc;
		bool aut;
	};

	bool divide(const cv::Mat &img, const std::vector<double> &scales, std::vector<Line> &lines)const;
	void drawLines(const cv::Mat &img, const std::vector<Line> &lines, cv::Mat &imgOut)const;

private:
	void detectLines(const cv::Mat &, std::vector<Line> &, double )const;
	bool filterLines(const std::vector<Line> &, std::vector<Line> &, const std::vector<double> &)const;
};

#endif
