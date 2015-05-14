# ofxPubSubOsc

publish/subscribe OSC message.

## TOC

* [How to use](#HowToUse)
* [API](#API)
	* [ofxSubscribeOsc](#API_ofxSubscribeOsc)
	* [ofxSetLeakedOscPicker](#API_ofxSetLeakedOscPicker)
	* [ofxPublishOsc](#API_ofxPublishOsc)
* [Supported types](#SupportedTypes)
	* [Arithmetic](#SupportedTypes_Arithmetic)
	* [String](#SupportedTypes_String)
	* [openframeworks basic types](#SupportedTypes_ofBasic)
	* [array/vector of supported types](#SupportedTypes_ArrayVector)
	* [Callback](#SupportedTypes_Callback)
* [Update history](#UpdateHistory)
* [License](#License)
* [Author](#Author)
* [Special Thanks](#SpecialThanks)
* [At the last](#AtTheLast)

## <a name="HowToUse">How to use</a>

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

## <a name="API">API</a>

### <a name="API_ofxSubscribeOsc">ofxSubscribeOsc</a>

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

#### See [class ofxOscSubscriber](#Advanced_ofxOscSubscriber)

### <a name="API_ofxSetLeakedOscPicker">ofxSetLeakedOscPicker</a>

* void ofxSetLeakedOscPicker(int _port_, void (\*_callback_)(ofxOscMessage &))
* template \<typename T\> void ofxSetLeakedOscPicker(int _port_, T \*_that_, void (T::\*_callback_)(ofxOscMessage &)) 
* template \<typename T\> void ofxSetLeakedOscPicker(int _port_, T &_that_, void (T::\*_callback_)(ofxOscMessage &)) 

set callback for port. this callback is kick when receive OSC message has not binded address.

* void ofxRemoveLeakedOscPicker(int _port_)

remove callback.

#### See [Legacy style pick up leaked OSC](#Advanced_LegacyStylePickUpLeakedOSCMessage)

### <a name="API_ofxPublishOsc">ofxPublishOsc</a>

* template \<typename T\> ofxPublishOsc(const string &_ip_, int _port_, const string &_address_, T &_value_, bool _whenValueIsChanged_ = **true**);
* template \<typename T\> ofxPublishOsc(const string &_ip_, int _port_, const string &_address_, T (*_getter_)(), bool _whenValueIsChanged_ = **true**);
* template \<typename T, typename U\> ofxPublishOsc(const string &_ip_, int _port_, const string &_address_, U \*_that_, T (U::*_getter_)(), bool _whenValueIsChanged_ = **true**);

publish _value_ / _getter()_ / _(that.*getter)()_ to OSC message has _address_ to _ip:port_. if _whenValueIsChanged_ is set to **false**, then we send binded value **every frame** after `App::update`.


* ofxUnpublishOSC(int _port_, const string &_address_);

unpublish OSC message has _address_ is send to _ip:port_.

* ofxUnpublishOSC(int _port_);

unpublish all OSC messages is send to _ip:port_.

#### See [class ofxOscPublisher](#Advanced_ofxOscPublisher)

## <a name="AdvancedAPI">Advanced API</a>

### <a name="Advanced_ofxOscSubscriber">class ofxOscSubscriber</a>

TODO

#### <a name="Advanced_LegacyStylePickUpLeakedOSCMessage">How to "Legacy style pick up leaked OSC"</a>

TODO

### <a name="Advanced_ofxOscPublisher">class ofxOscPublisher</a>

TODO

## <a name="SupportedTypes">Supported types</a>

### <a name="SupportedTypes_Arithmetic">Arithmetic (Int32, Int64, Float)</a>
* **bool**
* (**unsigned**) **char**
* (**unsigned**) **short**
* (**unsigned**) **int**
* (**unsigned**) **long**
* **float**
* **double**

### <a name="SupportedTypes_String">String (String)</a>
* **string**

### <a name="SupportedTypes_ofBasic">openframeworks basic types</a>

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

### <a name="SupportedTypes_ArrayVector">array/vector of supported types</a>

### <a name="SupportedTypes_Callback">Callback</a>
* **T (\*callback)(ofxOscMessage &)**;
* pair of **U &that**, **T (U::\*callback)(ofxOscMessage &)**;
* pair of **U \*that**, **T (U::\*callback)(ofxOscMessage &)**;

## <a name="UpdateHistory">Update history</a>

### 2015/05/15 ver 0.05 release

* add system about pick up leaked OSC messages
* some bugfix
* add examples

### 2015/05/11 ver 0.04 release

* support ofMatrix3x3/4x4, ofQuaternion, ofBuffer (only subscribe now), vector<ofXXX>, ofXXX[]
* bugfix for Visual Studio

### 2015/05/11 ver 0.03 release

* add subscribe with callback

### 2015/05/11 ver 0.02 release

* rename from ofxOscSubscriber
* add ofxOscPublisher

### 2015/05/09 ver 0.01 release

## <a name="License">License</a>

MIT License.

## <a name="Author">Author</a>

* ISHII 2bit [bufferRenaiss co., ltd.]
* ishii[at]buffer-renaiss.com

## <a name="SpecialThanks">Special Thanks</a>

* [HIGA Satoru](http://github.com/satoruhiga)
* [SHIMIZU Motoi](http://github.com/motoishmz)
* [IWATANI Nariaki](http://github.com/nariakiiwatani)
* [HIEDA Naoto](http://github.com/micuat)

## <a name="AtTheLast">At the last</a>

Please create new issue, if there is a problem.
And please throw pull request, if you have a cool idea!!