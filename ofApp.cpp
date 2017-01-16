#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
	
	ofHideCursor();
	
	rec_timer=FrameTimer(Param::val()->rec_time*1000,Param::val()->count_time*1000);
	qrcode_timer=FrameTimer(Param::val()->qrcode_time*1000,0);
	finish_timer=FrameTimer(Param::val()->finish_time*1000,0);
	//cout<<Param::val()->finish_time<<endl;


	cur_millis=ofGetElapsedTimeMillis();

	scaled_vol=0;
	smooth_vol=0;
	
	mode=MODE::SLEEP;

	last_record=Param::val()->last_record;
	
	out_sound_stream.printDeviceList();
	out_sound_stream.setup(this,2,1,SAMPLE_RATE,BUFFER_SIZE,4);
	//out_sound_stream.stop();


	serial.setup("/dev/ttyACM0",9600);
	if(serial.isInitialized()) ofLog()<<"Serial Initialized!!";
	else ofLog()<<"Serial Fail!";
	

	osc_receive.setup(Param::val()->osc_port);

	ofAddListener(upload_manager.formResponseEvent,this,&ofApp::newResponse);


	back_video.load("movies/fingers.mov");
	back_video.setLoopState(OF_LOOP_NORMAL);
	back_video.play();

}

//--------------------------------------------------------------
void ofApp::update(){
	
	float dm=ofGetElapsedTimeMillis()-cur_millis;
	cur_millis+=dm;
	beat.update(cur_millis);
		
	scaled_vol=ofMap(smooth_vol,0,0.17,0,1.0,true);
	writeSerial(int(ofMap(ofClamp(scaled_vol,0.5,1),0.5,1,10.0,255.0)));

	int key_=readSerial();
	if(key_==3) resetSleep();
	handleOsc();

	back_video.update();


	switch(mode){
		case SLEEP:
			//key_=readSerial();
			if(key_==1) startRecord();
			break;
		case REC:
			rec_timer.update(dm);
			if(rec_timer.val()==1){
				stopRecord();
			}
			break;
		case FINISH:
			//cout<<"at finish: "<<finish_timer.val()<<endl;
			finish_timer.update(dm);
			//key_=readSerial();
			if(key_==1){
				startRecord();
			}else if(key_==2){
				playRecord();
				finish_timer.reset();
			}
			if(finish_timer.val()==1){
				mode=MODE::QRCODE;
				qrcode_timer.restart();
				uploadFile(cur_id,ofToDataPath("audio_"+cur_id+".wav",true));	
			}
			break;
		case QRCODE:
			qrcode_timer.update(dm);
			if(qrcode_timer.val()==1){
				mode=MODE::STORED;
			}
			if(key_==2) playRecord();
			break;
		case PLAY:

			break;
		case STORED:
			//key_=readSerial();
			if(key_==2) playRecord();
			break;
	}

}
//--------------------------------------------------------------
// serial
//--------------------------------------------------------------
int ofApp::readSerial(){
	if(serial.isInitialized() && serial.available()){
		vector<string> val=readSerialString(serial,'|');
		if(val.size()<1) return -1;
		ofLog()<<"serial read: "<<ofToString(val)<<endl;
		return ofToInt(val[0]);

	}
	return -1;
}
void ofApp::writeSerial(int val_){
	if(serial.isInitialized()){
		//ofLog()<<"serial write: "<<val_;

		string str_=ofToString(val_)+'|';
		int len_=str_.size();
		unsigned char* c_=new unsigned char[len_];
		memcpy(c_,str_.c_str(),len_);

		serial.writeBytes(c_,len_);
		delete [] c_;

	}
}
//--------------------------------------------------------------
// osc
//--------------------------------------------------------------
void ofApp::handleOsc(){
	if(!osc_receive.hasWaitingMessages()) return;
	ofxOscMessage message_;
	osc_receive.getNextMessage(message_);
	if(message_.getAddress()=="/reset"){
		ofLog()<<"Get RESET!";
		resetSleep();
	}
	
}


//--------------------------------------------------------------
void ofApp::draw(){
	ofSetColor(255);
	
	back_video.draw(0,0);

	ofSetColor(0);
	ofDrawBitmapString("vol= "+ofToString(scaled_vol),20,60);
	
	ofPushStyle();
	ofSetColor(245,58,135);
	ofNoFill();
	

	float rad=60;
	int ngrid=5;
	int mgrid=ngrid*ngrid;

	ofPushMatrix();
	ofTranslate(ofGetWidth()/2-rad*ngrid/2,ofGetHeight()/2-rad*ngrid/2);
	for(int i=0;i<mgrid;++i){		
		float px=rad*float(i%ngrid);
		float py=rad*float(floor(i/ngrid));
		int b=i+(BUFFER_SIZE-mgrid);//floor((float)i/(float)ngrid*(float)BUFFER_SIZE);
		ofDrawCircle(px,py,rad*(.4+.6*beat.getBand(b)));
	}
	ofPopMatrix();

	
	ofPopStyle();

	switch(mode){
		case REC:
			ofDrawBitmapString("REC "+ofToString(rec_timer.count()),20,20);
			break;
		case PLAY:
			ofDrawBitmapString("PLAY",20,20);
			break;
		case SLEEP:
			ofDrawBitmapString("SLEEP, red to rec",20,20);
			break;
		case FINISH:
			ofDrawBitmapString("FINSH "+ofToString(finish_timer.count()),20,20);
			ofDrawBitmapString("red to re-rec, white to play",20,40);
			break;
		case QRCODE:
			if(qrcode.isAllocated()){
				ofSetColor(255,255,255);
				qrcode.draw(ofGetWidth()/2-qrcode.getWidth()/2,ofGetHeight()/2-qrcode.getHeight()/2,0);
			}
			ofDrawBitmapString("QRCODE "+ofToString(qrcode_timer.count()),20,20);
			break;
		case STORED:
			ofDrawBitmapString("STORE, white to play",20,20);
			break;
	}
	ofDrawBitmapString(ofToString(ofGetFrameRate()),10,10);
}

