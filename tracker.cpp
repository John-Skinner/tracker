#include <opencv2/opencv.hpp>
#include <iostream>
#include <string>
#include <chrono>

static bool ShowCamera = false;


using namespace cv;

void CenterOfMassRed(const Mat& img, float& x, float& y)
{
	bool canProcess = (img.isContinuous() && img.depth() == CV_8U && img.channels() == 3);
	if (!canProcess)
	{
		std::cout << " format of image not supported" << std::endl;
		return;
	}


	unsigned char *pix = img.data;
	float xWeighted = 0.0f;
	float yWeighted = 0.0f;
	float totalIntensity = 0.0f;
	for (int iy = 0;iy < img.rows;iy++)
	{
		for (int ix = 0; ix < img.cols;ix++)
		{
			unsigned char cRed = *(pix+2);
			unsigned char cGreen = *(pix+1);
			unsigned char cBlue = *pix;
			float redRatio = 0.0;
			int sum=cRed+cGreen+cBlue;
			if (sum > 0)
			{
				redRatio=float(cRed)/float(sum);
			}
			if (redRatio < 0.7)
			{
				redRatio = 0.0;
			}
			xWeighted += redRatio*ix;
			yWeighted += redRatio*iy;
			totalIntensity += redRatio;
			pix = pix+3;
		}
	}
	x = xWeighted/totalIntensity;
	y = yWeighted/totalIntensity;
	return;
};

int main(int argc, char* argv[])
{
	if (argc > 1)
	{
		ShowCamera=true;
	}
	else
	{
		ShowCamera=false;
	}
    std::string pipeline = "nvarguscamerasrc tnr-mode=2 tnr-strength=1 saturation=2 ! video/x-raw(memory:NVMM), width=640, height=480, format=NV12,framerate=60/1 ! nvvidconv ! video/x-raw, format=(string)BGRx ! videoconvert ! video/x-raw, width=640, height=480, format=(string)BGR ! appsink";

    std::cout << "Using pipeline: \n\t" << pipeline << "\n";
 
    VideoCapture cap(pipeline, CAP_GSTREAMER);
    if(!cap.isOpened()) {
	std::cout<<"Failed to open camera."<<std::endl;
	return (-1);
    }

    namedWindow("CSI Camera", WINDOW_AUTOSIZE);
    Mat img;
    Mat coordImg;

    std::cout << "Hit ESC to exit" << "\n" ;
    int nbFrames = 0;
    std::chrono::time_point<std::chrono::system_clock> t1;
    std::chrono::time_point<std::chrono::system_clock> t2;
    std::chrono::time_point<std::chrono::system_clock> ttmp;
    t1 = std::chrono::system_clock::now();
    	if (!cap.read(img)) {
		std::cout<<"Capture read error"<<std::endl;
	}


    while(true)
    {
	
	nbFrames++;
	if ((nbFrames % 1000) == 0)
	{
		ttmp = t1;
		t2 = std::chrono::system_clock::now();
		auto diff = std::chrono::duration_cast<std::chrono::seconds>(t2 - t1);
		t1 = t2;
		int secs = diff.count();
		std::cout << " fps:" << std::to_string(1000/secs) << std::endl;

	}
	float imagex,imagey;
	CenterOfMassRed(img,imagex,imagey);
	float normX = imagex/img.cols;
	float normY = imagey/img.rows;
	std::cout << "(" << normX << "," << normY << ")" << std::endl;
	if (ShowCamera) {
	imshow("Camera",img);
	}
	cap.read(img);
	if (ShowCamera) {
        int keycode = waitKey(30) & 0xff ; 
        if (keycode == 27) break ;
	}
    }
    std::cout << " matrix type:" << std::to_string(img.type()) << std::endl;
    std::cout << " matrix depth:" << std::to_string(img.depth()) << std::endl;
    std::cout << " CV_8U:" << std::to_string(CV_8S) << std::endl;
    std::cout << " dims:" << img.cols << " x " << img.rows << std::endl;

    std::cout << " frames:" << std::to_string(nbFrames) << std::endl;
    cap.release();
    destroyAllWindows() ;
    return 0;
}


