#include "ofMain.h"
#include "ofxOscPublisher.h"

/* launch this and ofxOscSubscriberExample */

class ofApp : public ofBaseApp {
public:
    void setup() {
        ofSetWindowPosition(100, 100);
        ofxPublishOsc("localhost", 9005, "/cursor", p);
        ofxPublishOsc("localhost", 9005, "/fps", &ofGetFrameRate);
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
