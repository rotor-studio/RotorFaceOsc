#include "ofApp.h"
#include "ofxXmlPoco.h"

using namespace ofxCv;
using namespace cv;
//--------------------------------------------------------------
void ofApp::loadSettings() {
    // if you want to package the app by itself without an outer
    // folder, you can place the "data" folder inside the app in
    // the Resources folder (right click on the app, "show contents")
    //ofSetDataPathRoot("../Resources/data/");
    
    // setup gui
    bGuiVisible = true;
    gui.setup();
    gui.setName("general settings: ");
    gui.setPosition(0, 0);
    gui.add(bIncludePose.set("send pose", true));
    gui.add(bIncludeGestures.set("send gesture", true));
    gui.add(bIncludeAllVertices.set("send raw", false));
    gui.add(bNormalizeRaw.set("send normalize raw", false));
    gui.add(bIterations.set("iterations",20,1,50));
    gui.add(bClamp.set("clamp",2,0,10));
    gui.add(bTolerance.set("tolerance",0.01,0.01,0.1));
    gui.add(bAttemps.set("attemps",1,1,10));
    gui.add(enableHaarFinder.set("faces", false));
    gui.add(printValues.set("print osc", true));
    gui.add(mini.set("minimice", false));
    
    
    
    // load settings file
    ofxXmlPoco xml;
    if(!xml.load(ofToDataPath("settings.xml"))) {
        return;
    }
    
    // expects following tags to be wrapped by a main "faceosc" tag
    
    mode=0;
    
    xml.setTo("source");
    if(xml.exists("useCamera")) {
        mode = xml.getValue("useCamera", 0);
    }
    xml.setToParent();
    
    xml.setTo("camera");
    if(xml.exists("device")) {
        cam.setDeviceID(xml.getValue("device", 0));
    }
    if(xml.exists("framerate")) {
        cam.setDesiredFrameRate(xml.getValue("framerate", 30));
    }
    camWidth = xml.getValue("width", 640);
    camHeight = xml.getValue("height", 480);
    xml.setToParent();
    
    xml.setTo("movie");
    if(xml.exists("filename")) {
        string filename = ofToDataPath((string)xml.getValue("filename", ""));
        if(filename != "") {
            if(!movie.load(filename)) {
                ofLog(OF_LOG_ERROR, "Could not load movie \"%s\", reverting to camera input", filename.c_str());
            
            }
        }
    }
    else {
        ofLog(OF_LOG_ERROR, "Movie filename tag not set in settings, reverting to camera input");
        mode=0;
    }
    if(xml.exists("volume")) {
        float movieVolume = ofClamp(xml.getValue("volume", 1.0), 0, 1.0);
        movie.setVolume(movieVolume);
    }
    if(xml.exists("speed")) {
        float movieSpeed = ofClamp(xml.getValue("speed", 1.0), -16, 16);
        movie.setSpeed(movieSpeed);
    }
    bPaused = false;
    movieWidth = movie.getWidth();
    movieHeight = movie.getHeight();
    xml.setToParent();
    
    
    if(mode == 0) {
        ofSetWindowShape(1280,720);
        setVideoSource(mode);
        miniDisplace= 0;
        syphonMini=true;
        modeName = "Syphon Mode";
    }
    else if(mode == 1) {
        ofSetWindowShape(camWidth, camHeight);
        cam.initGrabber(camWidth, camHeight);
        setVideoSource(mode);
        miniDisplace= 0;
        syphonMini=false;
        modeName = "Cam Mode";
        
    }
    else if(mode == 2) {
        ofSetWindowShape(1280,720);
        setVideoSource(mode);
        miniDisplace= 0;
        syphonMini=false;
        modeName = "Video Mode";
    }
    
    xml.setTo("face");
    if(xml.exists("rescale")) {
        tracker.setRescale(xml.getValue("rescale", 1.0));
    }
    if(xml.exists("iterations")) {
        bIterations = xml.getValue("iterations", 5);
    }
    if(xml.exists("clamp")) {
        bClamp = xml.getValue("clamp", 3.0);
    }
    if(xml.exists("tolerance")) {
        bTolerance = xml.getValue("tolerance", 0.01);
    }
    if(xml.exists("attempts")) {
        bAttemps = xml.getValue("attempts", 1);
    }
    bDrawMesh = true;
    if(xml.exists("drawMesh")) {
        bDrawMesh = (bool) xml.getValue("drawMesh", 1);
    }
    tracker.setup();
    xml.setToParent();
    
    xml.setTo("oscIn");
    portIn = xml.getValue("port", 6000);
    oscIn.setup(portIn);
    xml.setToParent();
    
    xml.setTo("osc");
    host = xml.getValue("host", "localhost");
    port = xml.getValue("port", 7000);
    osc.setup(host, port);
    xml.setToParent();
    
    xml.setTo("oscFace");
    hostFace = xml.getValue("host", "localhost");
    portFace = xml.getValue("port", 8000);
    oscFace.setup(hostFace, portFace);
    xml.setToParent();
    
    
    xml.clear();
}




