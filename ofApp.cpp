#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
	
	ofHideCursor();
	
	qrcode_timer=FrameTimer(Param::val()->qrcode_time*1000,0);
	qrcode_timer.setContinuous(false);

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


	serial.setup("/dev/ttyUSB0",9600);
	if(serial.isInitialized()) ofLog()<<"Serial Initialized!!";
	else ofLog()<<"Serial Fail!";
	

	osc_receive.setup(Param::val()->osc_port);

	ofAddListener(upload_manager.formResponseEvent,this,&ofApp::newResponse);

	//scene videos
	back_seq=ImageSeq("src/back/back",119,24,0,".jpg");
	logo_seq=ImageSeq("src/logo/logo",104,24,0,".png");

	for(int i=0;i<9;++i){
		front_image[i].load("src/front_"+ofToString(i)+".png");
	}
	for(int i=0;i<10;++i){
		count_image[i].load("src/count_"+ofToString(i)+".png");
	}
	
	hint_timer=FrameTimer(2000);
	ofAddListener(hint_timer.finish_event,this,&ofApp::onHintTimerEnd);
	
	count_timer=FrameTimer(1000);
	ofAddListener(count_timer.finish_event,this,&ofApp::onCountTimerEnd);

	blink_timer=FrameTimer(500);
	ofAddListener(blink_timer.finish_event,this,&ofApp::onBlinkTimerEnd);
	
	glow_timer=FrameTimer(2000);
	
	

	transition_timer=FrameTimer(1000);
	transition_timer.setContinuous(false);
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

	if(recording || playing){
		scaled_vol=smooth_vol*60.0*255.0;//ofMap(smooth_vol,0,0.1,0,1.0,true);
		//if(scaled_vol<35) scaled_vol=5;
		//else scaled_vol=(scaled_vol-35)*20.0+35;

		writeSerial(ofToString(int(ofClamp(scaled_vol,160.0,255.0))));
	}else if(mode==MODE::SLEEP){
		writeSerial(ofToString(int(255.0*sin(3.14*breath_timer.val()))));
	}

	//if(scaled_vol<40) scaled_vol=0;


	int key_=readSerial();
	if(key_==4) resetSleep();
	handleOsc();

	//if(mode!=MODE::QRCODE) back_video.update();
	transition_timer.update(dm);
	
	//back_video.update();	
	back_seq.update(dm);

	//int fr=back_seq.getIndex();
	bool transition_=(mode!=next_mode);
	
	switch(mode){
		case SLEEP:
			breath_timer.update(dm);

			logo_seq.update(dm);
			if(key_==1) closeMode(MODE::HINT);
			break;
		case HINT:
			hint_timer.update(dm);
			break;
		case REC:
			//rec_timer.update(dm);
			count_timer.update(dm);
			blink_timer.update(dm);			
			break;
		case FINISH:
			//if(fr>=687) back_seq.setIndex(662);
			glow_timer.update(dm);
			if(!transition_ && !recording && !playing){
				if(key_==1){
					if(!record_once){
						record_once=true;
						closeMode(MODE::REC);
					}
				}else if(key_==2){
					playRecord();
				}else if(key_==3){
					closeMode(MODE::QRCODE);
				}
			}
			break;
		case QRCODE:
			qrcode_timer.update(dm);
			//if(fr>=696) back_seq.setIndex(688);
			if(!transition_){
				if(qrcode_timer.val()==1){
					//qrcode_timer.reset();
					//cout<<"qrcode finish!";
					//mode=MODE::STORED;
					closeMode(MODE::STORED);
				}
				//if(key_==2) playRecord();
			}
			break;
		case PLAY:
			break;
		case STORED:
			glow_timer.update(dm);
			//if(fr>=705) back_seq.setIndex(697);
			//key_=readSerial();
			if(!transition_)
				if(key_==2) playRecord();
			break;
	}
	
	
	//ofSoundUpdate();
	//if(playing && sound_player.getPosition()==1.0) stopPlayRecord();

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
	if(mode==next_mode) drawMode(mode,1.0,false);
	else drawMode(mode,1.0f-t_,true);

}

