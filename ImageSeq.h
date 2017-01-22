#ifndef IMAGE_SEQ_H
#define IMAGE_SEQ_H

#include "ofMain.h"


class ImageSeq{
private:
	vector<ofImage> img;
	int num;
	float index;
	float fps;
	int start_count;

	bool pause;

	string nf(string format,int number){
		char buf[10];
		sprintf(buf,format.c_str(),number);
		return (string)buf;
	}
public:
	ImageSeq(){}
	ImageSeq(string filename_,int count_,int fps_,int start_){
		
		start_count=start_;
		num=count_;	
		
		for(int i=start_count;i<=num;++i){
			string name_=filename_+nf("%05d",i)+".jpg";
			img.push_back(ofImage(name_));
			if(i%100==0 || i==num-1) ofLog()<<"Load File: "<<name_;
		}
		index=start_count;
		fps=(float)fps_/1000.0f;
		pause=true;
	}
	void reset(){
		index=start_count;
		pause=true;
	}
	void start(){
		pause=false;
	}
	void setIndex(int i){
		index=i;
	}
	void update(float dm){
		if(pause) return;
		if(index>=num) index=index-num+start_count;

		if(index<num) index+=dm*fps;
	}
	void setPause(bool p){
		pause=p;
	}
	ofImage getCurFrame(){
		//return img[0];
		return img[ofClamp(floor(index-start_count),0,num-start_count-1)];
	}
	int getIndex(){
		return ofClamp(floor(index),start_count,num-1);
	}
};



#endif