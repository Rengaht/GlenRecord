#include "ofMain.h"
#include "ofApp.h"

//========================================================================
string Param::FileName="_param.xml";
Param *Param::instance=0;

int main( ){

	ofSetupOpenGL(480,800,OF_WINDOW);			// <-------- setup the GL context

	// this kicks off the running of my app
	// can be OF_WINDOW or OF_FULLSCREEN
	// pass in width and height too:
	ofRunApp( new ofApp());

}
