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
#include "ImageSeq.h"


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
		
		
		bool recording; 

		void audioIn(float *input, int buffersize, int nChannels);
		void audioOut(float *output,int buffersize,int nChannels);
		void calcVolume(float *data,int buffersize,int nChannels);

		vector<float> record_float;
		int read_record;

		ofxLibsndFileRecorder audio_recorder;
		int rec_start;
		string last_record;
		FrameTimer rec_timer,qrcode_timer,finish_timer,breath_timer;
	  	
		int cur_millis;


		//GPIO gpio16,gpio20,gpio21;
		//rpiPWM1 *pwm;
		ofSerial serial;
		int readSerial();
		void writeSerial(string val_);

		enum MODE{SLEEP,REC,FINISH,QRCODE,STORED,PLAY};
		MODE mode;
		MODE next_mode;
		bool playing;
		void closeMode(MODE next_mode_);
		void startMode(MODE mode_);
		
		void drawMode(MODE mode_,float t_);


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

		//video
		FrameTimer transition_timer;
		void onTransitionEnd(int &data);
		ofImage qrcode_back;
		
		ImageSeq back_seq;
		ImageSeq logo_seq;
		vector<ofImage> front_image;
		
		FrameTimer front_timer;
		FrameTimer count_timer;

};