//--------------------------------------------------------------
// audio
//--------------------------------------------------------------
void ofApp::audioIn(float *input,int buffer_size,int nchannels){
	
	if(rec_timer.val()>0)
		audio_recorder.addSamples(input,buffer_size*nchannels);
	
	calcVolume(input,buffer_size,nchannels);

}
void ofApp::calcVolume(float *data,int buffer_size,int nchannels){
	float cur_vol=0;
	for(int i=0;i<buffer_size;++i){
		cur_vol+=data[i]*data[i]*Param::val()->sound_scale;
	}
	cur_vol/=(float)buffer_size;
	cur_vol=sqrt(cur_vol);
	smooth_vol*=0.8;
	smooth_vol+=0.2*cur_vol;
	
	//record.insert(record.begin(),smooth_vol);
	//if(record.size()>8) record.pop_back();
//	cout<<record[record.size()-1]<<endl;


	//BmFFT::getSimpleSpectrum(BUFFER_SIZE,data,band_volume);
	beat.audioReceived(data,buffer_size,nchannels);

}


void ofApp::startRecord(){
	
	mode=MODE::REC;
	ofLog()<<"Start!";
	
	cur_id=createId();
	string path_="audio_"+cur_id+".wav";
	audio_recorder.setup(path_);
	audio_recorder.setFormat(SF_FORMAT_WAV|SF_FORMAT_PCM_16);
	
	//rec_start=ofGetElapsedTimeMillis();
	rec_timer.restart();

	last_record=path_;

}
void ofApp::stopRecord(){
	
	ofLog()<<"Stop!";
	audio_recorder.finalize();
	mode=MODE::FINISH;

	finish_timer.restart();
	ofLog()<<"Finish!";

	//uploadFile(cur_id,ofToDataPath("audio_"+cur_id+".wav",true));	

}

void ofApp::playRecord(){
	
	ofLog()<<"Play file: "<<last_record<<"!";
	if(last_record.length()<1) return;
	//mode=MODE::PLAY;
	//out_sound_stream.start();
	
	wav_reader.setup(last_record);
	//play_start=ofGetElapsedTimeMillis();
	playing=true;

}

void ofApp::stopPlayRecord(){
	ofLog()<<"End!";
	if(mode==MODE::FINISH) finish_timer.restart();
	playing=false;	 	
	//out_sound_stream.stop();
	//mode=MODE::SLEEP;
}

void ofApp::audioOut(float *output,int buffer_size,int nchannels){

	if(!playing) return;
	
	float* buf=new float[buffer_size];
	int count_=wav_reader.read(buf,buffer_size);
	//cout<<count_<<endl;

	for(int i=0;i<buffer_size;++i){
	
		output[i*nchannels]=buf[i];
		output[i*nchannels+1]=buf[i];
	}
	calcVolume(output,buffer_size,nchannels);
	if(count_==0) stopPlayRecord();

	delete buf;
}


//-----------------------------------------------------------------------
// mode
//-----------------------------------------------------------------------
void ofApp::setMode(MODE mode_){

	switch(mode_){
		case SLEEP:
		case REC:
		case FINISH:
		case QRCODE:
		case STORED:
		case PLAY:
			break;
	}



}
void ofApp::resetSleep(){
	mode=MODE::SLEEP;
	rec_timer.reset();
	finish_timer.reset();
	qrcode_timer.reset();
}

void ofApp::exit(){
	ofSoundStreamClose();
	Param::val()->saveLastRecord(last_record);
}


//-----------------------------------------------------------------------
// upload
//-----------------------------------------------------------------------
void ofApp::newResponse(HttpFormResponse &response){
	if(response.status==200){
		ofLog()<<"Upload ready!";
		qrcode.fetch(Param::val()->share_url+"?id="+cur_id,200);
	}
}
void ofApp::uploadFile(string id_,string file_){

	HttpForm f=HttpForm(Param::val()->upload_url);
	f.addFile("file",file_);
	f.addFormField("action","upload_wav");
	f.addFormField("guid",id_);

	upload_manager.submitForm(f,false);

}

string ofApp::createId(){
	return ofGetTimestampString();
}