//--------------------------------------------------------------
void ofApp::setup(){
    
    ofSetVerticalSync(true);
    loadSettings();
    
    //Titulo
    ofSetWindowTitle("RotorFace_00 - "+modeName);
   // ofSetWindowShape(sourceWidth, sourceHeight);
    
    //Syphon
   // server.setName("rotorOSC output");
    
    if(mode==0){
        //setup our directory
        dir.setup();
        
        //register for our directory's callbacks
        ofAddListener(dir.events.serverAnnounced, this, &ofApp::serverAnnounced);
        // not yet implemented
        //ofAddListener(dir.events.serverUpdated, this, &ofApp::serverUpdated);
        ofAddListener(dir.events.serverRetired, this, &ofApp::serverRetired);
        
        dirIdx = -1;
        
        cliente.setup();
        cliente.set("","rotorOSC ext input");
        
        texIn.allocate(1280/2,720/2, GL_RGBA);
        //texIn2.allocate(1280/2,720/2, GL_RGBA);
       // texOut.allocate(1280/2, 720/2, GL_RGBA);
        
        //Salida
        textFinder.allocate(1280,720, GL_RGBA);
    
        
        serverFinder.setName("finder Output");
        
    }
    if(mode==2){
        movie.play();
    }
    
    finder.setup("facefinder/haarcascade_frontalface_default.xml");
    finder.setPreset(ObjectFinder::Fast);
    
    for(int i=0;i<3;i++){
        faces[i].load("faces/face"+ofToString(i)+".png");
    }
    showSquare=true;
    
}

