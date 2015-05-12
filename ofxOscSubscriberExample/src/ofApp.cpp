#include "ofMain.h"
#include "ofxOscSubscriber.h"

/* launch this and ofxOscPublisherExample */

class ofApp : public ofBaseApp {
public:
    void setup() {
        ofSetWindowPosition(300, 100);
        ofxSubscribeOsc(9005, "/cursor", p);
        ofxSubscribeOsc(9005, "/fps", fps);
    }
    
    void update() {

    }
    void draw() {
        ofBackground(0);
        ofSetColor(255);
        ofDrawBitmapString("Publisher fps: " + ofToString(fps), 10, 10);

        ofCircle(p, 3);
    }
    
private:
    ofPoint p;
    float fps;
};

int main() {
    ofSetupOpenGL(200, 200, OF_WINDOW);
    ofRunApp(new ofApp());
}
