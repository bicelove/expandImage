#include <stdio.h>  // for snprintf
#include <string>
#include <vector>
#include <iostream>
#include <sstream> // stringstream
#include <fstream> // NOLINT (readability /streams)
#include <utility> // Create key-value pair (there could be not used)
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h> 
#include <unistd.h>
#include <time.h>
#include <opencv/cv.h>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

using namespace std;
using namespace cv;

//dirs to save predict patch logo results
int mkDir(
	const string									keyfile,			//[In]: Input dictionary path
	const string									resultPath)			//[In]: Input save results path
{
	string line;
	string IDdict = keyfile + "dirList.txt";

	/**************************************/	
	std::ifstream IDifs(IDdict);
	if(!IDifs){
		cout<<"[Open dictionary failed:]"<<IDdict;
		return -1;
	}
	
	while(getline(IDifs, line)){
		if(line.length() != 0){
			std::string foldname = resultPath + line;
			//cout<<foldname<<endl;
			if(access(foldname.c_str(), 0) == -1 && mkdir(foldname.c_str(),0777))
				std::cout<<"create FIRST file bag failed!"<<endl;
		}
	}	
	
	IDifs.close();
	
	return 0;
}

//
int getStringIDFromFilePath(
	const string							imageString,
	string									&imageID,
	string									&rectLabel)
{
	string imageIDTemp;
	int sour_pos, postfix_pos;
	string sour_name;
	int rectLabelPos;
	string rectLabelString, rectLabelNew;

	//
	sour_pos 	= imageString.find_last_of('/');
	//cout<<"sour_pos = "<<sour_pos<<endl;	
	sour_name	= imageString.substr(sour_pos + 1);
	postfix_pos = sour_name.find_last_of('.');
	imageIDTemp = sour_name.erase(postfix_pos, 4);  		

	//labelName
	rectLabelString = imageString.substr(0, sour_pos);	
	rectLabelPos = rectLabelString.find_last_of('/');
	rectLabel = rectLabelString.substr(rectLabelPos + 1);		
	
	//combine the same labels (eg. Armani_1 & Armani_2)
	sour_pos = rectLabel.find_last_of('_');
	rectLabelString = rectLabel.substr(sour_pos + 1);
	if(rectLabelString == "1" || rectLabelString == "2" ){
		rectLabelNew = rectLabel.substr(0, sour_pos);
		rectLabel = rectLabelNew;		
	}

	imageID  = imageIDTemp;

	return 0;
}

//
int ReadImageToResize(
	const Mat  									cv_img_origin,
   	const int 									l_side, 
   	Mat  										&cv_img)
{		
	int height = cv_img_origin.rows;
	int width  = cv_img_origin.cols;

	int long_side, short_side;
	if(height > width){
		long_side = height;
		short_side = width;
	}else{
		long_side = width;
		short_side = height;
	}
	/*
	if(long_side/short_side >= 10)  
		return 0;
	*/
	
	if ( long_side <= l_side ){
		cv_img = cv_img_origin;
	}else if (long_side == height ) {
		cv::resize(cv_img_origin, cv_img, cv::Size((int)((l_side*width)/height), l_side));
	}else if(long_side == width){
		cv::resize(cv_img_origin, cv_img, cv::Size(l_side, (int)((l_side*height)/width)));
	}

	return 0;
}

