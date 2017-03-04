#include <iostream>
#include <opencv2/opencv.hpp>
#include "WindMillCounter.h"

using namespace cv;
using namespace std;

int main(int argc, char *argv[]){
	// some useful variables
	int keyPress =-1;
	bool bSuccess = true;
	WindMillCounter windmill;

	// main program loop.
	if(!windmill.init())
	{
		cout << "Failed to init windmill counter.  Exiting." << endl;
		return -1;
	}

	while(bSuccess)
	{
		// do something with the openIGTLink data.
		windmill.updateFrame();

		// check for key presses.
		keyPress = waitKey(30);
		
		if(keyPress >= 0 )
		{
			bSuccess = windmill.onKeyPress(keyPress);
		}
	}
   return 0;
}