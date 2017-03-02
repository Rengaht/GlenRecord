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

//	int machine_id;
	string last_record;
	string upload_url;
	string share_url;
	
	int osc_port;
	float sound_scale;
	
	float output_vol;
	float screen_offsetx;
	
	string machine_id;
	
	int sleep_time;

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
		sleep_time=_param.getValue("SLEEP_TIME",120000);

//		machine_id=_param.getValue("MACHINE_ID",0);
		last_record=_param.getValue("LAST_RECORD","");	
		upload_url=_param.getValue("UPLOAD_URL","");
		share_url=_param.getValue("SHARE_URL","");
		
		osc_port=_param.getValue("OSC_PORT",12000);
		sound_scale=_param.getValue("SOUND_SCALE",1.0);
		
		output_vol=_param.getValue("OUTPUT_VOL",0.5);
		screen_offsetx=_param.getValue("SCREEN_OFFSETX",0);

		machine_id=_param.getValue("MACHINE_ID","");
		readIpData();	


		if(!file_exist) saveParam();
	}
	void readIpData(){
		
		ifstream fin;
		fin.open(ofToDataPath("ip.data").c_str());
		vector<string> data;
		while(fin!=NULL){
			string str;
			getline(fin,str);
			data.push_back(str);
		}

		for(auto s:data){
			int begin=s.find("inet addr:");
			int end=s.find("Bcast:");
			if(begin!=std::string::npos && end!=std::string::npos){
				string tmp=s.substr(begin+11,end-begin-13);
				ofLog()<<"Get Machine IP: "<<tmp;
				int last=tmp.find_last_of(".");
				machine_id=tmp.substr(last+1);
				ofLog()<<"Machine ID=_m"<<machine_id<<"_";
			}
		}

	}

	void saveParam(){
		ofxXmlSettings _xml;
		_xml.setValue("REC_TIME",rec_time);
		_xml.setValue("COUNT_TIME",count_time);
		_xml.setValue("FINISH_TIME",finish_time);
		_xml.setValue("QRCODE_TIME",qrcode_time);
		_xml.setValue("SLEEP_TIME",sleep_time);

		_xml.setValue("LAST_RECORD",last_record);
		_xml.setValue("UPLOAD_URL",upload_url);
		_xml.setValue("SHARE_URL",share_url);	
		_xml.setValue("OSC_PORT",osc_port);
		
		_xml.setValue("SOUND_SCALE",sound_scale);
		_xml.setValue("OUTPUT_VOL",output_vol);
		_xml.setValue("SCREEN_OFFSETX",screen_offsetx);
		_xml.setValue("MACHINE_ID",machine_id);
		
		_xml.save(FileName);

		ofLog()<<"Param file saved!";
	}
	void saveLastRecord(string last_){
		last_record=last_;
		saveParam();
	}

};







#endif
