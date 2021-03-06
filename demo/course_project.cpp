/* Demo application for Computer Vision Library.
 * @file
 * @date 2018-12-04
 * @author Pol22
 */
#include <fstream>
#include <opencv2/opencv.hpp>

#include "cvlib.hpp"
#include "utils.hpp"


using namespace std;

class Object
{
public:
	int h;
	int w;
	cv::Point center;
	double speed_x;
	double speed_y;

	Object(const cv::Rect& rect)
	{
		h = rect.height;
		w = rect.width;
		center = cv::Point(rect.x + w / 2, rect.y + h / 2);
		speed_x = 0.0;
		speed_y = 0.0;
	}

	double distance_from_next(const cv::Rect& rect)
	{
		double x = center.x + speed_x - (rect.x + rect.width / 2);
		double y = center.y + speed_y - (rect.y + rect.height / 2);
		return sqrt(x * x + y * y);
	}

	void update(const cv::Rect& rect)
	{
		h = rect.height;
		w = rect.width;
		cv::Point new_center = cv::Point(rect.x + w / 2, rect.y + h / 2);
		speed_x = new_center.x - center.x;
		speed_y = new_center.y - center.y;
		center = new_center;
	}
};


class ObjectEscort
{
public:
	ObjectEscort() = default;
	bool insert(cv::Rect rect, const double mid_width)
	{
		auto nearest = objects.end();
		double dist = max_delta;
		for (auto obj = objects.begin(); obj != objects.end(); obj++)
		{
			double distance_to_rect = obj->distance_from_next(rect);
			if (distance_to_rect < dist)
			{
				nearest = obj;
				dist = distance_to_rect;
			}
		}

		if (nearest == objects.end())
		{
			objects.emplace_back(rect);
			return false;
		}

		cv::Point2f last_center = nearest->center;
		nearest->update(rect);
		if (nearest->center.x >= mid_width && last_center.x < mid_width)
			return true;
		else
			return false;
	}



private:
	list<Object> objects;
	const double max_delta = 20.0;
};


int course_project(int argc, char* argv[])
{
	/*
	const cv::String keys = // clang-format off
		"{help h usage ? |      | print this message   }"
		"{video          |      | video file           }";

	// clang-format on
	cv::CommandLineParser parser(argc, argv, keys);
	parser.about("Application name v1.0.0");
	if (parser.has("help"))
	{
		parser.printMessage();
		return 0;
	}

	auto video = parser.get<cv::String>("video");
	*/
	//cv::VideoCapture cap(video);
	cv::VideoCapture cap("TestVideo.mp4");
	if (!cap.isOpened())
		return -1;

	cv::Mat frame, blured, mask;
	vector<vector<cv::Point>> contours;
	vector<cv::Point> contour_poly;
	cv::Rect boundingBox;
	/*
	std::ofstream out(video + ".txt");
	if (!out.is_open())
		return -1;
	*/

	cv::Ptr<cv::BackgroundSubtractorMOG2> bg_substractor =
		cv::createBackgroundSubtractorMOG2(1500, 100.0, true);
	bg_substractor->setShadowThreshold(0.005);

	ObjectEscort escort;

	cv::Mat element1 = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5));
	cv::Mat element2 = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(25, 25));

	const auto origin_wnd = "origin";
	cv::namedWindow(origin_wnd, 1);
	
	utils::fps_counter fps;
	bool cross = false;
	double mid_w = double(frame.cols) / 2;
	while (true) // ESC
	{
		cap >> frame;
		if (frame.empty())
			break;

		blur(frame, blured, cv::Size(5, 5));

		bg_substractor->apply(blured, mask);
		mask = mask > 200;
		erode(mask, mask, element1);
		dilate(mask, mask, element2);

		findContours(mask, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
		for (int i = 0; i < contours.size(); i++)
		{
			cv::approxPolyDP(cv::Mat(contours[i]), contour_poly, 3, true);
			boundingBox = cv::boundingRect(cv::Mat(contour_poly));
			
			rectangle(frame, boundingBox, cv::Scalar(0, 255, 0), 2);
			cross = escort.insert(boundingBox, mid_w);
			if (cross)
				cout << "Cross!!!" << endl;
		}

		cv::line(frame, cv::Point(frame.cols / 2, 0), cv::Point(frame.cols / 2, frame.rows),
			cv::Scalar(0, 0, 255), 2, 8);

		utils::put_fps_text(frame, fps);
		cv::imshow(origin_wnd, frame);

		switch (cv::waitKey(1))
		{
			case 27:
				cv::destroyWindow(origin_wnd);
				return 0;
			case ' ':
				//out << cap.get(cv::CAP_PROP_POS_MSEC) << "\r\n";
				break;
			default:
				break;
		}
	}

	cv::destroyWindow(origin_wnd);

	return 0;
}
