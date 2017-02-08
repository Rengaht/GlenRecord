#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
	
	ofHideCursor();
	
	qrcode_timer=FrameTimer(Param::val()->qrcode_time*1000,0);
	qrcode_timer.setContinuous(false);

	breath_timer=FrameTimer(2500);
	breath_timer.restart();

	cur_millis=ofGetElapsedTimeMillis();

	scaled_vol=0;
	smooth_vol=0;
	
	//mode=MODE::SLEEP;

	last_record=Param::val()->last_record;
	


	serial.setup("/dev/ttyUSB0",9600);
	if(serial.isInitialized()) ofLog()<<"Serial Initialized!!";
	else ofLog()<<"Serial Fail!";
	

	osc_receive.setup(Param::val()->osc_port);

	ofAddListener(upload_manager.formResponseEvent,this,&ofApp::newResponse);

	//scene videos
	back_seq=ImageSeq("src/back/back",119,24,0,".jpg");
	logo_seq=ImageSeq("src/logo/logo",103,24,0,".png");

	for(int i=0;i<9;++i){
		front_image[i].load("src/front_"+ofToString(i)+".png");
	}
	for(int i=0;i<10;++i){
		count_image[i].load("src/count_"+ofToString(i)+".png");
	}
	
	dino_seq=ImageSeq("src/dinosaure/",3,6,2,".png");
	for(int i=0;i<3;++i){
		cat_image[i].load("src/dinosaure/c"+ofToString(i+1)+".png");
	}
	over_image.load("src/dinosaure/over.png");
	dead_image.load("src/dinosaure/005.png");


	hint_timer=FrameTimer(3000);
	hint_timer.setContinuous(false);
	ofAddListener(hint_timer.finish_event,this,&ofApp::onHintTimerEnd);
	
	go_timer=FrameTimer(1000);
	go_timer.setContinuous(false);
	ofAddListener(go_timer.finish_event,this,&ofApp::onGoTimerEnd);
	
	count_timer=FrameTimer(1000);
	ofAddListener(count_timer.finish_event,this,&ofApp::onCountTimerEnd);

	blink_timer=FrameTimer(500);
	ofAddListener(blink_timer.finish_event,this,&ofApp::onBlinkTimerEnd);
	
	glow_timer=FrameTimer(2000);
	
	jump_timer=FrameTimer(800);
	jump_timer.setContinuous(false);
	

	transition_timer=FrameTimer(1000);
	transition_timer.setContinuous(false);
	ofAddListener(transition_timer.finish_event,this,&ofApp::onTransitionEnd);

	cat_val=3000.0;
	for(int i=0;i<2;++i){
		cat_timer[i]=FrameTimer(cat_val,cat_val*i+cat_val*ofRandom(.5,1.5));
		cat_timer[i].setContinuous(false);
		cat_id[i]=(int)ofRandom(0,3);
	}

	sound_fx[0].load("src/sound/button.wav");
	//sound_fx[1].load("src/sound/10sec.wav");
	//sound_fx[2].load("src/sound/go.wav");
	//sound_fx[3].load("src/sound/blink.wav");
	
	 out_sound_stream.printDeviceList();
	 out_sound_stream.setup(this,2,1,SAMPLE_RATE,BUFFER_SIZE,4);
	//out_sound_stream.stop();
	

	next_mode=MODE::SLEEP;
	startMode(MODE::SLEEP);

//	writeSerial("a");
}

//--------------------------------------------------------------
void ofApp::update(){
	
	float dm=ofGetElapsedTimeMillis()-cur_millis;
	cur_millis+=dm;
//	beat.update(cur_millis);
//	breath_timer.update(dm);

	if(recording || playing){
		scaled_vol=smooth_vol*250.0*255.0;//ofMap(smooth_vol,0,0.1,0,1.0,true);
		//if(scaled_vol<35) scaled_vol=5;
		//else scaled_vol=(scaled_vol-35)*20.0+35;

		writeSerial(ofToString(int(ofClamp(scaled_vol,160.0,255.0))));
	}else if(mode==MODE::SLEEP){
		writeSerial(ofToString(160+int(95.0*sin(3.14*breath_timer.val()))));
	}

	//if(scaled_vol<40) scaled_vol=0;


	int key_=readSerial();
	if(key_==4) resetSleep();
	if(key_==7) closeMode(MODE::DINO);

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
			if(key_==1){
				closeMode(MODE::HINT);
				sound_fx[0].setPlay(true);
			}
			break;
		case HINT:
			hint_timer.update(dm);
			go_timer.update(dm);
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
						closeMode(MODE::HINT);
						sound_fx[0].setPlay(true);
					}
				}else if(key_==2){
					playRecord();
					sound_fx[0].setPlay(true);			
				}else if(key_==3){
					closeMode(MODE::QRCODE);
					sound_fx[0].setPlay(true);			
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
				if(key_==2){
					playRecord();
					sound_fx[0].setPlay(true);
				}
			break;
		case DINO:
			dino_seq.update(dm);
			jump_timer.update(dm);
			for(int i=0;i<2;++i){
				cat_timer[i].update(dm);
				float val=cat_timer[i].val();
				if(val==1){
					cat_timer[i]=FrameTimer(cat_val,cat_val*ofRandom(.5,1.5));
					cat_timer[i].restart();
					cat_id[i]=(int)ofRandom(0,3);
					cat_val*=.98;
					
				}else if(val>=0.45 && val<=0.5){
				//	cout<<"c- "<<val<<"  j-"<<jump_timer.val()<<endl;
					if(jump_timer.val()<.1 || jump_timer.val()>.9){
						dino_dead=true;
						jump_timer.stop();
						for(int i=0;i<2;++i) cat_timer[i].stop();
					}
				}
			}
			if(dino_dead){
				
				if(key_==2){ //reset game
					jump_timer.reset();
					cat_val=3000;
					for(int i=0;i<2;++i){
						cat_timer[i]=FrameTimer(cat_val,cat_val*i+cat_val*ofRandom(.5,1.5));
						cat_timer[i].restart();
						cat_id[i]=(int)ofRandom(0,3);
					}
					dino_dead=false;
				}
			
			}else{
				if(key_==1) jump_timer.restart();
			}
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
	if(message_.getAddress()=="/dino"){
		closeMode(MODE::DINO);
	}
	
}


