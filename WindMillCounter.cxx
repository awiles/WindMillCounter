#include "WindMillCounter.h"

#define IGTCLIENT 1
#define TRACKING_BUFFER_SIZE 5

WindMillCounter::WindMillCounter()
{
	// server defaults.
	this->m_serverIP = "localhost"; // e.g. this computer.
	this->m_serverPort = 18944;		// default port.
	// instantiate socket.
#if IGTCLIENT
	this->m_socket = igtl::ClientSocket::New();
#else
	this->m_serverSocket = igtl::ServerSocket::New();
	this->m_socket = 0;
#endif
	this->m_headerMsg = igtl::MessageHeader::New();
	this->m_timestamp = igtl::TimeStamp::New();	
	this->m_positionMessage = igtl::PositionMessage::New();
	
	this->m_angularSpeed = 0.0;
	this->m_maxAngularSpeed = 0.0;
}

WindMillCounter::~WindMillCounter()
{
	//this->m_socket->CloseSocket();
}

bool WindMillCounter::init()
{
	cout << "Hello Wind Mill Experimenters!" << endl;
	// set-up the window.
	namedWindow("Windmill Counter", CV_WINDOW_NORMAL);

	// initialize the socket.
#if IGTCLIENT
	int r = this->m_socket->ConnectToServer(this->m_serverIP.c_str(), this->m_serverPort);
	if( r != 0 )
	{
		cerr << "Cannot connect to the server -- IP: " << this->m_serverIP
			<< ", Port: " << this->m_serverPort << endl;
		return false;
	}
#else
	// create the server socket.
	int r = this->m_serverSocket->CreateServer(this->m_serverPort);
	if( r != 0 )
	{
		cerr << "Cannot create server socket at port " << this->m_serverPort << endl;
		return false;
	}
	// create the client socket.
	this->m_socket = this->m_serverSocket->WaitForConnection(30000); // wait for 30 seconds.
	if( !this->m_socket.IsNotNull() ) 
	{
		this->printError("Failed to initialize the client socket.");
		return false;
	}
#endif

	this->m_trackingFrames.clear();
	this->m_angularSpeed = 0.0;
	this->m_maxAngularSpeed = 0.0;

	return true;
}

bool WindMillCounter::updateFrame()
{
	bool bMessageSuccess;
	// get IGTLink message.
	bMessageSuccess = this->getNewIGTLinkMessage();

	// update the output image.
	this->buildOutFrame();
	// show image.
	imshow("Windmill Counter", this->m_outFrame);

	return true;
}

bool WindMillCounter::onKeyPress(int keyPress)
{
	bool bSuccess = true;
	switch((char)keyPress)
	{
	case 'r': // reset the max speed.
		cout << "r key pressed, reset max angular speed." << endl;
		this->m_maxAngularSpeed = 0.0;
		break;
	case 27: // 'esc' key
		cout << "esc key pressed, exiting..." << endl;
		bSuccess = false;
		break;
	default:
		cout << "Unrecognized key pressed: " << keyPress << endl;
		//bUnknown = true;
		break;
	}

	return bSuccess;
}

bool WindMillCounter::buildOutFrame()
{
	// helper variables.
	int fontFace = CV_FONT_HERSHEY_SIMPLEX;
	double fontScale = 2.5;
	int thickness = 3;
	// initialize black image.
	Mat img(600, 800, CV_8UC3, Scalar::all(0));

	stringstream streamAS, streamMAS;
	
	// build string -- angular speed.
	streamAS << "Speed: " << (int)this->m_angularSpeed << " RPM";
	string textAS = streamAS.str();

	// build string -- max angular speed.
	streamMAS << "Max:   " << (int)this->m_maxAngularSpeed << " RPM";
	string textMAS = streamMAS.str();

	// get text size -- angular speed.
	int baseline = 0;
	Size textSizeAS = getTextSize(textAS, fontFace, fontScale, thickness, &baseline);

	// get text size -- max angular speed.
	baseline = 0;
	Size textSizeMAS = getTextSize(textMAS, fontFace, fontScale, thickness, &baseline);

	// center the text -- angular speed.
	Point textOrgAS(
		(img.cols - textSizeAS.width)/2,
		(img.rows + textSizeAS.height)/2);

	// center the text near bottom -- max angular speed.
	Point textOrgMAS(
		(img.cols - textSizeMAS.width)/2,
		(img.rows + textSizeMAS.height)*3/4);

	// put text to image -- angular speed.
	putText(img, textAS, textOrgAS, fontFace, fontScale, Scalar(0,0,255), thickness, 8);

	// put text to image -- max angular speed.
	putText(img, textMAS, textOrgMAS, fontFace, fontScale, Scalar(0,255,0), thickness, 8);

	img.copyTo(this->m_outFrame);

	return true;
}

