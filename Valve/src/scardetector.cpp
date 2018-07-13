#include "..\include\include.h"


//将图像转成彩色 在每一个伤痕的外围标记为红色
void ScarDetector::showScars(const cv::Mat &img, const std::vector<std::vector<cv::Point>> &scars)const
{
	cv::Mat rgb_img = img.clone();
	if (img.channels() == 1)
		cv::cvtColor(img, rgb_img, CV_GRAY2BGR);

	for (auto i = scars.cbegin(); i != scars.cend(); ++i)
		for (auto j = i->cbegin(); j != i->cend(); ++j)
			rgb_img.at<cv::Vec3b>(j->y, j->x) = { 0,0,255 };

	//cv::imwrite("scar.png", rgb_img);

	resize(rgb_img, rgb_img, cv::Size(rgb_img.cols / 2, rgb_img.rows / 2));
	//cv::imshow("Scar", rgb_img);
}


//将伤痕都检测出来 大于30的都算伤痕
void ScarDetector::detectScars(const cv::Mat &img, std::vector<std::vector<cv::Point>> &scars, const std::vector<AreaDivider::Line> &lines)const
{
	cv::Mat img_canny = img.clone();
	if (img.channels() == 3)
		cv::cvtColor(img, img_canny, CV_BGR2GRAY);

	cv::pyrDown(img_canny, img_canny);
	cv::pyrUp(img_canny, img_canny);
	cv::GaussianBlur(img_canny, img_canny, cv::Size(5, 5), 0);

	int width = (lines.cend() - 1)->l - lines.cbegin()->l;
	int l_1 = lines[2].l;
	int l_2 = lines.cbegin()->l + 0.3428*width;

	cv::Mat img_ = cv::Mat(img_canny, cv::Rect(cv::Point(l_1 - 20, 0), cv::Point(l_2 + 50, img_canny.rows)));
	cv::blur(img_, img_, cv::Size(5, 5));

	cv::Canny(img_canny, img_canny, 30, 60, 3);

	cv::dilate(img_canny, img_canny, cv::Mat());
	cv::erode(img_canny, img_canny, cv::Mat());

	std::vector<std::vector<cv::Point> > contours;
	cv::findContours(img_canny, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);

	for (auto i = contours.cbegin(); i != contours.cend(); ++i)
	{
		double contour_area = cv::contourArea(*i);
		double contour_length = cv::arcLength(*i, false);

        //伤痕面积或者长度大于30 定为伤痕
        if (contour_area > 30 || contour_length >30)
			scars.push_back(*i);
	}
}

