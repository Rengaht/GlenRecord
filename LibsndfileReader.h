#ifndef LIBSNDFILE_READER
#define LIBSNDFILE_READER

#include "sndfile.hh"
#include "ofUtils.h"

class LibsndfileReader{

public:
	LibsndfileReader(){
		initialized=false;
		_filename="";
	}

	void setup(string file_){
	
		if(initialized){
			if(file_!= _filename){
				close();
				openFile(file_);
			}else{
				_sndfile->seek(0,SEEK_SET);
			}
		}else{
			openFile(file_);
		}
		
	}
	void openFile(string file_){
		cout<<"------------ Load file ------------"<<endl;
		SndfileHandle *snd_=new SndfileHandle(ofToDataPath(file_,true));
		if(snd_){
			_sndfile=snd_;
		
			cout<<"Open file:"<<file_<<endl
		    		<<"Sampel Rate= "<<_sndfile->samplerate()<<endl
		   	 	<<"Channels= "<<_sndfile->channels()<<endl
		    		<<"Error= "<<_sndfile->strError()<<endl
		    		<<"Frames= "<<_sndfile->frames()<<endl;
		
			_filename=file_;
		
			initialized=true;
		}else initialized=false;

	}
	int read(float *buffer_, int nsample){
		//float buffer_[nsample];
		if(_sndfile) return _sndfile->read(buffer_,nsample);
		return 0;
		//return buffer_;
	}
	void close(){
		//if(!initialized) return;
			
		cout<<"close file!"<<endl;
		
		if(_sndfile){
			sf_close(_sndfile->takeOwnership());
			delete _sndfile;
			_sndfile=NULL;
		}
		initialized=false;
	}


private:
	SndfileHandle *_sndfile;
	string _filename;
	bool initialized;
};




#endif
