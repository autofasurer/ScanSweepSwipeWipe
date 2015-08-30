#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){

    //window init
    ofSetWindowTitle("ScanSweep");
    ofSetWindowShape(800, 600);
    ofSetFrameRate(60);
    ofSetVerticalSync(true);
    ofBackground(ofColor::black);
    ofHideCursor();
    ofSetFullscreen(true);
    ofEnableDepthTest();

    refBrightness = 0;
    brightMean = 1;
    //animation
    //throb = sin(ofGetElapsedTimef())* 2.;
	imgFader = 5;
    //3DCamera init
    distance = 370;
    distanceGoal = 960;
    slice = 5.0;
    panoWidth = 960;
    panoHeight = 240;
    //color
    fader = 0.;
    color = 255;
    brightness = 1.5 ;
    contrast = 3.5;
    saturation = -4.5;
    brcosaShader.load( "shaders/brcosa_GLSL");

    //VideoCam init & FBO setup
    camWidth 		= 320;	// try to grab at this size.
	camHeight 		= 240;
    comboFBO.allocate(panoWidth, panoHeight);
    blobFBO.allocate(panoWidth, panoHeight);
    videoTexture0.allocate(camWidth,camHeight, GL_RGB);
    videoTexture1.allocate(camWidth,camHeight, GL_RGB);
    videoTexture2.allocate(camWidth,camHeight, GL_RGB);

    colorImg1.allocate(panoWidth, panoHeight);
    grayImage1.allocate(panoWidth, panoHeight);
    grayBg1.allocate(panoWidth, panoHeight);
    grayDiff1.allocate(panoWidth, panoHeight);
    thread.startThread(true);
    float w = comboFBO.getWidth();
    float h = comboFBO.getHeight();
    pixelsNow.allocate(panoWidth, panoHeight, 3);
    pixelsNow.clear();
    pixelsGoal.allocate(panoWidth, panoHeight, 3);
    pixelsGoal.clear();
    pixelsTemp.allocate(panoWidth, panoHeight, 3);
    pixelsTemp.clear();
	pixelsFader.allocate(panoWidth, panoHeight, 1);
	pixelsFader.set(imgFader);
    tex0.allocate(panoWidth, panoHeight, 3);
    tex1.allocate(panoWidth, panoHeight, 3);

    //Phidget init
    //rotNow = rotThen = rotDif = 0.0;
    encOffset = 0;
    enc.init();
    meshDir = -1;
    scrDir =  1;

    ifKit.init();
    turning = 0;
    trigger = 0;
    startTime = ofGetElapsedTimef();
    timer = ofGetElapsedTimef() - startTime;

    //model & mesh setup
    //model.loadModel("teapot.obj");
    //`model.setRotation(0, 0, 1, 0, 0);
    //mesh = model.getMesh(0);
    glClipPlane(GL_CLIP_PLANE0, eqn0);
    glClipPlane(GL_CLIP_PLANE1, eqn1);
    //glClipPlane(GL_CLIP_PLANE2, eqn2);
    //glClipPlane(GL_CLIP_PLANE3, eqn3);

    //sphere init
    //sphere1.set(250,20);
    sphere.set(200,100);
    sphere.mapTexCoords(0, h, w, 0);
    vertices0 = sphere.getMesh().getVertices();
    rotaVec.y = 1;
    ofSetLineWidth(1);
	bLearnBakground = true;
	threshold = 80;
	verticesTemp = sphere.getMesh().getVertices();
	verticesGoal = sphere.getMesh().getVertices();
	verticesNow = sphere.getMesh().getVertices();
	cout << verticesNow.size() << endl;
	for (int i=0; i<verticesNow.size(); i++) {
		verticesNow[i] *= 0;
	}
}


