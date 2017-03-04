#ifndef WM_WMObject
#define WM_WMObject

#include <iostream>
#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;

//this is a top level class with some simple utilities.
class WMObject {
public:
	WMObject();
	void turnOnDebug();
	void turnOffDebug();
	void toggleDebug();

protected:
	// functions
	virtual string const className() = 0;
	void printError( const string& msg );
	void showDebugImage( const string& name, Mat image);
	
	// variables
	bool m_debug;
};

#endif