//--------------------------------------------------------------
void ofApp::update(){
    
    
    if(mode == 1 || mode == 2){
        
        
        videoSource->update();
        
         if(!bPaused){
        if(videoSource->isFrameNew()) {
            tracker.update(toCv(*videoSource));
            sendFaceOsc(tracker);
            rotationMatrix = tracker.getRotationMatrix();
            
         
        }
        }
    }
    else if(mode==0){
        
    
        //Dibujamos en el FBO si hay asunto syphon
        if(dir.isValidIndex(dirIdx)){
         
            texIn.begin();
            cliente.draw(0, 0,sourceWidth/2,sourceHeight/2);
            texIn.end();
            
            texIn.readToPixels(texIn2);
            texIn2.setFromPixels(texIn2);
            texIn2.setImageType(ofImageType::OF_IMAGE_GRAYSCALE);
            
            if(!bPaused){
            tracker.update(toCv(texIn2.getPixels()));
            sendFaceOsc(tracker);
            rotationMatrix = tracker.getRotationMatrix();
            }
            
        }
    }
    if(mini){
        if(!syphonMini){
        ofSetWindowShape(200,345);
        }else{
        ofSetWindowShape(200,345);
        }
        ofBackground(0);
    }else{
        ofSetWindowShape(sourceWidth,sourceHeight);
    }
    
    //busca caras
    if(enableHaarFinder){
        finder.update(texIn2);
    }


//OSC STUFF IN y FACES

// hide old messages
for(int i = 0; i < NUM_MSG_STRINGS; i++){
    if(timers[i] < ofGetElapsedTimef()){
        msgStrings[i] = "";
    }
}

// check for waiting messages
while(oscIn.hasWaitingMessages()){
    
    // get the next message
    ofxOscMessage m;
    oscIn.getNextMessage(m);
    
    // check for mouse moved message
    if(m.getAddress() == "/face"){
        
      
        enableHaarFinder=!enableHaarFinder;
      
    }
    // check for mouse moved message
    if(m.getAddress() == "/pause"){
        bPaused =! bPaused;
    }
    // check for mouse moved message
    if(m.getAddress() == "/syphonIn"){
        if (dir.size() > 0)
        {
            dirIdx++;
            if(dirIdx > dir.size() - 1)
                dirIdx = 0;
            
            cliente.set(dir.getDescription(dirIdx));
            string serverName = cliente.getServerName();
            string appName = cliente.getApplicationName();
            
            if(serverName == ""){
                serverName = "null";
            }
            if(appName == ""){
                appName = "null";
            }
            serverStatus = serverName;
        }
        else
        {
            serverStatus = "No Server";
        }
    }
    else{
        
        // unrecognized message: display on the bottom of the screen
        string msgString;
        msgString = m.getAddress();
        msgString += ":";
        for(size_t i = 0; i < m.getNumArgs(); i++){
            
            // get the argument type
            msgString += " ";
            msgString += m.getArgTypeName(i);
            msgString += ":";
            
            // display the argument - make sure we get the right type
            if(m.getArgType(i) == OFXOSC_TYPE_INT32){
                msgString += ofToString(m.getArgAsInt32(i));
            }
            else if(m.getArgType(i) == OFXOSC_TYPE_FLOAT){
                msgString += ofToString(m.getArgAsFloat(i));
            }
            else if(m.getArgType(i) == OFXOSC_TYPE_STRING){
                msgString += m.getArgAsString(i);
            }
            else{
                msgString += "unhandled argument type " + m.getArgTypeName(i);
            }
        }
        
        // add to the list of strings to display
        msgStrings[currentMsgString] = msgString;
        timers[currentMsgString] = ofGetElapsedTimef() + 5.0f;
        currentMsgString = (currentMsgString + 1) % NUM_MSG_STRINGS;
        
        // clear the next line
        msgStrings[currentMsgString] = "";
    }
}
    
    if(enableHaarFinder){
        
      
        textFinder.begin();
        
        ofBackground(0);
        ofPushMatrix();
        ofScale(2,2);
        if(showSquare){
        finder.draw();
        }
       //v ofDrawBitmapStringHighlight(ofToString(finder.size()), 10, 20);
        facesFind=finder.size();
        
        for(int i=0;i<finder.size();i++){
            
           // ofDrawBitmapStringHighlight(ofToString(finder.getObject(i).position*2), finder.getObject(i).position.x, finder.getObject(i).position.y);
            ofVec2f facePos= ofVec2f((int)ofMap(finder.getObject(i).position.x*2, 0,1280,-50,50), (int)ofMap(finder.getObject(i).position.y*2,0,720,50,-50));
            ofVec2f faceScale= ofVec2f(finder.getObject(i).getWidth(), finder.getObject(i).getHeight());
            sendFacePos("/face", facePos, i+1);
            
           
            ofEnableAlphaBlending();
             if(showImage){
            ofSetColor(255);
            faces[showFace].draw(finder.getObject(i).position.x*2,finder.getObject(i).position.y*2,finder.getObject(i).getWidth()*2, finder.getObject(i).getHeight()*2);
             }
            ofDisableAlphaBlending();
            
        }
    
        ofPopMatrix();
        
      textFinder.end();
    }
    if(!enableHaarFinder){
        sendFacePos("/face", ofVec2f(0,0), 0);
    }


    serverFinder.publishTexture(&textFinder.getTexture());
    
}