//--------------------------------------------------------------
void ofApp::update(){

    //Check button and start motor

    //if(!turning && ifKit.pushed){ //If the motor is not already turning and the button is pushed
    if(!turning && trigger && timer > 30){
        turning = 1;
        startTime = ofGetElapsedTimef();
        timer = ofGetElapsedTimef() - startTime;
        index = 0;
        if (!state){
            state = 1;
        }
        ifKit.digiOut(index, state);
    }
    if (turning){
        if(timer > 40){
            //ifKit.pushed = false;
            turning = 0;
            cout << "TURN IT OFF!" << endl;
            index = 0;
            state = 0;
            trigger = 0;
            ifKit.digiOut(index, state);
            startTime = ofGetElapsedTimef();
            timer = ofGetElapsedTimef() - startTime;
            //distanceGoal = 960;
        }
        //else{
            //cout  << "time passed since buttong push: " << timer << endl;
        //}
    }

    //cout << timer;
    rotThen = mRota;
    enc.update();
    //cout << "encoder :" << enc.encPos << endl;
    mRota = ((enc.encPos - encOffset) % 8000) / 22.2222;
    mRota = ((enc.encPos - enc.indexPos) % 8000) / 22.2222;
    encOffset = enc.indexPos;
    rotNow = ( rotThen - mRota );

    //cout << mRota << endl;
    //cout << model.getRotationAngle(1) << endl;

    thread.lock();
    videoTexture0.loadData(thread.pixels0);
    videoTexture1.loadData(thread.pixels1);
    videoTexture2.loadData(thread.pixels2);
    thread.unlock();

    //Combine the three camera images into 1 FBO
    comboFBO.begin();
    videoTexture0.draw( 0, 0);
    videoTexture1.draw(320, 0);
    videoTexture2.draw(640, 0);
    comboFBO.end();

    tex0=comboFBO.getTextureReference(); //put FBO containing the combined cam textures into an ofTexture...
    tex0.readToPixels(pixelsGoal); //...and copy the pixels from the texture into an ofPixels object...
    pixelsGoal.setImageType(OF_IMAGE_COLOR); //...and set the color to RGB, discarding the Alpha channel...
    colorImg1.setFromPixels(pixelsGoal); //...finally copy the pixels into an OpenCV color image

    grayImage1 = colorImg1;
    if (bLearnBakground == true){
        grayBg1 = grayImage1; // the = sign copys the pixels from grayImage into grayBg (operator overloading)
        bLearnBakground = false;
    }

    // take the abs value of the difference between background and incoming and then threshold:
    grayDiff1.absDiff(grayBg1, grayImage1);
    grayDiff1.threshold(threshold);
    contourfinder.findContours(grayDiff1, 20, (panoWidth*panoHeight)/3, 10, true);
    blobFBO.begin();
    ofClear(0,0,0);
    contourfinder.draw(0,0);
    blobFBO.end();

    tex1=blobFBO.getTextureReference();
    tex1.readToPixels(pixelsGoal);
    pixelsGoal.setImageType(OF_IMAGE_GRAYSCALE);
    grayDiff1.setFromPixels(pixelsGoal);

	grayImage2.setFromPixels(pixelsFader);

	grayImage1 *= grayDiff1;
	grayImage1 += grayImageTemp;

	grayImageTemp = grayImage1;
	grayImageTemp -= grayImage2;

	pixelsGoal.setFromPixels(grayImage1.getPixels(), panoWidth, panoHeight, 1);

    //Sphere Deform update
    vector<ofPoint> &vertices = sphere.getMesh().getVertices();
    for (int i=0; i<vertices.size(); i++) {
        ofPoint v = vertices0[i];
        v.normalize();
        float sx = sin( v.x * 5 );
        float sy = sin( v.y * 4 );
        float sz = sin( v.z * 6 );
        v.x += sy * sz * (sin(ofGetElapsedTimef())*0.1);
        v.y += sx * sz * (cos(ofGetElapsedTimef())*0.1);
        v.z += sx * sy * (sin(ofGetElapsedTimef())*0.1);
        v *= 150;
        vertices[i] = v;
		vertices[i] *= verticesGoal[i];
    }
        totalBrightness = 0;

	for (int i=0; i<vertices.size(); i++) {

		verticesGoal[i] *= 0;
		verticesGoal[i] += 1;
		ofVec2f t = sphere.getMesh().getTexCoords()[i];
        t.x = ofClamp( t.x, 0, pixelsGoal.getWidth()-1 );
        t.y = ofClamp( t.y, 0, pixelsGoal.getHeight()-1 );
        float br = pixelsGoal.getColor(t.x, t.y).getBrightness();

		verticesGoal[i] *= 1 + br / 255.0 * 1.5;
        totalBrightness += br;
    }

        if (totalBrightness > 200000){
        trigger = 1;
        bLearnBakground = true;
        totalBrightness = 0;
    }

    eqn0[3] = eqn1[3] = slice;

    glClipPlane(GL_CLIP_PLANE0, eqn0);
    glClipPlane(GL_CLIP_PLANE1, eqn1);

    //update timer
    timer = ofGetElapsedTimef() - startTime;
    //cout << "timer: " << timer << endl;
    //animate();
}