void ScarDetector::filterScars(const cv::Mat &img, const std::vector<std::vector<cv::Point>> &scars_in, std::vector<std::vector<cv::Point>> &scars_out, const std::vector<AreaDivider::Line> &lines)const
{
	std::vector<std::vector<cv::Point>> back_up, contours_filtered, contours_filtered_;
	for (auto i = scars_in.cbegin(); i != scars_in.cend(); ++i)
	{
		std::vector<cv::Point> contour;
		for (auto j = i->cbegin(); j != i->cend(); ++j)
		{
            //在每个线附近三个像素以及三个槽内还有 2-3线之间的区域不检索
			bool scar_pt = true;
			for (auto k = lines.cbegin(); k != lines.cend(); ++k)
			{
				if (k == lines.cbegin() + 2)
					continue;
				if (fabs(k->l - j->x) < 3
					|| (j->x >= lines[3].l&&j->x <= lines[4].l)
					|| (j->x >= lines[5].l&&j->x <= lines[6].l)
					|| (j->x >= lines[7].l&&j->x <= lines[8].l)
					)
					scar_pt = false;
			}
			if (scar_pt)
				contour.push_back(*j);
		}
		if (!contour.empty())
			back_up.push_back(contour);
	}

    //设置掩码图  绘出边缘
	cv::Mat mask_img(cv::Size(img.cols, img.rows), CV_8UC1);
	for (int i = 0; i < mask_img.rows; ++i)
	{
		unsigned char *data = mask_img.ptr<unsigned char>(i);
		for (int j = 0; j < mask_img.cols; ++j)
			data[j] = 0;
	}

	for (auto i = back_up.cbegin(); i != back_up.cend(); ++i)
		for (auto j = i->cbegin(); j != i->cend(); ++j)
			mask_img.at<unsigned char>(j->y, j->x) = 255;

	cv::findContours(mask_img, contours_filtered, CV_RETR_LIST, CV_CHAIN_APPROX_NONE);

	int width = (lines.cend() - 1)->l - lines.cbegin()->l;
	int l_1 = lines.cbegin()->l + 0.3428*width;
	int l_2 = lines[3].l - 30;

    
	for (auto i = contours_filtered.cbegin(); i != contours_filtered.cend(); ++i)
	{
        //将所有的边缘的准确边缘
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

        //
		if ((center<lines[2].l + 5 && center>lines[1].l) || center > (lines.end() - 1)->l)
			continue;
		if ((center > l_1 - 60 && center<l_1 + 60) || (center>l_2 - 25 && center < l_2 + 25))
		{
			cv::Mat block(img, cv::Rect(cv::Point(min_x, min_y), cv::Point(max_x + 1, max_y + 1)));
			if (!(contourIsEdge(block, *i, cv::Point(min_x, min_y)) && (max_x - min_x) / double(max_y - min_y+0.000000000001) < 0.5))
				contours_filtered_.push_back(*i);
		}
		
			contours_filtered_.push_back(*i);
	}

	cv::Mat final_proc_img(cv::Size(img.cols, img.rows), CV_8UC1);
	for (int i = 0; i < final_proc_img.rows; ++i)
	{
		unsigned char *data = final_proc_img.ptr<unsigned char>(i);
		for (int j = 0; j < final_proc_img.cols; ++j)
			data[j] = 0;
	}

	for (auto i = contours_filtered_.cbegin(); i != contours_filtered_.cend(); ++i)
		for (auto j = i->cbegin(); j != i->cend(); ++j)
			final_proc_img.at<unsigned char>(j->y, j->x) = 255;

	cv::Mat element = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(10, 10));
	cv::morphologyEx(final_proc_img, final_proc_img, cv::MORPH_CLOSE, element);

	cv::findContours(final_proc_img, scars_out, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
}

void ScarDetector::detect(const cv::Mat &img, const std::vector<AreaDivider::Line> &lines, std::vector<std::vector<cv::Point>> &scars)const
{
	std::vector<std::vector<cv::Point>> scars_pre;
	detectScars(img, scars_pre, lines);
	filterScars(img, scars_pre, scars, lines);
}

bool ScarDetector::contourIsEdge(const cv::Mat &img, const std::vector<cv::Point> &contour, const cv::Point &pt)const
{
	cv::Mat img_ = img.clone();
	cv::medianBlur(img_, img_, 3);

    //将所有轮廓线标记为黑色
	for (auto i = contour.cbegin(); i != contour.cend(); ++i)
		img_.at<unsigned char>(i->y - pt.y, i->x - pt.x) = 0;

	int total_l = 0, total_r = 0;
	int total_count_l = 0, total_count_r = 0;
	for (int i = 0; i < img_.rows; ++i)
	{
		const unsigned char *data = img_.ptr<unsigned char>(i);
		bool left = true;
		for (int j = 0; j < img_.cols; ++j)
		{
            //从做往右依次查找到不是黑色的点
			if (data[j] == 0)
			{

				left = false;
				continue;
			}
			if (left)
			{
				total_l += data[j];
				++total_count_l;
			}
			else
			{
				total_r += data[j];
				++total_count_r;
			}
		}
	}

	double avg_l = total_l / (total_count_l + 0.00000000001);
	double avg_r = total_r / (total_count_r + 0.00000000001);
	double avg = (avg_l + avg_r) / 2.0;

	int left_sma_num = 0, left_big_num = 0;
	for (int i = 0; i < img_.rows; ++i)
	{
		const unsigned char *data = img_.ptr<unsigned char>(i);
		for (int j = 0; j < img_.cols; ++j)
		{
			if (data[j] == 0)
				break;

			if (data[j] - avg > 0)
				++left_big_num;
			else
				++left_sma_num;
		}
	}

	double length = cv::arcLength(contour,false);
	if ((left_big_num / double(total_count_l+0.0000000000001) > 0.8
		|| left_sma_num / double(total_count_l+0.0000000000001) > 0.8)||length < 20)
		return true;
	else
		return false;
}
