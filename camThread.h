//
//  camThread.h
//  videoGrabberExample
//
//  Created by Brecht Debackere on 30/06/15.
//
//
#pragma once

class MyThread : public ofThread {

public:

	ofVideoGrabber cam0, cam1, cam2;
	ofPixels pixels0, pixels1, pixels2;

	MyThread() {
        cam0.setDeviceID(0);
		cam0.initGrabber(320,240,false);
        cam1.setDeviceID(1);
        cam1.initGrabber(320,240,false);
        cam2.setDeviceID(2);
        cam2.initGrabber(320,240,false);
        pixels0.setImageType(OF_IMAGE_COLOR);
        pixels1.setImageType(OF_IMAGE_COLOR);
        pixels2.setImageType(OF_IMAGE_COLOR);
	}

	void threadedFunction() {
		while(isThreadRunning()) {
			cam0.update();
			if(cam0.isFrameNew()) {
				lock();
				pixels0 = cam0.getPixelsRef();
				unlock();
			}
            cam1.update();
			if(cam1.isFrameNew()) {
				lock();
				pixels1 = cam1.getPixelsRef();
				unlock();
			}
            cam2.update();
			if(cam2.isFrameNew()) {
				lock();
				pixels2 = cam2.getPixelsRef();
				unlock();
			}
		}
	}
};

