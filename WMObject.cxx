#include "WMObject.h"

WMObject::WMObject()
{
	this->m_debug =  false;
}

void WMObject::turnOnDebug()
{
	this->m_debug = true;
}

void WMObject::turnOffDebug()
{
	this->m_debug = false;
}

void WMObject::toggleDebug()
{
	this->m_debug = this->m_debug ? false : true;
}

void WMObject::printError(const string& msg)
{
	cout << "Error in " << this->className() << ": " << msg << endl; 
}

void WMObject::showDebugImage(const string& name, Mat image)
{
	if( this->m_debug)
	{
		string winname;
		winname = this->className() + "::" + name;
		imshow( winname, image);
	}
}