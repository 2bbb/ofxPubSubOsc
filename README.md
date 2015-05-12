# ofxPubSubOsc

publish/subscribe OSC message.

## TOC

* [How to use][1]
* [API][2]
	* [ofxSubscribeOsc][2.1]
	* [ofxPublishOsc][2.2]
* [Supported types][3]
	* [Arithmetic][3.1]
	* [String][3.2]
	* [openframeworks basic type][3.3]
	* [array/vector of supported types][3.4]
	* [Callback][3.5]
* [Update history][4]
* [License][5]
* [Author][6]
* [Special Thanks][7]
* [At the last][8]

[1]:
## How to use

```

class ofApp : public ofBaseApp {
	int foo;
	ofColor c;
	ofPoint p;
public:
	void setup() {
		ofxSubscribeOsc(9005, "/foo", foo);
		ofxSubscribeOsc(9005, "/color", c);
		ofxSubscribeOsc(9005, "/p", p);
		
		ofxPublishOsc("localhost", 9006, "/fps", &ofGetFrameRate);
	}
	
	void draw() {
		ofSetColor(c);
		ofDrawCircle(p, 5);
	}
};

```

[2]:
## API

[2.1]:
### ofxSubscribeOsc

* template \<typename T\> ofxSubscribeOsc(int _port_, const string &_address_, T &_value_);
* ofxSubscribeOsc(int _port_, const string &_address_, void (*callback)(ofxOscMessage &));
* template \<typename T\> ofxSubscribeOsc(int _port_, const string &_address_, T &_that_, void (T::*_callback_)(ofxOscMessage &));
* template \<typename T\> ofxSubscribeOsc(int _port_, const string &_address_, T * _that_, void (T::*_callback_)(ofxOscMessage &));

bind value/function/method to OSC message has _address_ incoming from _port_.

if use function/method, then call `callback(mess)` or `(that.*callback)(mess)` when receive OSC message `mess`.

* ofxUnsubscribeOSC(int _port_, const string &_address_);

unbind OSC message has _address_ incoming from _port_.

* ofxUnsubscribeOSC(int _port_);

unbind all OSC messages incoming from _port_.

[2.2]:
### ofxPublishOsc

* template \<typename T\> ofxPublishOsc(const string &_ip_, int _port_, const string &_address_, T &_value_);
* template \<typename T\> ofxPublishOsc(const string &_ip_, int _port_, const string &_address_, T (*_getter_)());
* template \<typename T, typename U\> ofxPublishOsc(const string &_ip_, int _port_, const string &_address_, U \*_that_, T (U::*_getter_)());

publish _value_ / _getter()_ / _that.*getter()_ to OSC message has _address_ to _ip:port_.


* ofxUnpublishOSC(int _port_, const string &_address_);

unpublish OSC message has _address_ is send to _ip:port_.

* ofxUnpublishOSC(int _port_);

unpublish all OSC messages is send to _ip:port_.

[3]:
## [supported types]:

[3.1]:
### Arithmetic (Int32, Int64, Float)
* **bool**
* (**unsigned**) **char**
* (**unsigned**) **short**
* (**unsigned**) **int**
* (**unsigned**) **long**
* **float**
* **double**

[3.2]:
### String (String)
* **string**

[3.3]:
### openframeworks basic type

#### Arithmetic\[2\]
* **ofVec2f**

#### Arithmetic\[3\]
* **ofVec3f** (= **ofPoint**)

#### Arithmetic\[4\]
* **ofVec4f**
* **ofColor**
* **ofShortColor**
* **ofFloatColor**
* **ofQuaternion**
* **ofRectangle**

#### Arithmetic\[9\]
* **ofMatrix3x3**

#### Arithmetic\[16\]
* **ofMatrix4x4**

#### Blob
* **ofBuffer** (_now subscribe only_)

[3.4]:
### array/vector of supported types

[3.5]:
### Callback
* **T (\*callback)(ofxOscMessage &)**;
* pair of **U &that**, **T (U::\*callback)(ofxOscMessage &)**;
* pair of **U \*that**, **T (U::\*callback)(ofxOscMessage &)**;

[4]:
## Update history

### 2015/05/11 ver 0.04 release

* support ofMatrix3x3/4x4, ofQuaternion, ofBuffer (only subscribe now), vector<ofXXX>, ofXXX[]
* bugfix for Visual Studio

### 2015/05/11 ver 0.03 release

* add subscribe with callback

### 2015/05/11 ver 0.02 release

* rename from ofxOscSubscriber
* add ofxOscPublisher

### 2015/05/09 ver 0.01 release

[5]:
## License

MIT License.

[6]:
## Author

* ISHII 2bit [bufferRenaiss co., ltd.]
* ishii[at]buffer-renaiss.com

[7]:
## Special Thanks

* [HIGA Satoru](http://github.com/satoruhiga)
* [SHIMIZU Motoi](http://github.com/motoishmz)
* [IWATANI Nariaki](http://github.com/nariakiiwatani)
* [HIEDA Naoto](http://github.com/micuat)

[8]:
## At the last

Please create new issue, if there is a problem.
And please throw pull request, if you have a cool idea!!