#ifndef PARAM_H
#define PARAM_H
#include "ofMain.h"
#include "ofxXmlSettings.h"


class Param{
	static Param* instance;
	
public:
	static string FileName;
	int rec_time;
	int count_time;
	int finish_time;
	int qrcode_time;

	int machine_id;
	string last_record;
	string upload_url;
	string share_url;
	
	int osc_port;
	float sound_scale;
	
	float output_vol;
	float screen_offsetx;


	Param(){
		readParam();
	}
	static Param* val(){
		if(!instance) instance=new Param();
		return instance;
	}

	void readParam(){
		ofxXmlSettings _param;
		bool file_exist=_param.loadFile(FileName);
		if(file_exist) ofLog()<<"Param file loaded!";


		rec_time=_param.getValue("REC_TIME",10);
		count_time=_param.getValue("COUNT_TIME",3);
		finish_time=_param.getValue("FINISH_TIME",5);
		qrcode_time=_param.getValue("QRCODE_TIME",120);
	

		machine_id=_param.getValue("MACHINE_ID",0);
		last_record=_param.getValue("LAST_RECORD","");	
		upload_url=_param.getValue("UPLOAD_URL","");
		share_url=_param.getValue("SHARE_URL","");
		
		osc_port=_param.getValue("OSC_PORT",12000);
		sound_scale=_param.getValue("SOUND_SCALE",1.0);
		
		output_vol=_param.getValue("OUTPUT_VOL",0.5);
		screen_offsetx=_param.getValue("SCREEN_OFFSETX",0);

		if(!file_exist) saveParam();
	}
	void saveParam(){
		ofxXmlSettings _xml;
		_xml.setValue("REC_TIME",rec_time);
		_xml.setValue("COUNT_TIME",count_time);
		_xml.setValue("FINISH_TIME",finish_time);
		_xml.setValue("QRCODE_TIME",qrcode_time);

		_xml.setValue("MACHINE_ID",machine_id);
		_xml.setValue("LAST_RECORD",last_record);
		_xml.setValue("UPLOAD_URL",upload_url);
		_xml.setValue("SHARE_URL",share_url);	
		_xml.setValue("OSC_PORT",osc_port);
		
		_xml.setValue("SOUND_SCALE",sound_scale);
		_xml.setValue("OUTPUT_VOL",output_vol);
		_xml.setValue("SCREEN_OFFSETX",screen_offsetx);
		_xml.save(FileName);

		ofLog()<<"Param file saved!";
	}
	void saveLastRecord(string last_){
		last_record=last_;
		saveParam();
	}

};







#endif