//--------------------------------------------------------------
void ofApp::draw(){
    
    if(mode == 1 || mode == 2){
        ofSetColor(255);
        ofDrawBitmapString("source: camera input",10,155);
        
        ofSetColor(255);
        if(!mini){
            videoSource->draw(0, 0,sourceWidth,sourceHeight);
        }else{
            videoSource->draw(0,160,200,120);
        }
        
    }
    else if(mode==0 && dir.isValidIndex(dirIdx) ){
        
        if(!mini){
        cliente.draw(0,0,sourceWidth,sourceHeight);
        }
            
        ofSetColor(0,100);
        ofDrawRectangle(0,140, 320/2+40,180/2*2+50);
        
        ofSetColor(255);
        ofDrawBitmapString("source: "+serverStatus,10,155);
        
        ofSetColor(255);
        texIn.draw(0,160,200,120);
        
        if(!mini){
        texIn2.draw(0,160+120,200,120);
        }
 
       
    }
    ofSetColor(255,255,0,255);
    ofDrawBitmapString(" | receive OSC on : "+ofToString(portIn)+" | sending OSC Faces to "+hostFace+":"+ofToString(portFace)+" | caras: "+ofToString(facesFind)+" | imagen: "+ofToString(showFace)+" | Caras: "+ofToString(showImage), 460, ofGetHeight()-4);

    
    if(tracker.getFound()) {
        
        if(!mini){
            
            if(!bPaused){
                ofSetColor(0,255,0,255);
            }else{
                ofSetColor(255,255,0,255);
            }
        ofDrawBitmapString("framerate "+ofToString((int) ofGetFrameRate())+" | sending OSC to "+host+":"+ofToString(port)+" | "+trackStatus, 10, ofGetHeight()-4);
            

        }else{
            
            ofPushStyle();
            if(!bPaused){
                ofSetColor(0,255,0,255);
            }else{
                ofSetColor(255,255,0,255);
            }
            ofDrawBitmapString("framerate "+ofToString((int) ofGetFrameRate()),10,300+miniDisplace);
            ofDrawBitmapString("OSC to: "+host+":"+ofToString(port),10,315+miniDisplace);
            ofDrawBitmapString("status: "+trackStatus,10,330+miniDisplace);

            ofDrawEllipse(10,170,10,10);
            ofPopStyle();
            
        
        }
        
        if(bDrawMesh) {
            
            ofSetLineWidth(1);
            
            if(!mini){
                ofPushView();
                ofSetColor(255);
                
                ofPushMatrix();
                
                if(mode==0){
                     ofScale(2, 2);
                }else{
                     ofScale(1, 1);
                }
                ofSetColor(255);
                tracker.getImageMesh().drawWireframe();
                ofSetColor(0,255,0);
                tracker.draw();
                ofPopMatrix();
                
                 ofSetColor(255);
                ofSetupScreenOrtho(sourceWidth/2, sourceHeight/2, -1000, 1000);
                ofVec2f pos = tracker.getPosition();
                ofTranslate(pos.x, pos.y);
                applyMatrix(rotationMatrix);
                ofScale(10,10,10);
                ofDrawAxis(tracker.getScale()*2);
                ofPopView();
            }
          
            
        }
        
    } else {
        if(!mini){
            ofSetColor(255,0,0,255);
            ofDrawBitmapString("framerate "+ofToString((int) ofGetFrameRate())+ "| searching for face... | "+trackStatus, 10, ofGetHeight()-6);
        }
        if(mini){
           // ofPushStyle();
            ofSetColor(255,0,0,255);
            ofDrawEllipse(10,170,10,10);
            ofDrawBitmapString("searching for face...",10,300+miniDisplace);
            ofDrawBitmapString("framerate "+ofToString((int) ofGetFrameRate()),10,315+miniDisplace);
          //  ofPopStyle();
        }
    }
    
    if(bPaused) {
        trackStatus= "paused";
    }else{
        trackStatus= "running";
    }
    
    if(mode==2) {
        ofSetColor(255, 0, 0);
        ofDrawBitmapString("speed "+ofToString(movie.getSpeed()), ofGetWidth()-100, 20);
    }
    
    if(bGuiVisible) {
        gui.draw();
    }
    if(printValues && !mini) {
        
        ofSetColor(0,100);
        ofDrawRectangle(0, ofGetHeight()-200, ofGetWidth(), 180);
        
        if( tracker.getFound()){
            ofSetColor(255);
            ofDrawBitmapString("face values: ",10,ofGetHeight()-155);
            ofDrawBitmapString("position: x"+ofToString(position.x)+" | y: "+ofToString(position.y), 10,ofGetHeight()-135);
            ofDrawBitmapString("scale: x"+ofToString(scale), 10,ofGetHeight()-120);
            ofDrawBitmapString("orientation: x"+ofToString(orientation.x)+" | y: "+ofToString(ofNormalize(orientation.y,-0.4,0.4))+" | z:"+ofToString(orientation.z), 10, ofGetHeight()-105);
            ofDrawBitmapString("mouth: Height"+ofToString(mouthHeight)+" | with: "+ofToString(mouthtWidth), 10, ofGetHeight()-90);
            ofDrawBitmapString("eyes: Left"+ofToString(leftEyeOpenness)+" | right: "+ofToString(rightEyeOpenness), 10, ofGetHeight()-75);
            ofDrawBitmapString("eyes Brow: Left"+ofToString(leftEyebrowHeight)+" | right: "+ofToString(leftEyebrowHeight), 10, ofGetHeight()-60);
            ofDrawBitmapString("jaw: "+ofToString(jawOpenness), 10, ofGetHeight()-45);
            ofDrawBitmapString("nostril: "+ofToString(nostril), 10, ofGetHeight()-30);
            
            int separa = 500;
            ofDrawBitmapString("| iterations: "+ofToString(bIterations),separa,ofGetHeight()-135);
            ofDrawBitmapString("| clamp: "+ofToString(bClamp), separa,ofGetHeight()-120);
            ofDrawBitmapString("| tolerance: "+ofToString(bTolerance), separa,ofGetHeight()-105);
            ofDrawBitmapString("| attemps: "+ofToString(bAttemps), separa, ofGetHeight()-90);
            
            ofDrawBitmapString("| osc in messages: ", separa + 155, ofGetHeight()-155);
            // draw recent unrecognized messages
            for(int i = 0; i < NUM_MSG_STRINGS; i++){
                ofDrawBitmapString(msgStrings[i], separa + 155, ofGetHeight()-145 + 15 * i);
            }
            
        }
        
      
        
    }
    
    if(mode==0 && !mini && printValues ){
        ofSetColor(255);
        ofDrawBitmapString("press S  to cycle through all available Syphon servers: "+serverStatus, ofPoint(10, ofGetHeight()-180));
    }
    
    if(enableHaarFinder){
        
        ofColor(255);
        ofPushMatrix();
        ofScale(2,2);
        if(showSquare){
        finder.draw();
        }
      //  ofDrawBitmapStringHighlight(ofToString(finder.size()), 10, 20);
        facesFind=finder.size();
        
       for(int i=0;i<finder.size();i++){
        ofEnableAlphaBlending();
        ofSetColor(255);
     //   faces[showFace].draw(finder.getObject(i).position.x,finder.getObject(i).position.y,finder.getObject(i).getWidth(), finder.getObject(i).getHeight());
        ofDisableAlphaBlending();
       }
    }

}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    
    switch(key) {
        case 'r':
            tracker.reset();
            break;
        case 'm':
            bDrawMesh = !bDrawMesh;
            break;
        case 'p':
            bPaused = !bPaused;
              break;
        case 'f':
            enableHaarFinder=!enableHaarFinder;
            break;
        case 'c':
            showSquare=!showSquare;
            break;
        case 'v':
            showImage=!showImage;
            break;
        case 'z':
            if(showFace > 0){
                showFace--;
            }
            break;
        case 'x':
            if(showFace<2){
                showFace++;
            }
            break;
        case 'g':
            bGuiVisible = !bGuiVisible;
            break;
        case OF_KEY_UP:
            movie.setSpeed(ofClamp(movie.getSpeed()+0.2, -16, 16));
            break;
        case OF_KEY_DOWN:
            movie.setSpeed(ofClamp(movie.getSpeed()-0.2, -16, 16));
            break;
        case 's':
            
            if (dir.size() > 0)
            {
                dirIdx++;
                if(dirIdx > dir.size() - 1)
                    dirIdx = 0;
                
                cliente.set(dir.getDescription(dirIdx));
                string serverName = cliente.getServerName();
                string appName = cliente.getApplicationName();
                
                if(serverName == ""){
                    serverName = "null";
                }
                if(appName == ""){
                    appName = "null";
                }
                serverStatus = serverName;
            }
            else
            {
                serverStatus = "No Server";
            }
    }
    
}

