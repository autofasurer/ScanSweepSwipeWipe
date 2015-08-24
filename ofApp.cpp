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

    //animation
    throb = sin(ofGetElapsedTimef())* 2.;

    //3DCamera init
    distance = 960;
    distanceGoal = 960;
    slice = 5.0;

    //color
    fader = 0.;
    color = 255;
    brightness = 1. ;
    contrast = 5;
    saturation = 0.8;
    brcosaShader.load( "shaders/brcosa_GLSL");

    //VideoCam init & FBO setup
    camWidth 		= 320;	// try to grab at this size.
	camHeight 		= 240;
    comboFBO.allocate(960, 240);
    videoTexture0.allocate(camWidth,camHeight, GL_RGBA);
    videoTexture1.allocate(camWidth,camHeight, GL_RGBA);
    videoTexture2.allocate(camWidth,camHeight, GL_RGBA);
	
    colorImg1.allocate(960, 240);
    //colorImg2.allocate(640, 480);
    //colorImg3.allocate(640, 480);
    grayImage1.allocate(960, 240);
    //grayImage2.allocate(640, 480);
    //grayImage3.allocate(640, 480);
    grayDiff1.allocate(960, 240);
    //grayDiff2.allocate(640, 480);
    //grayDiff3.allocate(640, 480);
    
    //colorImg1.getTextureReference();
    
    thread.startThread(true);
    float w = comboFBO.getWidth();
    float h = comboFBO.getHeight();
    pixelsNow.allocate(960, 240, 3);
    pixelsNow.clear();
    pixelsGoal.allocate(960,240, 3);
    pixelsGoal.clear();
    pixelsTemp.allocate(960,240, 3);
    pixelsTemp.clear();
    tex0.allocate(960, 240, 3);
    //comboFBO.readToPixels(pixels);

    //Phidget init
    //rotNow = rotThen = rotDif = 0.0;
    encOffset = 0;
    enc.init();
    meshDir = -1;
    scrDir =  1;

    ifKit.init();
    turning = 0;
    startTime = ofGetElapsedTimef();
    timer = ofGetElapsedTimef() - startTime;

    //model & mesh setup
    //model.loadModel("teapot.obj");
    //`model.setRotation(0, 0, 1, 0, 0);
    //mesh = model.getMesh(0);
    glClipPlane(GL_CLIP_PLANE0, eqn0);
    glClipPlane(GL_CLIP_PLANE1, eqn1);
    glClipPlane(GL_CLIP_PLANE2, eqn2);
    glClipPlane(GL_CLIP_PLANE3, eqn3);

    //sphere init
    //sphere1.set(250,20);
    sphere.set(150,70);
    sphere.mapTexCoords(0, h, w, 0);
    vertices0 = sphere.getMesh().getVertices();
    rotaVec.y = 1;
    ofSetLineWidth(2);
    
	bLearnBakground = true;
	threshold = 80;

}


