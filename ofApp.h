#pragma once

#include "ofMain.h"
#include "ofxLibsndFileRecorder.h"
#include "ofxGPIO.h"
#include "HttpFormManager.h"
#include "ofxQRcode.h"
#include "ofxOsc.h"
#include "ofxSoundFile.h"

//#include "bmFFT.h"
//#include "ofxFft.h"
#include "ofxBeat.h"

//#include "LibsndfileReader.h"
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


		void prepareRecord();

		float smooth_vol;
		float scaled_vol;
		
		
		bool recording; 

		void audioIn(float *input, int buffersize, int nChannels);
		void audioOut(float *output,int buffersize,int nChannels);
		void calcVolume(float *data,int buffersize,int nChannels);

		vector<float> record_float;
		int read_record,num_record;

		ofxLibsndFileRecorder audio_recorder;
		//ofSoundPlayer sound_player;
		ofxSoundFile *sound_file;

		int rec_start;
		string last_record;
		FrameTimer qrcode_timer,breath_timer;
	  	bool record_once;

		int cur_millis;
		

		//GPIO gpio16,gpio20,gpio21;
		//rpiPWM1 *pwm;
		ofSerial serial;
		int readSerial();
		void writeSerial(string val_);

		enum MODE{SLEEP,HINT,REC,BLINK,FINISH,QRCODE,STORED,PLAY,DINO};
		MODE mode;
		MODE next_mode;
		bool playing;
		void closeMode(MODE next_mode_);
		void startMode(MODE mode_);
		
		void drawMode(MODE mode_,float t_,bool fade_out);


		//LibsndfileReader wav_reader;
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
		ofImage front_image[11];
		ofImage count_image[10];

	//	FrameTimer front_timer;
		FrameTimer hint_timer;
		FrameTimer go_timer;
		FrameTimer count_timer;
		FrameTimer blink_timer;
		FrameTimer glow_timer;

		FrameTimer sleep_timer;

		void onHintTimerEnd(int &data);
		void onGoTimerEnd(int &data);

		void onCountTimerEnd(int &data);
		void onBlinkTimerEnd(int &data);


		ofxSoundFile sound_fx[1];
		

		ImageSeq dino_seq;
		FrameTimer jump_timer;
		bool dino_start;
		bool dino_dead;
		ofImage dead_image;
		ofImage hi_image;
		ofImage score_image;
		int dino_score;

		FrameTimer cat_timer[2];
		ofImage cat_image[3];
		ofImage over_image;
		int cat_id[2];
		bool cat_scored[2];
		float cat_val;
	
		void resetDinoGame();
		void resetCatTimer(int i);
		void drawDinoScore(int score);

};
