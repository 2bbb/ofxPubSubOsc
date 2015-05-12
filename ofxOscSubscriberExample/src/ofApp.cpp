#include "ofMain.h"
#include "ofxOscSubscriber.h"

/* launch this and ofxOscPublisherExample */

class ofApp : public ofBaseApp {
public:
    void setup() {
        ofSetWindowPosition(300, 100);
        ofxSubscribeOsc(9005, "/cursor", p);
    }
    
    void update() {
        
    }
    void draw() {
        ofBackground(0);
        ofSetColor(255);
        ofCircle(p, 3);
    }
    
private:
    ofPoint p;
};

int main() {
    ofSetupOpenGL(200, 200, OF_WINDOW);
    ofRunApp(new ofApp());
}