//--------------------------------------------------------------
void ofApp::setVideoSource(int mode) {
    
    if(mode ==0) {
        //videoSource = &cam;
        sourceWidth = 1280;
        sourceHeight = 720;
    }
    else if(mode ==1) {
        videoSource = &cam;
        sourceWidth = camWidth;
        sourceHeight = camHeight;
    }
    else if (mode ==2) {
        videoSource = &movie;
        sourceWidth = movieWidth;
        sourceHeight = movieHeight;
    }
    
}
//--------------------------------------------------------------
void ofApp::sendFacePos(string address, ofVec2f pos, int id){
    bundle.clear();
    ofxOscMessage msg;
    msg.setAddress(address);
    msg.addFloatArg(pos.x);
    msg.addFloatArg(pos.y);
    msg.addFloatArg(id);
    bundle.addMessage(msg);
    oscFace.sendBundle(bundle);
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){
    
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){
    
}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){
    
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){
    
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){
    
}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){
    
}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){
    
}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){
    
}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){
    
}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 
    
}

//these are our directory's callbacks
void ofApp::serverAnnounced(ofxSyphonServerDirectoryEventArgs &arg)
{
    for( auto& dir : arg.servers ){
        ofLogNotice("ofxSyphonServerDirectory Server Announced")<<" Server Name: "<<dir.serverName <<" | App Name: "<<dir.appName;
    }
    dirIdx = 0;
}

void ofApp::serverUpdated(ofxSyphonServerDirectoryEventArgs &arg)
{
    for( auto& dir : arg.servers ){
        ofLogNotice("ofxSyphonServerDirectory Server Updated")<<" Server Name: "<<dir.serverName <<" | App Name: "<<dir.appName;
    }
    dirIdx = 0;
}

void ofApp::serverRetired(ofxSyphonServerDirectoryEventArgs &arg)
{
    for( auto& dir : arg.servers ){
        ofLogNotice("ofxSyphonServerDirectory Server Retired")<<" Server Name: "<<dir.serverName <<" | App Name: "<<dir.appName;
    }
    dirIdx = 0;
}
