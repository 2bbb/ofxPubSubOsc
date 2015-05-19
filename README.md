# ofxPubSubOsc

publish/subscribe OSC message.

## TOC

* [How to use](#HowToUse)
* [Simple API Reference](#SimpleAPI)
	* [ofxSubscribeOsc](#SimpleAPI_ofxSubscribeOsc)
	* [ofxPublishOsc](#SimpleAPI_ofxPublishOsc)
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

## <a name="SimpleAPI">Simple API Reference</a>

#### If you want to use advanced features, see [Advanced](https://github.com/2bbb/ofxPubSubOsc/blob/master/API_Reference.md)

### <a name="SimpleAPI_ofxSubscribeOsc">ofxSubscribeOsc</a>

* template \<typename T\> void ofxSubscribeOsc(int _port_, const string &_address_, T &_value_);

bind a value to the argument(s) of OSC messages with an address pattern _address_ incoming to _port_.

#### See [more ofxSubscribeOsc](https://github.com/2bbb/ofxPubSubOsc/blob/master/API_Reference.md#API_ofxSubscribeOsc)

* ofxUnsubscribeOsc(int _port_, const string &_address_);

unbind from OSC messages with an address pattern _address_ incoming to _port_.

* ofxUnsubscribeOsc(int _port_);

unbind from OSC messages with any address patterns incoming to _port_.

* if you want to pick up OSCs which do not match the subscribed addresses, see [ofxSetLeadkedOscPicker](https://github.com/2bbb/ofxPubSubOsc/blob/master/API_Reference.md#API_ofxSetLeadkedOscPicker)

#### See [class ofxOscSubscriberManager](https://github.com/2bbb/ofxPubSubOsc/blob/master/API_Reference.md#Advanced_ofxOscSubscriberManager), [class ofxOscSubscriber](https://github.com/2bbb/ofxPubSubOsc/blob/master/API_Reference.md#Advanced_ofxOscSubscriber)

### <a name="SimpleAPI_ofxPublishOsc">ofxPublishOsc</a>

* template \<typename T\> void ofxPublishOsc(const string &_ip_, int _port_, const string &_address_, T &_value_, bool _whenValueIsChanged_ = **true**);

publish _value_ as an OSC message with an address pattern _address_ to _ip:port_ every time the _value_ has changed. If _whenValueIsChanged_ is set to **false**, then the binded value is sent **every frame** after `App::update`.

#### See [more ofxPublishOsc](https://github.com/2bbb/ofxPubSubOsc/blob/master/API_Reference.md#API_ofxPublishOsc), [ofxPublishOscIf](https://github.com/2bbb/ofxPubSubOsc/blob/master/API_Reference.md#API_ofxPublishOscIf)

* void ofxUnpublishOsc(const string &_ip_, int _port_, const string &_address_);

unbind a publisher sending OSC messages with an address pattern _address_ to _ip:port_.

* void ofxUnpublishOsc(const string &_ip_, int _port_);

unbind all the publishers sending to _ip:port_.

#### See [class ofxOscPublisherManager](https://github.com/2bbb/ofxPubSubOsc/blob/master/API_Reference.md#Advanced_ofxOscPublisherManager), [class ofxOscPublisher](https://github.com/2bbb/ofxPubSubOsc/blob/master/API_Reference.md#Advanced_ofxOscPublisher)

## <a name="SupportedTypes">Supported types</a>

* Arithmetic is any type of Int32, Int64 or Float

### <a name="SupportedTypes_Arithmetic">Arithmetic (Int32, Int64, Float)</a>
* **bool** (published as _Int32_)
* (**unsigned**) **char** (published as _Int32_)
* (**unsigned**) **short** (published as _Int32_)
* (**unsigned**) **int** (published as _Int32_)
* (**unsigned**) **long** (published as _Int64_)
* **float** (published as _Float_)
* **double** (published as _Float_)

### <a name="SupportedTypes_String">String (String)</a>
* **string**

### <a name="SupportedTypes_ofBasic">openframeworks basic types</a>

#### Arithmetic\[2\]
* **ofVec2f** (published as _Float_ \* 2)

#### Arithmetic\[3\]
* **ofVec3f** (= **ofPoint**) (published as _Float_ \* 3)

#### Arithmetic\[4\]
* **ofVec4f** (published as _Float_ \* 4)
* **ofColor** (published as _Int32_ \* 4)
* **ofShortColor** (published as _Int32_ \* 4)
* **ofFloatColor** (published as _Float_ \* 4)
* **ofQuaternion** (published as _Float_ \* 4)
* **ofRectangle** (published as _Float_ \* 4)

#### Arithmetic\[9\]
* **ofMatrix3x3**  (published as _Float_ \* 9)

#### Arithmetic\[16\]
* **ofMatrix4x4** (published as _Float_ \* 16)

#### Blob
* **ofBuffer**

### <a name="SupportedTypes_ArrayVector">array/vector of supported types</a>

### <a name="SupportedTypes_Callback">Callback</a>
* **T (\*callback)(ofxOscMessage &)**;
* pair of **U &that**, **T (U::\*callback)(ofxOscMessage &)**;
* pair of **U \*that**, **T (U::\*callback)(ofxOscMessage &)**;

## <a name="UpdateHistory">Update history</a>

### 2015/05/19 ver 0.0.7 release

* add ofxPublishOscIf
* some bugfix

### 2015/05/17 ver 0.0.6 release

* add publish ofBuffer as blob
* some bugfix
* big change on inner class structure

### 2015/05/15 ver 0.0.5 release

* add system about pick up leaked OSC messages
* some bugfix
* add examples

### 2015/05/11 ver 0.0.4 release

* support ofMatrix3x3/4x4, ofQuaternion, ofBuffer (only subscribe now), vector<ofXXX>, ofXXX[]
* bugfix for Visual Studio

### 2015/05/11 ver 0.0.3 release

* add subscribe with callback

### 2015/05/11 ver 0.0.2 release

* rename from ofxOscSubscriber
* add ofxOscPublisher

### 2015/05/09 ver 0.0.1 release

* initial

#### about Versioning

ofxPubSubOsc uses Mood Versioning. maybe, 1.0.0. will not come.

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

Please create a new issue if there is a problem.
And please throw a pull request if you have a cool idea!!
