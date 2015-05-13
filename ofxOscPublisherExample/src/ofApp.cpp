#include "ofMain.h"
#include "ofxOscPublisher.h"

/* launch this and ofxOscSubscriberExample */

class ofApp : public ofBaseApp {
public:
    void setup() {
        for(int i = 0; i < 16; i++) {
            m.addVertex(ofVec2f());
        }
        ofSetWindowPosition(100, 100);
        ofxPublishOsc("localhost", 9005, "/cursor", p);
        ofxPublishOsc("localhost", 9005, "/fps", &ofGetFrameRate);
        ofxPublishOsc("localhost", 9005, "/mesh", m.getVertices());
        ofEnableSmoothing();
        glPointSize(3);
    }
    
    void update() {
        p.x = ofGetMouseX();
        p.y = ofGetMouseY();
        for(int i = 0; i < m.getNumVertices(); i++) {
            m.setVertex(i, ofVec2f(ofRandom(200), ofRandom(200)));
        }
    }
    void draw() {
        ofBackground(255);
        ofSetColor(0);
        ofDrawBitmapString("move mouse here!", 10, 30);
        m.drawVertices();
    }
    
private:
    ofPoint p;
    ofMesh m;
};

int main() {
    ofSetupOpenGL(200, 200, OF_WINDOW);
    ofRunApp(new ofApp());
}
