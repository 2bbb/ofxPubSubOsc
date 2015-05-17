#include "ofMain.h"
#include "ofxOscPublisher.h"

/* launch this and ofxSubscribOscSimpleExample */

class ofApp : public ofBaseApp {
public:
    void setup() {
        ofSetWindowPosition(100, 100);
        
        // publish ofPoint
        ofxPublishOsc("localhost", 9005, "/cursor", p);
        
        // publish function returns float
        ofxPublishOsc("localhost", 9005, "/fps", &ofGetFrameRate);
        ofxPublishOsc("localhost", 9005, "/windowPosition/x", &ofGetWindowPositionX);
        ofxPublishOsc("localhost", 9005, "/windowPosition/y", &ofGetWindowPositionY);
    }
    
    void update() {
        p.x = ofGetMouseX();
        p.y = ofGetMouseY();
    }
    
    void draw() {
        ofBackground(255);
        ofSetColor(0);
        ofDrawBitmapString("move mouse here!", 10, 30);
    }
    
private:
    ofPoint p;
};

int main() {
    ofSetupOpenGL(200, 200, OF_WINDOW);
    ofRunApp(new ofApp());
}
