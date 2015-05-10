#include "ofMain.h"

#include "ofxOscSubscriber.h"

class ofApp : public ofBaseApp{
public:
    void setup() {
        subscribeX();
        subscribeY();
        
        sender.setup("localhost", 9005);
    }
    
    void subscribeX() {
        ofxSubscribeOsc(9005, "/x", x);
    }
    
    void unsubscribeX() {
        ofxUnsubscribeOsc(9005, "/x");
    }
    
    void subscribeY() {
        ofxSubscribeOsc(9005, "/y", y);
    }
    
    void unsubscribeY() {
        ofxUnsubscribeOsc(9005, "/y");
    }
    
    void subscribeP() {
        ofxSubscribeOsc(9005, "/p", p);
    }
    
    void unsubscribeP() {
        ofxUnsubscribeOsc(9005, "/p");
    }
    
    void update() {
        {
            ofxOscMessage m;
            m.setAddress("/x");
            m.addIntArg(static_cast<int>(ofRandom(200)));
            sender.sendMessage(m);
        }
        
        {
            ofxOscMessage m;
            m.setAddress("/y");
            m.addIntArg(static_cast<int>(ofRandom(200)));
            sender.sendMessage(m);
        }
        
        {
            ofxOscMessage m;
            m.setAddress("/p");
            m.addFloatArg(ofRandom(200));
            m.addFloatArg(ofRandom(200));
            sender.sendMessage(m);
        }
    }
    void draw() {
        ofBackground(0);
        ofSetColor(255);
        
        stringstream s("");
        s << "x: " << x << ", y: " << y;
        ofDrawBitmapString(s.str(), ofPoint(20, 20));
        
        ofCircle(p, 3);
    }
    
    void keyPressed(int key) {
        switch (key) {
            case 'p':
                subscribeP();
                break;
            case 'x':
                subscribeX();
                break;
            case 'y':
                subscribeY();
                break;
            case 'P':
                unsubscribeP();
                break;
            case 'X':
                unsubscribeX();
                break;
            case 'Y':
                unsubscribeY();
                break;
            default:
                break;
        }
    }
    
private:
    int x, y;
    ofPoint p;
    ofxOscSender sender;
};

int main() {
    ofSetupOpenGL(200, 200, OF_WINDOW);
    ofRunApp(new ofApp());
}
