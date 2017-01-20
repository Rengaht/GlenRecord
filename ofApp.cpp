#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
	
	ofHideCursor();
	
	//rec_timer=FrameTimer(Param::val()->rec_time*1000,Param::val()->count_time*1000);
	qrcode_timer=FrameTimer(Param::val()->qrcode_time*1000,0);
	//finish_timer=FrameTimer(Param::val()->finish_time*1000,0);
	//cout<<Param::val()->finish_time<<endl;
	breath_timer=FrameTimer(1500);
	breath_timer.restart();

	cur_millis=ofGetElapsedTimeMillis();

	scaled_vol=0;
	smooth_vol=0;
	
	//mode=MODE::SLEEP;

	last_record=Param::val()->last_record;
	
	out_sound_stream.printDeviceList();
	out_sound_stream.setup(this,2,1,SAMPLE_RATE,BUFFER_SIZE,4);
	//out_sound_stream.stop();


	serial.setup("/dev/ttyACM0",9600);
	if(serial.isInitialized()) ofLog()<<"Serial Initialized!!";
	else ofLog()<<"Serial Fail!";
	

	osc_receive.setup(Param::val()->osc_port);

	ofAddListener(upload_manager.formResponseEvent,this,&ofApp::newResponse);

	//scene videos
	back_seq=ImageSeq("seq2/video_",706,30,180);

	transition_timer=FrameTimer(1000);
	ofAddListener(transition_timer.finish_event,this,&ofApp::onTransitionEnd);

	startMode(MODE::SLEEP);
	writeSerial("a");
}

