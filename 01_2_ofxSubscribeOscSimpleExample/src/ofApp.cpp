#include "ofMain.h"
#include "ofxOscSubscriber.h"

/* launch this and ofxPublishOscSimpleExample */

class ofApp : public ofBaseApp {
public:
    void setup() {
        ofSetWindowPosition(300, 100);
        
        // subscribe ofPoint
        ofxSubscribeOsc(9005, "/cursor", p);
        
        // subscribe float value
        ofxSubscribeOsc(9005, "/fps", fps);
        ofxSubscribeOsc(9005, "/windowPosition/x", windowPosition.x);
        ofxSubscribeOsc(9005, "/windowPosition/y", windowPosition.y);
    }
    
    void update() {
        ofSetWindowPosition(windowPosition.x + 200, windowPosition.y);
    }
    
    void draw() {
        ofBackground(0);
        ofSetColor(255);
        ofDrawBitmapString("Publisher fps: " + ofToString(fps), 10, 30);
        ofDrawBitmapString("Subscriber fps: " + ofToString(ofGetFrameRate()), 10, 50);
        
        ofCircle(p, 3);
    }
    
private:
    ofPoint p;
    float fps;
    ofPoint windowPosition;
};

int main() {
    ofSetupOpenGL(200, 200, OF_WINDOW);
    ofRunApp(new ofApp());
}
