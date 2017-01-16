#pragma once

#include "ofMain.h"
#include "ofxLibsndFileRecorder.h"
#include "ofxGPIO.h"
#include "HttpFormManager.h"
#include "ofxQRcode.h"
#include "ofxOsc.h"
//#include "bmFFT.h"
//#include "ofxFft.h"
#include "ofxBeat.h"

#include "LibsndfileReader.h"
#include "Params.h"
#include "FrameTimer.h"
#include "StringUtil.h"


#define NUM_CHANNELS 1
#define SAMPLE_RATE 44100
#define BUFFER_SIZE 512
#define N_BAND 9

class ofApp : public ofBaseApp{
	public:
		void setup();
		void update();
		void draw();
		
		
		void exit();


		Param* param;


		bool isRecording;
		void startRecord();
		void stopRecord();
		void playRecord();
		void stopPlayRecord();
		void resetSleep();

		float smooth_vol;
		float scaled_vol;
		
		
		//vector<float> record;
		
		void audioIn(float *input, int buffersize, int nChannels);
		void audioOut(float *output,int buffersize,int nChannels);
		void calcVolume(float *data,int buffersize,int nChannels);

		ofxLibsndFileRecorder audio_recorder;
		int rec_start;
		string last_record;
		FrameTimer rec_timer,qrcode_timer,finish_timer;
	  	
		int cur_millis;


		//GPIO gpio16,gpio20,gpio21;
		//rpiPWM1 *pwm;
		ofSerial serial;
		int readSerial();
		void writeSerial(int val_);

		enum MODE{SLEEP,REC,FINISH,QRCODE,STORED,PLAY};
		MODE mode;
		bool playing;
		void setMode(MODE mode_);

		LibsndfileReader wav_reader;
		ofSoundStream out_sound_stream;		
		int play_start;


		//upload
		HttpFormManager upload_manager;
		void newResponse(HttpFormResponse &response);
		void uploadFile(string id_,string file_);
		ofxQRcode qrcode;

		string cur_id;
		string createId();
		
		//osc
		ofxOscReceiver osc_receive;
		void handleOsc();

		//fft
		ofxBeat beat;
		float *sample;
		float *band_volume;
		//int buffer_countero;

		//video
		ofxVideoPlayer back_video;

};