//--------------------------------------------------------------
void ofApp::update(){

    //Check button and start motor

    if(!turning && ifKit.pushed){ //If the motor is not already turning and the button is pushed
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
            ifKit.pushed = false;
            turning = 0;
            cout << "TURN IT OFF!" << endl;
            index = 0;
            state = 0;
            ifKit.digiOut(index, state);
            distanceGoal = 960;
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
    //colorImg1.setFromPixels(thread.pixels0);
    //colorImg2.setFromPixels(thread.pixels1);
    //colorImg3.setFromPixels(thread.pixels2);
    videoTexture0.loadData(thread.pixels0);
    videoTexture1.loadData(thread.pixels1);
    videoTexture2.loadData(thread.pixels2);
    thread.unlock();
    
    
    /*
    grayImage1 = colorImg1;
    grayImage2 = colorImg2;
    grayImage3 = colorImg3;
    
    if (bLearnBakground == true){
        grayBg1 = grayImage1;
        grayBg2 = grayImage2;
        grayBg3 = grayImage3;	// the = sign copys the pixels from grayImage into grayBg (operator overloading)
        bLearnBakground = false;
    }
    
    // take the abs value of the difference between background and incoming and then threshold:
    grayDiff1.absDiff(grayBg1, grayImage1);
    grayDiff1.threshold(threshold);
    grayImage1 *= grayDiff1;
    
    grayDiff2.absDiff(grayBg2, grayImage2);
    grayDiff2.threshold(threshold);
    grayImage2 *= grayDiff2;
    
    grayDiff3.absDiff(grayBg3, grayImage3);
    grayDiff3.threshold(threshold);
    grayImage3 *= grayDiff3;
    
    pixels.setFromPixels(grayImage1.getPixels(), 640, 480, 1);
    videoTexture0.readToPixels(pixels);
    pixels.setFromPixels(grayImage2.getPixels(), 640, 480, 1);
    videoTexture1.readToPixels(pixels);
    pixels.setFromPixels(grayImage3.getPixels(), 640, 480, 1);
    videoTexture2.readToPixels(pixels);
    */
    videoTexture2.clear();
    comboFBO.begin();
    videoTexture0.draw( 0, 0);
    videoTexture1.draw(320, 0);
    videoTexture2.draw(640, 0);
    comboFBO.end();
   
    //comboFBO.readToPixels(pixels);
    tex0=comboFBO.getTextureReference();
    tex0.readToPixels(pixelsGoal);
    
    colorImg1.setFromPixels(pixelsGoal);
    tex0 = colorImg1.getTextureReference();
    
    grayImage1 = colorImg1;
    grayDiff1.absDiff(grayBg1, grayImage1);
    grayDiff1.threshold(threshold);
    grayImage1 *= grayDiff1;
    
    if (bLearnBakground == true){
        grayBg1 = grayImage1;
        bLearnBakground = false;
    }

    
    
    //Sphere Deform update
    vector<ofPoint> &vertices = sphere.getMesh().getVertices();
    for (int i=0; i<vertices.size(); i++) {
        ofPoint v = vertices0[i];
        v.normalize();
        float sx = sin( v.x * 5 );
        float sy = sin( v.y * 4 );
        float sz = sin( v.z * 6 );
        v.x += sy * sz * 0;
        v.y += sx * sz * 0;
        v.z += sx * sy * 0;
        v *= 100;
        vertices[i] = v;
    }
    
    //pixelsNow. = (pixelsGoal - pixelsNow);
    
    
    for (int i=0; i<vertices.size(); i++) {
        ofVec2f t = sphere.getMesh().getTexCoords()[i];
        t.x = ofClamp( t.x, 0, pixelsGoal.getWidth()-1 );
        t.y = ofClamp( t.y, 0, pixelsGoal.getHeight()-1 );
        //pixelsTemp.setColor(t.x, t.y, abs((pixelsNow.getColor(t.x, t.y).getBrightness() - pixelsGoal.getColor(t.x, t.y).getBrightness())*0.1));
        
        //pixelsNow.setColor(t.x, t.y, pixelsTemp.getColor(t.x, t.y));
        
        float br = pixelsGoal.getColor(t.x, t.y).getBrightness();
        //cout << "br: " << br << endl;
        vertices[i] *= 1 + br / 255.0 * 3.0;
    }

    eqn0[3] = eqn1[3] = slice;

    glClipPlane(GL_CLIP_PLANE0, eqn0);
    glClipPlane(GL_CLIP_PLANE1, eqn1);

    //update timer
    timer = ofGetElapsedTimef() - startTime;

    animate();
}

//--------------------------------------------------------------
void ofApp::draw(){
    tex0.draw(320, 240);
    //comboFBO.draw(0,480);
    //colorImg1.draw(320, 240);
    //grayImage.draw(0, 0, 264, 96);
    //grayImage1.draw(0, 0, 1320, 480);
    //grayImage2.draw(640,0,1280, 480);
    //colorImg1.draw(0, 240, 1260, 480);
    //colorImg3.draw(640, 0, 960,240);
    
    /** Store camera image in fbo **/

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
    else if (key == 'a'){                cout << "saturation:" << saturation << endl;

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
        distanceGoal += 10;
        cout << "Distance: " << distance << endl;
    }
    else if (key == OF_KEY_DOWN){
        distanceGoal -= 10;
        cout << "Distance: " << distance << endl;
    }
    else if (key == 'l'){
        bLearnBakground = true;
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
    //throb = sin(ofGetElapsedTimef())* 2.0;

    if (timer >= 5 && timer <= 6 ){
            distanceGoal = -100;
    }
    distance += (distanceGoal - distance) * 0.05 + throb;
    //slice = abs(sin(ofGetElapsedTimef())*10) + 1;
}

//--------------------------------------------------------------

void ofApp::exit() {
    ifKit.digiOut(0, 0);
	thread.stopThread();
}