//
int expandImg(
	const Mat  										resizeImg,
	Mat											&dstImg)
{	
	if(!resizeImg.data || resizeImg.channels()!=3){	
		cout<<"Can't open resizeImg!"<< endl;
		return -1;
	}
	
	Mat imgClone = resizeImg.clone();
	Mat blurImg;
	Scalar value;
	int exSide;
	int top, bottom, left, right;

	//
	medianBlur(imgClone, blurImg, 3);

	//
	IplImage imgTmp = blurImg;
	IplImage *blurImgTemp = cvCloneImage(&imgTmp);
	CvScalar avgChannels = cvAvg(blurImgTemp);    
	double avgB = avgChannels.val[0];  
	double avgG = avgChannels.val[1];  
	double avgR = avgChannels.val[2];  

	//	
	if(resizeImg.cols <= resizeImg.rows){
		top = 0; 
		bottom = 0;
		left = abs(resizeImg.rows - resizeImg.cols) / 2;
		right = abs(resizeImg.rows - resizeImg.cols) / 2;
	}else{
		top = abs(resizeImg.rows - resizeImg.cols) / 2;
		bottom = abs(resizeImg.rows - resizeImg.cols) / 2;
		left = 0;
		right = 0;
	}
	value = Scalar(avgB, avgG, avgR);
	copyMakeBorder( resizeImg, dstImg, top, bottom, left, right, BORDER_CONSTANT, value );	
	
	imgClone.release();
	
	return 0;
}

//
int imgExpand(
	const string 									imgID,
	const string 									svPath)
{
	FILE* fpQueryList = fopen(imgID.c_str(), "r");
	if(NULL == fpQueryList)
	{
		cout<<"cannot open "<<imgID<<endl;
		return -1;
	}

	int nRet = 0;
	char buff[1000];
	string srcImgID, rectLabel;
	char savePath[256];
	int l_side = 224; 
	int numImg = 0;
	long long imgName = 201705120000000;
	
	while(fgets(buff, 1000 ,fpQueryList) != NULL)
	{	
		char *pstr = strtok(buff, "\n");
		//cout<<"pstr:"<<pstr<<endl;
	
		cv::Mat cv_img_origin = cv::imread(pstr, 1);
		if(!cv_img_origin.data) {
			std::cout << "Could not open or find the file :" << pstr <<endl;
			continue;
		}

		//		
		nRet = getStringIDFromFilePath(pstr, srcImgID, rectLabel);
		if(nRet != 0){
			cv_img_origin.release();
			continue;
		}		
		imgName++;
		sprintf(savePath, "%s/%s/%lld.jpg", svPath.c_str(), rectLabel.c_str(), imgName);
		//cout<<"savePath: "<<savePath<<endl;
		
		/*
		if(cv_img_origin.cols == cv_img_origin.rows){
			imwrite(savePath, cv_img_origin);
			continue;
		}
		//cout<<"cv_img_origin.cols = "<<cv_img_origin.cols<<endl;
		//cout<<"cv_img_origin.rows = "<<cv_img_origin.rows<<endl;
		*/

		//
		Mat cv_img, dstImg;  
		nRet = ReadImageToResize(cv_img_origin, l_side, cv_img);
		cv_img_origin.release();
		if(nRet != 0){
			cv_img.release();
			continue;
		}
		
		nRet = expandImg(cv_img, dstImg);
		cv_img.release();
		if(nRet != 0){			
			dstImg.release();
			continue;
		}

		//
		imwrite(savePath, dstImg);
		//
		dstImg.release();

		numImg++;
		if(numImg % 100 == 0){
			cout<<"the "<<numImg<<"th image end!"<<endl;
		}
		
	}
	
	fclose(fpQueryList);
	
	return 0;
}

//
int main(int argc, char** argv) {

	const int num_required_args = 3;
	if( argc < num_required_args ){
	    cout<<
	    "This program extracts features of an image and predict its label and score.\n"
	    "Usage: Demo_mainboby szQueryList outputList\n";
	    return 1;
  }	
	
	/***********************************Init**********************************/
	string imgList = argv[1];		
	string svPath = argv[2];	
	string keyfilePath = argv[3];
	
	int nRet = 0;		
	/**************************** getAllDataQueryList *************************/
	nRet = mkDir(keyfilePath, svPath);
	if(nRet != 0){
		cout<<"fail to getQueryList!"<<endl;
		return -1;
	}

	nRet = imgExpand(imgList, svPath);
	if(nRet != 0){
		cout<<"fail to getQueryList!"<<endl;
		return -1;
	}
	
	cout<<"deal end!"<<endl;
	
	return 0;
}