void ofApp::drawMode(MODE mode_,float t,bool fade_out){

	float h=375;
	float w=h*1.2;

	back_seq.getCurFrame().draw(0,0,h*1.2,h);
	
	ofSetColor(255,255.0f*t);
	
	ofPushStyle();
	ofPushMatrix();
//	int width=ofGetWidth();
//	int height=ofGetHeight();
	float tt=t;
//	ofTranslate(w/2,h/2);
//	ofScale(t,t);
//	ofTranslate(-w/2,-h/2);
	
	bool fade_in=(!fade_out && t<1.0);

	switch(mode_){
		case SLEEP:
			logo_seq.getCurFrame().draw(0,0,w,h);
			ofDrawBitmapString("SLEEP, red to rec",20,20);
			break;
		case HINT:
			ofPushStyle();
			if(fade_out) ofSetColor(255.0);
				front_image[0].draw(0,0,w,h);
			ofPopStyle();

			if(!fade_in && !fade_out){
				tt*=1.0-hint_timer.eval();
				ofPushStyle();
					ofSetColor(255,255.0*tt);
					front_image[hint_timer.num()+1].draw(0,0,w,h);
				ofPopStyle();
			}
			break;
		case REC:
			ofPushStyle();
			if(fade_in) ofSetColor(255.0);
				front_image[0].draw(0,0,w,h);
			ofPopStyle();

			if(recording){
				tt*=1.0-count_timer.eval();
				ofDrawBitmapString("REC",20,20);
				ofPushStyle();
					ofSetColor(255,255.0*tt);
					count_image[(int)ofClamp(9-count_timer.num(),0,9)].draw(0,0,w,h);
				ofPopStyle();
			}else{
				if(blink_timer.val()>0){
					tt*=1.0-blink_timer.eval();
					ofPushStyle();
					ofSetColor(255,255.0*tt);
						front_image[3].draw(0,0,w,h);
					ofPopStyle();
				}
			}
			break;
			
		case FINISH:
			tt*=sin(3.14*glow_timer.val());
			ofDrawBitmapString("FINSH",20,20);
			ofPushStyle();
				front_image[4].draw(0,0,w,h);
				ofSetColor(255,255.0*(1.0-tt));
				front_image[5].draw(0,0,w,h);
			ofPopStyle();
			break;
		case QRCODE: 
			if(qrcode.isAllocated()){
				front_image[6].draw(0,0,w,h);
				ofSetColor(255,255,255);
				ofPushMatrix();
				ofScale(h/320.0,h/320.0);
					qrcode.draw(89,66,203,203);
				ofPopMatrix();
				ofDrawBitmapString("QRCODE "+ofToString(qrcode_timer.count()),20,20);
			}
			break;
		case STORED:
			front_image[7].draw(0,0,w,h);
			tt*=sin(3.14*glow_timer.val());
			ofPushStyle();
			ofSetColor(255,255.0*tt);
				front_image[8].draw(0,0,w,h);
			ofPopStyle();
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
				
		calcVolume(input,buffer_size,nchannels);
	}
}
void ofApp::calcVolume(float *data,int buffer_size,int nchannels){
	float cur_vol=0;
	for(int i=0;i<buffer_size;++i){
	 	cur_vol+=data[i]*data[i]*Param::val()->sound_scale;
	//	record_float.push_back(data[i]);
	}
//	read_record++;

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
	
	//record_float.clear();
//	vector<float>().swap(record_float);
//	read_record=0;

	cur_id=createId();
	string path_="audio_"+cur_id+".wav";
	//string path_="audio.wav";
	audio_recorder.setup(path_);
	audio_recorder.setFormat(SF_FORMAT_WAV|SF_FORMAT_PCM_16);
	

	//last_record=path_;
	recording=true;


}
void ofApp::stopRecord(){
	
	if(!recording) return;

	ofLog()<<"Stop!";
	audio_recorder.finalize();

	ofLog()<<"Finish! #buffer= "<<read_record;
	recording=false;
}

void ofApp::playRecord(){
	if(playing) return;

	ofLog()<<"Play file !";

	

	if(sound_file!=NULL && sound_file->isLoaded()){
		if(last_record==cur_id){
			sound_file->seekTo(0);
			playing=true;
			read_record=0;
			return;
		}
		else{
			sound_file->close();
			delete sound_file;
			sound_file=NULL;
		}
	}
	last_record=cur_id;

	ofxSoundFile *file_=new ofxSoundFile("audio_"+last_record+".wav");
	sound_file=file_;
	if(sound_file->isLoaded()){
		ofLog()<<"file loaded!";
		sound_file->seekTo(0);
		playing=true;
		read_record=0;
	}else ofLog()<<"file not loaded!";
	

}