//--------------------------------------------------------------
void ofApp::draw(){
	ofSetBackgroundColor(0);
//	front_image[0].draw(0,0,676,480);

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

	float h=480;
	float w=h*1.2;
	float offx=Param::val()->screen_offsetx;
	
	ofPushMatrix();
	ofTranslate(0,offx+w);
	ofRotateZ(-90);
	
	

	float t_=transition_timer.eval();
	if(mode==next_mode) drawMode(mode,t_,false);
	else drawMode(mode,1.0f-t_,true);
	


	ofPopMatrix();

}

void ofApp::drawMode(MODE mode_,float t,bool fade_out){

	float h=480;
	float w=h*1.2;

	ofPushMatrix();

	if(mode_==MODE::DINO) ofSetBackgroundColor(255);
	else back_seq.getCurFrame().draw(0,0,h*1.2,h);
	
	
	ofPushStyle();
	ofSetColor(255,255.0f*t);
	
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
			//ofDrawBitmapString("SLEEP, red to rec",20,20);
			break;
		case HINT:
			ofPushStyle();
			if(fade_out) ofSetColor(255.0,180);
				front_image[0].draw(0,0,w,h);
			ofPopStyle();

			if(!fade_in && !fade_out){
				
				if(hint_timer.val()<1) tt*=1.0-hint_timer.eval();
				else tt*=1.0-go_timer.eval();

				ofPushStyle();
					ofSetColor(255,255.0*tt);
					front_image[hint_timer.num()+1].draw(0,0,w,h);
				ofPopStyle();
			}
			break;
		case REC:
			ofPushStyle();
			if(fade_in) ofSetColor(255.0,180);
				front_image[0].draw(0,0,w,h);
			
			ofPopStyle();

			if(recording){
				tt*=1.0-count_timer.eval();
				//ofDrawBitmapString("REC",20,20);
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
			//ofDrawBitmapString("FINSH",20,20);
			ofPushStyle();
				front_image[4].draw(0,0,w,h);
				ofSetColor(255,255.0*tt);
				front_image[5].draw(0,0,w,h);
			ofPopStyle();
			break;
		case QRCODE: 
			if(qrcode.isAllocated()){
				front_image[6].draw(0,0,w,h);
				ofPushMatrix();
				ofScale(h/320.0,h/320.0);
					qrcode.draw(89,66,203,203);
				ofPopMatrix();
				//ofDrawBitmapString("QRCODE "+ofToString(qrcode_timer.count()),20,20);
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
			//ofDrawBitmapString("STORE, white to play",20,20);
			break;
		case DINO:
			ofPushStyle();
			
			for(int i=0;i<2;++i){
				float val=cat_timer[i].val();
				if(val>0 && val<1){
					ofPushMatrix();
					ofTranslate(536-656.0*val,240);
					ofScale(.5,.5);
						cat_image[cat_id[i]].draw(0,0);
					ofPopMatrix();
				}
			}
			ofPushStyle();
			ofSetColor(120);
				ofDrawLine(280,0,675,0);
			ofPopStyle();
			
			float jt=sin(3.1416*jump_timer.val());
			ofPushMatrix();
			ofTranslate(267,240.0-40.0*jt);
				if(!dino_dead) dino_seq.getCurFrame().draw(0,0,42,45);
				else{
					dead_image.draw(0,0,42,45);
					over_image.draw(-172.5,-40);
				}
			ofPopMatrix();

			ofPopStyle();
			break;
	}

	//ofDrawBitmapString(ofToString(ofGetFrameRate()),10,10);
	//ofDrawBitmapString(ofToString(scaled_vol),10,30);
	ofPopMatrix();
	ofPopStyle();

	ofPopMatrix();

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
	}

	cur_vol/=(float)buffer_size;
	cur_vol=sqrt(cur_vol);
	smooth_vol*=0.8;
	smooth_vol+=0.2*cur_vol;
	


	//BmFFT::getSimpleSpectrum(BUFFER_SIZE,data,band_volume);
	//beat.audioReceived(data,buffer_size,nchannels);

}