//--------------------------------------------------------------
void ofApp::update(){
	
	float dm=ofGetElapsedTimeMillis()-cur_millis;
	cur_millis+=dm;
	beat.update(cur_millis);
//	breath_timer.update(dm);

	if(mode!=SLEEP){
		scaled_vol=ofMap(smooth_vol,0,0.17,0,1.0,true);
		writeSerial(ofToString(int(ofMap(ofClamp(scaled_vol,0.5,1),0.5,1,10.0,255.0))));
	}else{
		writeSerial(ofToString(int(255.0*transition_timer.val())));
	}

	int key_=readSerial();
	if(key_==4) resetSleep();
	handleOsc();

	//if(mode!=MODE::QRCODE) back_video.update();
	transition_timer.update(dm);
	
	//back_video.update();	
	back_seq.update(dm);

	int fr=back_seq.getIndex();
	bool transition_=(mode!=next_mode);

	switch(mode){
		case SLEEP:
			if(breath_timer.val()==1) breath_timer.restart();

			if(fr>=284) back_seq.setIndex(180);	
			if(key_==1) closeMode(MODE::REC);
			break;
		case REC:
			if(!transition_){
				if(fr>=651){
				//	back_seq.setPause(true);
					stopRecord();
					closeMode(MODE::FINISH);
				}else if(fr>=393){
					startRecord();
				}
			}
			break;
		case FINISH:
			if(fr>=687) back_seq.setIndex(662);
			if(!transition_){
				if(key_==1){
					closeMode(MODE::REC);
				}else if(key_==2){
					playRecord();
				}else if(key_==3){
					closeMode(MODE::QRCODE);
				}
			}
			break;
		case QRCODE:
			qrcode_timer.update(dm);
			if(fr>=696) back_seq.setIndex(688);
			if(!transition_){
				if(qrcode_timer.val()==1){
					qrcode_timer.reset();
					//cout<<"qrcode finish!";
					//mode=MODE::STORED;
					closeMode(MODE::STORED);
				}
				if(key_==2) playRecord();
			}
			break;
		case PLAY:
			break;
		case STORED:
			if(fr>=705) back_seq.setIndex(697);
			//key_=readSerial();
			if(!transition_)
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
void ofApp::writeSerial(string val_){
	if(serial.isInitialized()){
		//ofLog()<<"serial write: "<<val_;

		string str_=val_+'|';
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
	ofSetBackgroundColor(0);
	
//	back_video.draw(0,0);
//	front_video[0].draw(0,0);

	//back_seq.getCurFrame().draw(0,0);

//	ofSetColor(255);
//	ofDrawBitmapString("vol= "+ofToString(scaled_vol),20,60);
	

	//draw fft circles
/*	ofPushStyle();
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
*/


	float t_=transition_timer.val();
	if(mode==next_mode) drawMode(mode,t_);
	else drawMode(mode,1.0f-t_);

}

void ofApp::drawMode(MODE mode_,float t){

	ofPushStyle();
	ofSetColor(255,255.0f*t);
	float h=375;
	back_seq.getCurFrame().draw(0,0,h*1.2,h);

	ofPushMatrix();
	int width=ofGetWidth();
	int height=ofGetHeight();

	switch(mode_){
		case REC:
			ofDrawBitmapString("REC",20,20);
			break;
		case PLAY:
			ofDrawBitmapString("PLAY",20,20);
			break;
		case SLEEP:
			ofDrawBitmapString("SLEEP, red to rec",20,20);
			break;
		case FINISH:
			ofDrawBitmapString("FINSH",20,20);
			ofDrawBitmapString("red to re-rec, white to play",20,40);
			break;
		case QRCODE:
			if(qrcode.isAllocated()){
				ofSetColor(255,255,255);
				ofPushMatrix();
				ofScale(h/320.0,h/320.0);
					qrcode.draw(89,66,203,203);
				ofPopMatrix();
			}else{
				ofSetColor(255);
				ofDrawRectangle(89,66,203,203);
			}
			ofDrawBitmapString("QRCODE "+ofToString(qrcode_timer.count()),20,20);
			break;
		case STORED:
			//qrcode_back.draw(0,0,width,height);
			ofDrawBitmapString("STORE, white to play",20,20);
			break;
	}

	ofDrawBitmapString(ofToString(ofGetFrameRate()),10,10);
	ofDrawBitmapString(ofToString(scaled_vol),10,30);
	ofPopMatrix();
	ofPopStyle();

}

//--------------------------------------------------------------
// audio
//--------------------------------------------------------------
void ofApp::audioIn(float *input,int buffer_size,int nchannels){
	
	//if(rec_timer.val()>0)
	int fr=back_seq.getIndex();
	if(recording){
	//	cout<<"record.. ";
		audio_recorder.addSamples(input,buffer_size*nchannels);
		
	}
	calcVolume(input,buffer_size,nchannels);
	
}
void ofApp::calcVolume(float *data,int buffer_size,int nchannels){
	float cur_vol=0;
	for(int i=0;i<buffer_size;++i){
	 	cur_vol+=data[i]*data[i]*Param::val()->sound_scale;
//		record_float.push_back(data[i]);
	}
	cur_vol/=(float)buffer_size;
	cur_vol=sqrt(cur_vol);
	smooth_vol*=0.8;
	smooth_vol+=0.2*cur_vol;
	
//	delete data;
//	data=NULL;


	//record.insert(record.begin(),smooth_vol);
	//if(record.size()>8) record.pop_back();
//	cout<<record[record.size()-1]<<endl;


	//BmFFT::getSimpleSpectrum(BUFFER_SIZE,data,band_volume);
	//beat.audioReceived(data,buffer_size,nchannels);

}


void ofApp::startRecord(){

	if(recording) return;
	
	ofLog()<<"Start!";
	
//	record_float.clear();

	cur_id=createId();
	string path_="audio_"+cur_id+".wav";
	//string path_="audio.wav";
	audio_recorder.setup(path_);
	audio_recorder.setFormat(SF_FORMAT_WAV|SF_FORMAT_PCM_16);
	

	last_record=path_;
	recording=true;
}
void ofApp::stopRecord(){
	
	if(!recording) return;

	ofLog()<<"Stop!";
	audio_recorder.finalize();

	ofLog()<<"Finish!";
	recording=false;

}

void ofApp::playRecord(){
	if(playing) return;

	ofLog()<<"Play file: "<<last_record<<"!";
	if(last_record.length()<1) return;
	//mode=MODE::PLAY;
	//out_sound_stream.start();
	
	wav_reader.setup(last_record);
	//wav_reader.setup("audio.wav");
	//play_start=ofGetElapsedTimeMillis();
	playing=true;

}

void ofApp::stopPlayRecord(){
	
	if(!playing) return;

	ofLog()<<"End!";
//	if(mode==MODE::FINISH) finish_timer.restart();
	playing=false;	 	
	//out_sound_stream.stop();
	//mode=MODE::SLEEP;
}

void ofApp::audioOut(float *output,int buffer_size,int nchannels){

	if(!playing) return;
//	cout<<"play ";
	float* buf=new float[buffer_size];
	int count_=wav_reader.read(buf,buffer_size);
	//cout<<count_<<endl;

	for(int i=0;i<buffer_size;++i){
	
		output[i*nchannels]=buf[i];
		output[i*nchannels+1]=buf[i];
	}
	calcVolume(output,buffer_size,nchannels);
	if(count_<BUFFER_SIZE) stopPlayRecord();

	delete buf;
//	delete output;
}


//-----------------------------------------------------------------------
// mode
//-----------------------------------------------------------------------
void ofApp::closeMode(MODE next_mode_){
	
	ofLog()<<mode<<"->"<<next_mode_;

	back_seq.setPause(true);

	switch(next_mode_){
		case SLEEP:
		case REC:
		case FINISH:
			break;
		case QRCODE:
			uploadFile(cur_id,ofToDataPath("audio_"+cur_id+".wav",true));	
			//uploadFile(cur_id,ofToDataPath("audio.wav",true));
			break;
		case STORED:
		case PLAY:
			break;
	}
	next_mode=next_mode_;
	transition_timer.restart();
}
void ofApp::startMode(MODE mode_){
	
	back_seq.setPause(false);

	switch(mode_){
		case SLEEP:
			back_seq.reset();
			back_seq.start();
			writeSerial("a");
			break;
		case REC:
			back_seq.setIndex(284);
			//startRecord();
			break;
		case FINISH:
			break;
		case QRCODE:
			writeSerial("b");
			back_seq.setIndex(688);
			//back_seq.setPause(true);
			qrcode_timer.restart();
			break;
		case STORED:
			back_seq.setIndex(697);
			break;
		case PLAY:
			break;
	}
	//back_seq.setPause(false);

	mode=mode_;
	transition_timer.restart();
	ofLog()<<"set mode: "<<mode;
}

void ofApp::onTransitionEnd(int &e){
	ofLog()<<"transition end!";
	if(next_mode!=mode){
		startMode(next_mode);
	}else{
		switch(mode){
			case REC:
				startRecord();
				break;
			case QRCODE:
				qrcode_timer.restart();
				cout<<"get qrcode!"<<endl;
				qrcode.fetch(Param::val()->share_url+"?id="+cur_id,200);
				qrcode.load("qrcode.jpg");
				break;
			case STORED:
				break;
		}
	}
	
}


void ofApp::resetSleep(){
	closeMode(MODE::SLEEP);
	//rec_timer.reset();
	//finish_timer.reset();
	//qrcode_timer.reset();
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
		//qrcode.fetch(Param::val()->share_url+"?id="+cur_id,200);
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





