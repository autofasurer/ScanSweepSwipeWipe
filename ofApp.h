#pragma once

#include "ofMain.h"
#include "ofxAssimpModelLoader.h"
#include "ofxPhidgetsEnc.h"
#include "camThread.h"
#include "ofxOpenCv.h"

class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();
		void animate();
        void exit();
        void keyPressed(int key);
		void gotMessage(ofMessage msg);

	//the camera
    ofCamera            modelCam, maskCam;
    ofNode              globalRotation;

    //primitives
    ofSpherePrimitive   sphere;
    ofSpherePrimitive   sphere1, center;

    //model loaded from disk
    ofxAssimpModelLoader model;

    //mesh which will contain the model's data
    ofMesh              mesh;

    //misc variables
    float               y, distance, distanceGoal, slice, color, gap, gapOffset, throb, totalBrightness, refBrightness, brightMean;
    float               rotNow, rotThen, mRota;
    int                 encOffset;
    bool                showModel, showPlane, trigger;

    //Phidgets
    ofxPhidgetsEncoder  enc;
    ofxPhidgetsIfkit    ifKit;
    int index, state, turning;

    //Texture
    ofTexture           tex0, tex1;

    //Video Camera & cam thread
    ofTexture			videoTexture0, videoTexture1, videoTexture2;
    int 				camWidth;
    int 				camHeight;
    int                 panoWidth, panoHeight;
    ofFbo               comboFBO, blobFBO;
    MyThread            thread;
    ofPixels            pixelsNow;
    ofPixels            pixelsGoal;
    ofPixels            pixelsTemp;
    ofPixels            pixelsFader;
    //FBO
    //ofFbo fbo, fboBlobs;

    vector<ofPoint> vertices;
    vector<ofPoint> vertices0;
    vector<ofPoint> verticesTemp;
    vector<ofPoint> verticesGoal;
    vector<ofPoint> verticesNow;
    float fader, brightness, contrast, saturation, x1, x2, y1, y2, z1, z2, dist1, dist2, scrDir, meshDir, startTime, timer;
    ofVec3f rotaVec;

    //Shader
    ofShader            brcosaShader;

    //Clip plane equations
    double eqn0[4] = {0., 0.1, -0.1, -23.};
    double eqn1[4] = {0., -0.1, 0.1, 23.};
    //double eqn2[4] = {0.0, 1.0, 0.0, 0.0};
    //double eqn3[4] = {0.0, 0.0, 0.0, 0.0};

    ofxCvColorImage			colorImg1;
    ofxCvGrayscaleImage 	grayImage1, grayImage2, grayImageTemp, grayBg1, grayDiff1;
    ofxCvContourFinder      contourfinder;
    int 				threshold, imgFader;
    bool				bLearnBakground;



};
