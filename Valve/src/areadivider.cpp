#include "..\include\include.h"



//通过canny来求边缘之后分割图像
bool AreaDivider::divide(const cv::Mat &img, const std::vector<double> &scales, std::vector<Line> &lines)const
{
	std::vector<Line> lines_;
	int canny_param = 100;

	detectLines(img, lines_, canny_param);
	if (!filterLines(lines_, lines, scales))
	{
		detectLines(img, lines_, canny_param - 50);
		if (!filterLines(lines_, lines, scales))
			return false;
	}

	return true;
}


//分割线区域画成绿色的线，非分割区画成蓝色的线
void AreaDivider::drawLines(const cv::Mat &img, const std::vector<Line> &lines, cv::Mat &imgOut)const
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

//将图像进行一次缩放之后检测边缘 再进行线的查找
void AreaDivider::detectLines(const cv::Mat &img, std::vector<Line> &lines,double canny_param)const
{
	cv::Mat img_gray = img.clone();
	if (img.channels() == 3)
		cv::cvtColor(img, img_gray, CV_BGR2GRAY);

	cv::pyrDown(img_gray, img_gray);
	cv::pyrUp(img_gray, img_gray);

	cv::Canny(img_gray, img_gray, 50, canny_param, 3);
	
	//对于得到边缘的图像的每一列进行计数， 当一列的边缘点数大于300时便认为检测到直线
	for (auto i = 0; i < img_gray.cols; ++i)
	{
		int c = 0;
		for (auto j = 0; j < img_gray.rows; ++j)
			c = img_gray.at<unsigned char>(j, i) == 255 ? c + 1 : c;

		if (c > 300)
			lines.push_back({ i,c / double(img_gray.rows),true });
	}
}

//将获取到的线进行过滤，
bool AreaDivider::filterLines(const std::vector<Line> &lines_in, std::vector<Line> &lines_out, const std::vector<double> &scales)const
{
	//9根线对应平行面两根线，小端面一根线，锁夹槽6根线
	if (lines_in.size() < 9)
		return false;

	
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

		
	double width = (lines.cend() - 1)->l - lines.cbegin()->l;   //气门宽度
	auto it = lines.cbegin();

	if (lines.size() < 9)
		return false;

	//scales{ 0.0063,0.0777,0.9045,0.9171 };
	//对应平行面第二条线，蓝线，锁夹槽下方线，第一级锁夹槽
	
	lines_out.push_back(*it++);		//第一条线
	lines_out.push_back(*it++);		//第二条线
	lines_out.push_back({ lines[0].l + int(scales[1] * width),1,false });   //划分区域的蓝线  确定第三条线

	//如果在3号线附近10pix则确定   第四条线
	for (; it != lines.cend(); ++it)
		if (fabs(it->l - lines[0].l - scales[2] * width) < 10)
		{
			lines_out.push_back(*it);
			++it;
			break;
		}

	//如果已经是最后的线则报错， 根据上面推出的第四条线以及锁夹槽宽度 确定第五条线
	for (; it != lines.cend(); ++it)
	{
		if (it == lines.cend() - 1)
			return false;
		if (fabs(it->l - (lines_out.cend() - 1)->l - (scales[3] - scales[2]) * width) < 4)
		{
			lines_out.push_back(*it);
			++it;
			break;
		}
	}

	//如果已经到最后则报错， 确定第六条线
	if (it == lines.cend())
		return false;
	lines_out.push_back(*it);
	++it;

    //根据第六条线以及四五线的间距确定第七条线
	for (; it != lines.cend(); ++it)
	{
		if (it == lines.cend() - 1)
			return false;
		if (fabs(it->l - (lines_out.cend() - 1)->l - ((lines_out.cend() - 2)->l - (lines_out.cend() - 3)->l)) < 4)
		{
			lines_out.push_back(*it);
			++it;
			break;
		}
	}

    //确定第八条线
	if (it == lines.cend())
		return false;
	lines_out.push_back(*it);
	++it;

    //根据第八条线以及六七线的间距确定第九条线
	for (; it != lines.cend(); ++it)
	{
		if (it == lines.cend() - 1)
			return false;
		if (fabs(it->l - (lines_out.cend() - 1)->l - ((lines_out.cend() - 2)->l - (lines_out.cend() - 3)->l)) < 4)
		{
			lines_out.push_back(*it);
			++it;
			break;
		}
	}

    //确定最后一条线
	if (it == lines.cend() - 1 && lines_out.size() == 9)
		lines_out.push_back(*it);
	else
		return false;
    
    //循环判定是否有相等的线 否则报错
	for (auto i = lines_out.cbegin(); i != lines_out.cend(); ++i)
		for (auto j = i + 1; j != lines_out.cend(); ++j)
			if (i->l == j->l)
				return false;

	return true;
}