//--------------------------------------------------------------
void ofApp::draw(){

    //comboFBO.draw(0,480);
    //blobFBO.draw(0,0);
    //grayImageTemp.draw(0, 0);
    //grayBg1.draw(0, 240);
    //grayDiff1.draw(0, 480);
    //colorImg1.draw(0, 720);

    //BRCOSA

brcosaShader.begin();

    brcosaShader.setUniformTexture("tex0", tex0, 0);
    brcosaShader.setUniform1f("contrast", contrast);
    brcosaShader.setUniform1f("brightness", brightness);
    brcosaShader.setUniform1f("saturation", saturation);
    brcosaShader.setUniform1f("alpha", 1.0);
    //
    ofTranslate(ofGetWidth()/2, ofGetHeight()/2, distance);

    //rotate the entire view so we 'look from above'
    //ofRotate(90, 1, 0, 0);

        glEnable(GL_CLIP_PLANE0);
        glEnable(GL_CLIP_PLANE1);
         ofRotate(mRota * scrDir, 0, 0, 1);
            ofEnableDepthTest();
                    //model.setRotation(1, -mRota, 0, 1, 0);
                    //model.drawFaces();
                    sphere.rotate(rotNow * meshDir, 0, 1, 0);
                    sphere.drawWireframe();
                    //sphere.drawVertices();
            ofDisableDepthTest();
        glDisable(GL_CLIP_PLANE1);
        glDisable(GL_CLIP_PLANE0);
brcosaShader.end();


}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

    if (key == ' '){
        ofToggleFullscreen();
    }
    else if (key == 'r'){
        slice += 0.1;
    }
    else if (key == 'f'){
        slice -= 0.1;
    }
    else if (key == 't'){
        scrDir *= -1;
        cout << "srcDir:" << scrDir << endl;
    }
    else if (key == 'g'){
        meshDir *= -1;
        cout << "meshDir:" << meshDir << endl;
    }
     //BrCoSa
    else if (key == 'q'){
        brightness += 0.1;
          cout << "brightness:" << brightness << endl;
    }
    else if (key == 'a'){
        brightness -= 0.1;
        cout << "brightness:" << brightness << endl;
    }
    else if (key =='w'){
        contrast += 0.1;
        cout << "contrast:" << contrast << endl;
    }
    else if (key == 's'){
        contrast -= 0.1;
        cout << "contrast:" << contrast << endl;
    }
    else if (key == 'e'){
        saturation += 0.1;
        cout << "saturation:" << saturation << endl;
    }
    else if (key == 'd'){
        saturation -= 0.1;
        cout << "saturation:" << saturation << endl;
    }
    //reset encoder position
    else if (key == 'o'){
        encOffset = enc.encPos;
    }
    else if (key == OF_KEY_UP){
		distance += 10;
		cout << "Distance: " << distance << endl;
    }
    else if (key == OF_KEY_DOWN){
		distance -= 10;
		cout << "Distance: " << distance << endl;
    }
    else if (key == 'l'){
		bLearnBakground = true;
		verticesTemp = sphere.getMesh().getVertices();
		verticesGoal = sphere.getMesh().getVertices();
    }
    else if(key =='+'){
    threshold ++;
    if (threshold > 255) threshold = 255;
    }
    else if(key =='-'){
    threshold --;
    if (threshold < 0) threshold = 0;
    }

}


//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::animate(){
    /*throb = sin(ofGetElapsedTimef())* 2.0;

    if (timer >= 5 && timer <= 6 ){
            distanceGoal = -100;
    }
    distance += (distanceGoal - distance) * 0.05 + throb;
    //slice = abs(sin(ofGetElapsedTimef())*10) + 1;*/
}

//--------------------------------------------------------------

void ofApp::exit() {
    ifKit.digiOut(0, 0);
	thread.stopThread();
}