void ofApp::startRecord(){

	if(recording) return;
	
	ofLog()<<"Start!";
	

	cur_id=createId();
	string path_="audio_"+cur_id+".wav";
	audio_recorder.setup(path_);
	audio_recorder.setFormat(SF_FORMAT_WAV|SF_FORMAT_PCM_16);
	

	recording=true;


}
void ofApp::stopRecord(){
	
	if(!recording) return;

	recording=false;
	
	ofLog()<<"Stop!";

	audio_recorder.finalize();
	


	
	ofLog()<<"Finish! #buffer= "<<read_record;
	//recording=false;
}

void ofApp::playRecord(){
	
	if(mode!=MODE::STORED && mode!=MODE::FINISH) return;
	if(playing) return;

	ofLog()<<"Play file !";

	

	if(sound_file!=NULL && sound_file->isLoaded()){
		if(last_record==cur_id){
			sound_file->seekTo(0);
			playing=true;
			read_record=0;
			return;
		}else{
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


	if(recording) return;

	//sfx
	//for(int j=0;j<1;++j){
		if(sound_fx[0].isLoaded()){
			
			ofSoundBuffer fxbuf;
			int fx_read=sound_fx[0].readTo(fxbuf,buffer_size);
			if(fx_read==0) sound_fx[0].setPlay(false);
			
			if(sound_fx[0].isPlaying()){
				for(int i=0;i<buffer_size;++i){
					int r=ofClamp(i,0,fxbuf.size()-1);
					output[i*nchannels]=fxbuf[r]*.2*Param::val()->output_vol;
					output[i*nchannels+1]=fxbuf[r]*.2*Param::val()->output_vol;
				}
			}
		}
	//}
	

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
	
	for(int k=0;k<buffer_size;++k){
		output[k*nchannels]*=Param::val()->output_vol;
		output[k*nchannels+1]*=Param::val()->output_vol;
	}
}
//-----------------------------------------------------------------------
// mode
//-----------------------------------------------------------------------
void ofApp::closeMode(MODE next_mode_){
	
	ofLog()<<mode<<"->"<<next_mode_;

	back_seq.setPause(true);
	
	stopRecord();
	stopPlayRecord();

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
		case DINO:
			dino_seq.setPause(true);
			jump_timer.stop();
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
			qrcode_timer.restart();
			break;
		case STORED:
			glow_timer.restart();
			writeSerial(ofToString(255));
			//back_seq.setIndex(697);
			break;
		case PLAY:
			break;
		case DINO:
			jump_timer.reset();
			dino_seq.reset();
			dino_seq.start();
			
			dino_dead=false;			
			for(int i=0;i<2;++i) cat_timer[i].reset();

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
				if(record_once) go_timer.restart();
				else hint_timer.restart();

				//sound_fx[1].setPlay(true);
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
			case DINO:
				for(int i=0;i<2;++i) cat_timer[i].restart();
				break;
		}
	}
	
}

void ofApp::onHintTimerEnd(int &num){
	go_timer.restart();
	hint_timer.stop();
}
void ofApp::onGoTimerEnd(int &num){
	
	closeMode(MODE::REC);
	go_timer.stop();
}
void ofApp::onCountTimerEnd(int &num){

	ofLog()<<"count "<<num;

	if(num==10){
		count_timer.stop();
		stopRecord();
		blink_timer.restart();
		//sound_fx[3].setPlay(true);
	}
}
void ofApp::onBlinkTimerEnd(int &num){
	ofLog()<<"blink "<<num;
	if(num==3){
		if(!record_once) closeMode(MODE::FINISH);
		else closeMode(MODE::QRCODE);
	}else{
	//	sound_fx[2].setPlay(true);
	}
}


void ofApp::resetSleep(){
	
	if(mode!=MODE::SLEEP) closeMode(MODE::SLEEP);
	
	stopRecord();
	stopPlayRecord();

	//rec_timer.reset();
	//finish_timer.reset();
	//qrcode_timer.reset();
}

void ofApp::exit(){
	
	if(sound_file){
		sound_file->close();
		delete sound_file;
	}
	for(int i=0;i<4;++i){
		sound_fx[i].close();
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
	return ofGetTimestampString()+"_m"+Param::val()->machine_id+"_";
}





