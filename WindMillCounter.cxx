#include "WindMillCounter.h"

#define IGTCLIENT 1

using namespace cv;
using namespace std;

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
	return true;
}

bool WindMillCounter::updateFrame()
{
	bool bMessageSuccess;
	// get IGTLink message.
	bMessageSuccess = this->getNewIGTLinkMessage();

	// update the output image.
	this->buildOutFrame(3.1456788);
	// show image.
	imshow("Windmill Counter", this->m_outFrame);

	return true;
}

bool WindMillCounter::onKeyPress(int keyPress)
{
	bool bSuccess = true;
	switch((char)keyPress)
	{
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

bool WindMillCounter::buildOutFrame(float rpm)
{
	// build string.
	stringstream stream;
	stream << fixed << setprecision(1) << rpm << " rpm";
	string text = stream.str();

	// helper variables.
	int fontFace = CV_FONT_HERSHEY_SIMPLEX;
	double fontScale = 5;
	int thickness = 3;

	// initialize black image.
	Mat img(600, 800, CV_8UC3, Scalar::all(0));

	// get text size.
	int baseline = 0;
	Size textSize = getTextSize(text, fontFace, fontScale, thickness, &baseline);

	// center the text.
	Point textOrg(
		(img.cols - textSize.width)/2,
		(img.rows + textSize.height)/2);

	// put text in middle.
	putText(img, text, textOrg, fontFace, fontScale, Scalar(0,0,255), thickness, 8);

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

				this->printFrameInfo(frameNumber, toolName, position, quaternion);
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

bool WindMillCounter::printFrameInfo(igtlUint64 frame, string name, float *pos, float *quat)
{
	cout << "Frame: " << frame << " Tool: " << name << " -- Position: (" << pos[0] << ", " << pos[1] << ", " << pos[2] 
	<< ") Quaternion: (" << quat[3] << ", " << quat[0] << ", " << quat[1] << ", " << quat[2] << ")" << endl; 
	return true;
}

