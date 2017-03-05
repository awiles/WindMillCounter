#ifndef WM_WindMillCounter
#define WM_WindMillCounter

#include <iostream>
#include <iomanip>
#include <opencv2/opencv.hpp>
#include "WMObject.h"

#include <Eigen/Geometry>
#include "igtlOSUtil.h"
#include "igtlMessageHeader.h"
#include "igtlTransformMessage.h"
#include "igtlPositionMessage.h"
#include "igtlImageMessage.h"
#include "igtlServerSocket.h"
#include "igtlClientSocket.h"
#include "igtlStatusMessage.h"

using namespace cv;
using namespace std;
using namespace Eigen;

struct trackingFrame {
	igtlUint64 frame;
	string name;
	Vector3d pos;
	Quaternionf quat;
};

class WindMillCounter : public WMObject {
public:
	WindMillCounter();					// constructor.
	~WindMillCounter();					// destructor.
	virtual string const className() { return "WindMillCounter";}
	bool init();					// initialize the object.
	bool updateFrame();				// update the current frame.
	bool onKeyPress(int keyPress);	// reacts to key press.  Returns bExit.

protected:
	// output
	Mat m_outFrame;				// this is the image that will be displayed.
	// igtlink items.
	string m_serverIP;			// what is the server IP.
	int m_serverPort;
	igtl::ServerSocket::Pointer m_serverSocket;
	igtl::ClientSocket::Pointer m_socket;
	igtl::MessageHeader::Pointer m_headerMsg;
	igtl::TimeStamp::Pointer m_timestamp;
	igtl::PositionMessage::Pointer m_positionMessage;
	// tracking information.
	list<trackingFrame> m_trackingFrames;
	double m_angularSpeed;
	double m_maxAngularSpeed;
	
	// helper functions.
	bool buildOutFrame(); // create the output image.
	bool getNewIGTLinkMessage(); // get the latest message.
	bool addTrackingFrame(igtlUint64 frame, string name, float *pos, float *quat);
	bool printFrameInfo(trackingFrame tf);
};
#endif