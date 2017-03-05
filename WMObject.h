#ifndef WM_WMObject
#define WM_WMObject

#ifndef PI
#define PI      (double)3.1415926535897932384626433832795
#endif
#define TWO_PI  (double)6.2831853071795864769252867665590

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