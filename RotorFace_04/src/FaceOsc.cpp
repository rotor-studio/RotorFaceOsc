#include "FaceOsc.hpp"

void FaceOsc::sendFaceOsc(ofxFaceTracker& tracker) {
    
    
    
    clearBundle();
    
    if(tracker.getFound()) {
        
        tracker.setIterations(bIterations);
        tracker.setClamp(bClamp);
        tracker.setTolerance(bTolerance);
        tracker.setAttempts(bAttemps);
        

        
        addMessage("/found", 1);
        
        if(bIncludePose) {
            position = tracker.getPosition();
            addMessage("/pose/position", position);
            scale = tracker.getScale();
            addMessage("/pose/scale", scale);
            orientation = tracker.getOrientation();
            //addMessage("/pose/orientation", orientation);
            addMessage("/pose/orientation", ofVec3f(orientation.x,ofNormalize(orientation.y, -0.4, 0.4),orientation.z));
        }
        
        if (bIncludeGestures) {
           
         //  addMessage("/gesture/mouth/width", tracker.getGesture(ofxFaceTracker::MOUTH_WIDTH));
         //  mouthtWidth= tracker.getGesture(ofxFaceTracker::MOUTH_WIDTH);
         //  addMessage("/gesture/mouth/height", tracker.getGesture(ofxFaceTracker::MOUTH_HEIGHT));
       //     mouthHeight= tracker.getGesture(ofxFaceTracker::MOUTH_HEIGHT);
         //   addMessage("/gesture/eyebrow/left", tracker.getGesture(ofxFaceTracker::LEFT_EYEBROW_HEIGHT));
         //   leftEyebrowHeight= tracker.getGesture(ofxFaceTracker::LEFT_EYEBROW_HEIGHT);
         //   addMessage("/gesture/eyebrow/right", tracker.getGesture(ofxFaceTracker::RIGHT_EYEBROW_HEIGHT));
          //  rightEyebrowHeight= tracker.getGesture(ofxFaceTracker::RIGHT_EYEBROW_HEIGHT);
         //   addMessage("/gesture/eye/left", tracker.getGesture(ofxFaceTracker::LEFT_EYE_OPENNESS));
        //    leftEyeOpenness= tracker.getGesture(ofxFaceTracker::LEFT_EYE_OPENNESS);
         //   addMessage("/gesture/eye/right", tracker.getGesture(ofxFaceTracker::RIGHT_EYE_OPENNESS));
        //    rightEyeOpenness=tracker.getGesture(ofxFaceTracker::RIGHT_EYE_OPENNESS);
         //   addMessage("/gesture/jaw", tracker.getGesture(ofxFaceTracker::JAW_OPENNESS));
           // jawOpenness= tracker.getGesture(ofxFaceTracker::JAW_OPENNESS);
          //  addMessage("/gesture/nostrils", tracker.getGesture(ofxFaceTracker::NOSTRIL_FLARE));
          //  nostril= tracker.getGesture(ofxFaceTracker::NOSTRIL_FLARE);
            

            
            
             mouthtWidth = ofNormalize(tracker.getGesture(ofxFaceTracker::MOUTH_WIDTH),11.0f,16.0f);
            addMessage("/gesture/mouth/width", mouthtWidth);
            mouthHeight = ofNormalize(tracker.getGesture(ofxFaceTracker::MOUTH_HEIGHT),0.0f,7.0f);
            addMessage("/gesture/mouth/height", mouthHeight);
            leftEyebrowHeight = ofNormalize(tracker.getGesture(ofxFaceTracker::LEFT_EYEBROW_HEIGHT),7.2f,9.0f);
            addMessage("/gesture/eyebrow/left", leftEyebrowHeight);
            rightEyebrowHeight = ofNormalize(tracker.getGesture(ofxFaceTracker::RIGHT_EYEBROW_HEIGHT),7.2f,9.0f);
            addMessage("/gesture/eyebrow/right", rightEyebrowHeight);
            leftEyeOpenness = ofNormalize(tracker.getGesture(ofxFaceTracker::LEFT_EYE_OPENNESS),2.5f,3.0f);
            addMessage("/gesture/eye/left", leftEyeOpenness);
            rightEyeOpenness = ofNormalize(tracker.getGesture(ofxFaceTracker::RIGHT_EYE_OPENNESS),2.5f,3.0f);
            addMessage("/gesture/eye/right",rightEyeOpenness);
            jawOpenness = ofNormalize(tracker.getGesture(ofxFaceTracker::JAW_OPENNESS),18.0f,25.0f);
            addMessage("/gesture/jaw", jawOpenness);
            nostril = ofNormalize(tracker.getGesture(ofxFaceTracker::NOSTRIL_FLARE),6.0f,8.0f);
            addMessage("/gesture/nostrils", nostril);
        }
        
     
        if(bIncludeAllVertices){
            ofVec2f center = tracker.getPosition();
            ofxOscMessage msg;
            msg.setAddress("/raw");
            for(ofVec2f p : tracker.getImagePoints()) {
                if (bNormalizeRaw) {
                    msg.addFloatArg((p.x-center.x)/tracker.getScale());
                    msg.addFloatArg((p.y-center.y)/tracker.getScale());
                }
                else {
                    msg.addFloatArg(p.x);
                    msg.addFloatArg(p.y);
                }
            }
            bundle.addMessage(msg);
        }
        
    }
    else {
      //  addMessage("/found", 0);
    }
    
    sendBundle();
}

void FaceOsc::clearBundle() {
    bundle.clear();
}

void FaceOsc::addMessage(string address, ofVec3f data) {
    ofxOscMessage msg;
    msg.setAddress(address);
    msg.addFloatArg(data.x);
    msg.addFloatArg(data.y);
    msg.addFloatArg(data.z);
    bundle.addMessage(msg);
}

void FaceOsc::addMessage(string address, ofVec2f data) {
    ofxOscMessage msg;
    msg.setAddress(address);
    msg.addFloatArg(data.x);
    msg.addFloatArg(data.y);
    bundle.addMessage(msg);
}

void FaceOsc::addMessage(string address, float data) {
    ofxOscMessage msg;
    msg.setAddress(address);
    msg.addFloatArg(data);
    bundle.addMessage(msg);
}

void FaceOsc::addMessage(string address, int data) {
    ofxOscMessage msg;
    msg.setAddress(address);
    msg.addIntArg(data);
    bundle.addMessage(msg);
}

void FaceOsc::sendBundle() {
    osc.sendBundle(bundle);
}