bool WindMillCounter::getNewIGTLinkMessage()
{
	if( this->m_socket.IsNotNull() ) 
	{
		// initialize receive buffer.
		this->m_headerMsg->InitPack();
		// receive the generic header from the socket.
		int r = this->m_socket->Receive(this->m_headerMsg->GetPackPointer(), 
			this->m_headerMsg->GetPackSize());

		if( r == 0 )
		{
			this->printError("Unable to receive message from socket.");
			return false;
		}
		if( r != this->m_headerMsg->GetPackSize() )
		{
			this->printError("Header Pack Size is wrong.");
			return false;
		}

		// deserialize the header.
		this->m_headerMsg->Unpack();

		// get the timestamp.
		this->m_headerMsg->GetTimeStamp(this->m_timestamp);
		igtlUint64 frameNumber = this->m_timestamp->GetTimeStampUint64();

		// get the tool name.
		string toolName = this->m_headerMsg->GetDeviceName();

		// check the data type:
		if( strcmp(this->m_headerMsg->GetDeviceType(), "POSITION") == 0 )
		{			
			this->m_positionMessage->SetMessageHeader(this->m_headerMsg);
			this->m_positionMessage->AllocatePack();
			this->m_socket->Receive(this->m_positionMessage->GetPackBodyPointer(),
				this->m_positionMessage->GetPackBodySize());
			int c = this->m_positionMessage->Unpack(1);

			// if the CRC check is okay
			if( c & igtl::MessageHeader::UNPACK_BODY )
			{
				// get the message.
				float position[3];
				float quaternion[4];

				this->m_positionMessage->GetPosition(position);
				this->m_positionMessage->GetQuaternion(quaternion);

				this->addTrackingFrame(frameNumber, toolName, position, quaternion);
			}
			else
			{
				this->printError("CRC check failed");
				return false;
			}

		}

	}
	else
	{
		this->printError("Client Not Connected.");
		return false;
	}

	return true;
}



bool WindMillCounter::addTrackingFrame(igtlUint64 frame, string name, float *pos, float *quat)
{
	trackingFrame tf;
	tf.frame = frame;
	tf.name = name;
	tf.pos.x() = pos[0];
	tf.pos.y() = pos[1];
	tf.pos.z() = pos[2];
	tf.quat.w() = quat[3]; // openIGTLink puts the scalar last.
	tf.quat.x() = quat[0];
	tf.quat.y() = quat[1];
	tf.quat.z() = quat[2];

	this->printFrameInfo(tf);
	this->m_trackingFrames.push_back(tf);

	// check to see if we need to shorten the buffer.
	if( this->m_trackingFrames.size() > TRACKING_BUFFER_SIZE )
		this->m_trackingFrames.pop_front();

	// compute angular velocity -- http://lost-found-wandering.blogspot.ca/2011/09/revisiting-angular-velocity-from-two.html.
	trackingFrame f = this->m_trackingFrames.front();
	trackingFrame b = this->m_trackingFrames.back();

	double delta_t = (1/60.0) * (b.frame - f.frame);
	if( delta_t > 0.0)
	{

	Quaternionf r = b.quat * f.quat.inverse();
	double theta = 2 * acos(r.w());
	this->m_angularSpeed = (60/TWO_PI)*(theta) / delta_t;
	// Note: The formula gives rad/s
	//       Multiplying by 60 sec/min & 1 rot/TWO_PI gives RPM.

	if( this->m_angularSpeed > this->m_maxAngularSpeed)
		this->m_maxAngularSpeed = this->m_angularSpeed;
	}
	else
	{
		this->printError("delta_t not greater than 0.");
	}
	
	return true;
}

bool WindMillCounter::printFrameInfo(trackingFrame tf)
{
	cout << "Frame: " << tf.frame << " Tool: " << tf.name << endl
		<< "Position:" << endl << tf.pos << endl
		<< "Quaternion:" << endl << tf.quat.w() << endl << tf.quat.vec() << endl; 
	return true;
}

