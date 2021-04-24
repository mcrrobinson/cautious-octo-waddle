#include "vision.hpp"
#include <opencv2/highgui.hpp>

void Vision::PlayVision(cv::Mat img) {
	while (true) {
		cv::imshow("Image", img);
		cv::waitKey(1);
	}
}