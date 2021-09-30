#include "ofMain.h"
#include "ofxOscMessageEx.h"

class ofApp : public ofBaseApp {
public:
    void setup() {
        // create by constructor
        ofxOscMessageEx m("/address", 1, 2, 3.0f, "foo");
        // inherit ofxOscMessage
        m.addIntArg(4);
        // or simply
        m.add(5);
        m.add(glm::vec2{6.0, 7ul});
        // or like stream
        m << 8.0f << glm::vec2{9.0f, 10.0f} << "string";
        
        // get by subscript
        int x = m[0];
        // give the type explicitly
        int y = m[1].as<int>();
        float z = m[2];
        
        // std::string is troublesome...
        // with constructor is ok
        std::string foo = m[3];
        // substituion will failure... use type parameter explicitly
        foo = m[3].as<std::string>();
        
        // parse multiple value from offset is gived by index
        glm::vec3 v = m[0];
        // or, if index is 0 then omittable
        v = m;
        
        ofExit();
    }
};

int main() {
    ofSetupOpenGL(1024, 768, OF_WINDOW);
    ofRunApp(new ofApp());
}
