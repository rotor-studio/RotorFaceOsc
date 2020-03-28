#pragma once

#include "ofMain.h"
#include "ofxGui.h"
#include "ofxCv.h"
#include "FaceOsc.hpp"
#include "ofxSyphon.h"

// max number of strings to display
#define NUM_MSG_STRINGS 8



class ofApp : public ofBaseApp, public FaceOsc {
public:
    
        void iterations(int & ite) ;
        void loadSettings();
        void setup();
        void update();
        void draw();
        void setVideoSource(int mode);
        void sendFacePos(string address, ofVec2f pos,int ID);
       

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void mouseEntered(int x, int y);
		void mouseExited(int x, int y);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
    
    //Cam stuff.
    bool bPaused;
    int mode;
    string modeName;
    
    int camWidth, camHeight;
    int movieWidth, movieHeight;
    int sourceWidth, sourceHeight;
    
    ofVideoGrabber cam;
    ofVideoPlayer movie;
    ofBaseVideoDraws *videoSource;
    
    ofxFaceTracker tracker;
    glm::mat4 rotationMatrix;
    bool bDrawMesh;
    string trackStatus;
    
    //Findefaces.
    ofxCv::ObjectFinder finder;
    int facesFind;
    
    ofxPanel gui;
    bool bGuiVisible;
    
    //sending stuff
    ofParameter<bool> printValues;
    ofParameter<bool> enableHaarFinder;
  
    //syphon stuff
    ofTexture texOut;
    ofFbo texIn;
    ofImage  texIn2;
    
    
    ofFbo textFinder;
    ofxSyphonServer serverFinder;
    ofTexture  texFace;
    ofImage faces[3];
    bool showSquare;
     bool showImage;
    int showFace;
    
    ofxSyphonServerDirectory dir;
    ofxSyphonServer server;
    ofxSyphonClient cliente;
    ofParameter<bool> syphonOut;
    int dirIdx;
    string serverStatus;
    
    void serverAnnounced(ofxSyphonServerDirectoryEventArgs &arg);
    void serverUpdated(ofxSyphonServerDirectoryEventArgs &args);
    void serverRetired(ofxSyphonServerDirectoryEventArgs &arg);
    
    
    //Gui stuff
    ofParameter<bool> mini;
    int miniDisplace;
    bool syphonMini;
		
    //OSC Face y ordenes.
    ofxOscReceiver oscIn;
    ofxOscSender oscFace;
    ofxOscBundle bundle;
    
    string hostIn;
    int portIn;
    string hostFace;
    int portFace;
    
    int currentMsgString;
    string msgStrings[NUM_MSG_STRINGS];
    float timers[NUM_MSG_STRINGS];

};
