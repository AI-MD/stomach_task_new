#include "ImagePreprocessing.h"

Mat ImagePreprocessing::getImage()
{
	return procImg;
}
void ImagePreprocessing::setResize(Mat& img)
{
	resize(img, resize_image, Size(640, 480), 0, 0, cv::INTER_AREA);
	resize_image = resize_image.clone();
}

void ImagePreprocessing::procPadding()
{
	copyMakeBorder(resize_image, result, 30, 30, 30, 30, BORDER_CONSTANT, Scalar(0, 0, 0));
}

void ImagePreprocessing::procFitering()
{
	bilateralFilter(result, filter, -1, 15, 15);
	GaussianBlur(result, result, Size(5, 5), 0.75, 0.75);
}
void ImagePreprocessing::procGrayscale()
{
	cvtColor(result, gray, COLOR_BGR2GRAY);
}
void ImagePreprocessing::procMorphologyEx()
{
	Mat mask = getStructuringElement(cv::MORPH_RECT, Size(11, 11), cv::Point(1, 1));
	morphologyEx(gray, opening, cv::MorphTypes::MORPH_OPEN, mask, cv::Point(-1, -1), 4);
}
void ImagePreprocessing::procCanny()
{
	Canny(opening, edged, 10, 50);
}
void ImagePreprocessing::procCropImage()
{
	std::vector<std::vector<Point> > contours;

	findContours(edged, contours, RETR_EXTERNAL, CHAIN_APPROX_NONE);
	
	std::vector<Rect> boundRect(contours.size());
	
	int largest_contour_index = -1;
	int largest_area = 0;
	Rect bounding_rect;
		
	if (contours.size() > 0)
	{
		
		for (size_t i = 0; i < contours.size(); i++)
		{
			double a = contourArea(contours[i]);  
			if (a > largest_area) {
				largest_area = a;
				largest_contour_index = i;                
				bounding_rect = boundingRect(contours[i]); 
			}
		}
	}
	else
	{
		std::cout << "no countours" << std::endl;
		procImg = resize_image.clone();
	}
		
	if (largest_contour_index >= 0)
	{
		if (bounding_rect.width > 425 && bounding_rect.height > 400)
		{
			CropRect = bounding_rect;
			procImg = result(CropRect);
		}	
		else if(CropRect.width > 0 && CropRect.height > 0)
		{
			procImg = result(CropRect);
		}
		else
		{
			procImg = resize_image.clone();
		}
	}
	else if (CropRect.width > 0 && CropRect.height > 0)
	{
		procImg = result(CropRect);
	}
	else
	{
		procImg = resize_image.clone();
	}

	cvtColor(procImg, procImg, COLOR_BGR2RGB);
		
}
void ImagePreprocessing::procResultImage(Mat& img)
{
	setResize(img);
	procPadding();
	procFitering();
	procGrayscale();
	procMorphologyEx();
	procCanny();
	procCropImage();
}

void ImagePreprocessing::showImage(Mat image)
{
	if (image.empty())
	{
		std::cout << "Could not read the image: " << std::endl;
	}
	imshow("Display window", image);

	int k = waitKey(0); // Wait for a keystroke in the window

	if (k == 's')
	{
		imwrite("starry_night.png", image);
	}
}