#pragma once
#include <vector>
#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>


using namespace cv;

class ImagePreprocessing
{
private:

	Mat resize_image;
	Mat filter;
	Mat result;
	Mat gray;
	Mat opening;
	Mat edged;
	Mat procImg;
	Rect CropRect;

public:
	Mat getImage();
	void setResize(Mat &img);
	void procPadding();
	void procFitering();
	void procGrayscale();
	void procMorphologyEx();
	void procCanny();
	void procCropImage();
	void procResultImage(Mat& img);
	void showImage(Mat image);
};