void ofApp::stopPlayRecord(){
	
	if(!playing) return;
	
	writeSerial(ofToString(255));

	ofLog()<<"End!";
	playing=false;	 	
}

void ofApp::audioOut(float *output,int buffer_size,int nchannels){

	if(!playing) return;
	
	if(sound_file==NULL){
		ofLog()<<"null file!";
		stopPlayRecord();
		return;
	}


	ofSoundBuffer buf;
	int read_len=sound_file->readTo(buf,buffer_size);
	//ofLog()<<"buf len= "<<read_len;
	if(read_len==0){
		ofLog()<<"end of sound file!";
		stopPlayRecord();
		return;
	}

	for(int i=0;i<buffer_size;++i){
		int r=ofClamp(i,0,buf.size()-1);
		output[i*nchannels]=buf[r];
		output[i*nchannels+1]=buf[r];
	}
	read_record++;
	calcVolume(output,buffer_size,nchannels);

}


//-----------------------------------------------------------------------
// mode
//-----------------------------------------------------------------------
void ofApp::closeMode(MODE next_mode_){
	
	ofLog()<<mode<<"->"<<next_mode_;

	back_seq.setPause(true);

	switch(mode){
		case SLEEP:
			record_once=false;
			logo_seq.setPause(true);
			break;
		case REC:
			count_timer.stop();
			blink_timer.stop();
			break;
		case HINT:
			hint_timer.stop();
			break;
		case FINISH:
			break;
		case QRCODE:
			//uploadFile(cur_id,ofToDataPath("audio_"+cur_id+".wav",true));	
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
			logo_seq.reset();
			logo_seq.start();
			//writeSerial("a");
			break;
		case HINT:
			writeSerial(ofToString(128));
			//hint_timer.restart();
			break;
		case REC:
			writeSerial(ofToString(128));
		//	count_timer.restart();
			//back_seq.setIndex(284);
			//startRecord();
			break;
		case FINISH:
			glow_timer.restart();
			writeSerial(ofToString(255));
			break;
		case QRCODE:
			//writeSerial("b");
			writeSerial(ofToString(255));
			//uploadFile(cur_id,ofToDataPath("audio_"+cur_id+".wav",true));	
			//back_seq.setIndex(688);
			//back_seq.setPause(true);
			qrcode_timer.restart();
			break;
		case STORED:
			glow_timer.restart();
			writeSerial(ofToString(255));
			//back_seq.setIndex(697);
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
			case HINT:
				hint_timer.restart();
				break;
			case REC:
				count_timer.restart();
				startRecord();
				break;
			case QRCODE:
				//qrcode_timer.restart();
				uploadFile(cur_id,ofToDataPath("audio_"+cur_id+".wav",true));	
			
				cout<<"get qrcode!"<<endl;
				qrcode.clear();
				qrcode.fetch(Param::val()->share_url+"?id="+cur_id,200);
				qrcode.load("qrcode.jpg");

				break;
			case STORED:
				break;
		}
	}
	
}

void ofApp::onHintTimerEnd(int &num){
	ofLog()<<"num "<<num;
	if(num==2){
		closeMode(MODE::REC);
	}
}
void ofApp::onCountTimerEnd(int &num){

	ofLog()<<"count "<<num;

	if(num==10){
		count_timer.stop();
		stopRecord();
		blink_timer.restart();
	}
}
void ofApp::onBlinkTimerEnd(int &num){
	ofLog()<<"blink "<<num;
	if(num==3){
		if(!record_once) closeMode(MODE::FINISH);
		else closeMode(MODE::QRCODE);
	}
}


void ofApp::resetSleep(){
	closeMode(MODE::SLEEP);
	//rec_timer.reset();
	//finish_timer.reset();
	//qrcode_timer.reset();
}

void ofApp::exit(){
	
	if(sound_file){
		sound_file->close();
		delete sound_file;
	}

	ofSoundStreamClose();
	ofSoundStopAll();
	ofSoundShutdown();
	//wav_reader.close();
	//record_float.clear();
	//vector<float>().swap(record_float);

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





