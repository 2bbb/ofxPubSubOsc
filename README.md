# ofxPubSubOsc

easy utility for publish/subscribe OSC message.

## Dependencies

* ofxOsc

## Notice

* this addon is tested with oF0.9.0~
* if you use oF0.9.0~, then you can use `std::function<void(ofxOscMessage &)>`! detail: [API Reference](API_Reference.md#API_lambda_callback)
* **if you use oF0.8.2~0.8.4, then you can use [v0.1.2](releases/tag/v0_1_2)**
* if you have challange spirit, please use dev/main branch.
* if you want to join development ofxPubSubOsc, open the issue and post the PR for [dev/main](tree/dev/main).

## TOC

* [How to use](#HowToUse)
* [Simple API Reference](#SimpleAPI)
	* [ofxSubscribeOsc](#SimpleAPI_ofxSubscribeOsc)
	* [ofxPublishOsc](#SimpleAPI_ofxPublishOsc)
	* [ofxRegisterPublishingOsc](#SimpleAPI_ofxRegisterPublishingOsc)
* [Supported types](#SupportedTypes)
	* [Arithmetic](#SupportedTypes_Arithmetic)
	* [String](#SupportedTypes_String)
	* [openframeworks basic types](#SupportedTypes_ofBasic)
	* [ofParameter with supported types](#SupportedTypes_ofParameter)
	* [array/vector of supported types](#SupportedTypes_ArrayVector)
	* [Callback](#SupportedTypes_Callback)
* [Update history](#UpdateHistory)
* [License](#License)
* [Author](#Author)
* [Supporting Contributor](#SuppotingContributor)
* [Special Thanks](#SpecialThanks)
* [At the last](#AtTheLast)

## <a name="HowToUse">How to use</a>

```cpp
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
		
		// if you use v0.9.0~
		ofxSubscribeOsc(9005, "/lambda", [](ofxOscMessage &m){
			ofLogNotice() << m;
		});
	}
	
	void update() {
		// do NOTHING about OSC on update!!!
	}
	
	void draw() {
		ofSetColor(c);
		ofDrawCircle(p, 5);
	}
};

```

## <a name="SimpleAPI">Simple API Reference</a>

#### If you want to use advanced features, see [Advanced](API_Reference.md)

### <a name="SimpleAPI_ofxSubscribeOsc">ofxSubscribeOsc</a>

* void ofxSubscribeOsc(int _port_, const string &_address_, [SupportedType](#SupportedTypes) &_value_);

bind a value to the argument(s) of OSC messages with an address pattern _address_ incoming to _port_.

#### See [more ofxSubscribeOsc](API_Reference.md#API_ofxSubscribeOsc)

* ofxUnsubscribeOsc(int _port_, const string &_address_);

unbind from OSC messages with an address pattern _address_ incoming to _port_.

* ofxUnsubscribeOsc(int _port_);

unbind from OSC messages with any address patterns incoming to _port_.

* if you want to pick up OSCs which do not match the subscribed addresses, see [ofxSetLeadkedOscPicker](API_Reference.md#API_ofxSetLeadkedOscPicker)

#### See [class ofxOscSubscriberManager](API_Reference.md#Advanced_ofxOscSubscriberManager), [class ofxOscSubscriber](API_Reference.md#Advanced_ofxOscSubscriber)

### <a name="SimpleAPI_ofxPublishOsc">ofxPublishOsc</a>

* void ofxPublishOsc(const string &_ip_, int _port_, const string &_address_, [SupportedType](#SupportedTypes) &_value_, bool _whenValueIsChanged_ = **true**);

publish _value_ as an OSC message with an address pattern _address_ to _ip:port_ every time the _value_ has changed. If _whenValueIsChanged_ is set to **false**, then the binded value is sent **every frame** after `App::update`.

#### See [more ofxPublishOsc](API_Reference.md#API_ofxPublishOsc), [ofxPublishOscIf](API_Reference.md#API_ofxPublishOscIf)

* void ofxUnpublishOsc(const string &_ip_, int _port_, const string &_address_);

unbind a publisher sending OSC message with an address pattern _address_ to _ip:port_.

* void ofxUnpublishOsc(const string &_ip_, int _port_);

unbind all the publishers sending to _ip:port_.

#### See [class ofxOscPublisherManager](API_Reference.md#Advanced_ofxOscPublisherManager), [class ofxOscPublisher](API_Reference.md#Advanced_ofxOscPublisher)

### <a name="SimpleAPI_ofxRegisterPublishingOsc">ofxRegisterPublishingOsc</a>

* void ofxRegisterPublishingOsc(const string &_ip_, int _port_, const string &_address_, [SupportedType](#SupportedTypes) &_value_)
* void ofxPublishRegisteredOsc(const string &_ip_, int _port_, const string &_address_)

register value as an OSC message with an address pattern _address_ to _ip:port_. and publish when call `ofxPublishRegisteredOsc(ip, port, address)`.

* void ofxUnregisterPublishingOsc(const string &_ip_, int _port_, const string &_address_)

unregister OSC message with an address pattern _address_ to _ip:port_.

* void ofxUnregisterPublishingOsc(const string &_ip_, int _port_)

unregister all the messages sending to _ip:port_.

**NOTE**: registable type is same to `ofxPublishOsc`. see [more ofxPublishOsc](API_Reference.md#API_ofxPublishOsc).

## <a name="SupportedTypes">Supported types</a>

* Arithmetic is any type of Int32, Int64 or Float

### <a name="SupportedTypes_Arithmetic">Arithmetic (Int32, Int64, Float)</a>
* `bool` (published as _Int32_)
* `unsigned char`, `char` (published as _Int32_)
* `unsigned short`, `short` (published as _Int32_)
* `unsigned int`, `int` (published as _Int32_ or _Int64_ (if `sizeof(int) == 8` then _Int64_))
* `unsigned long`, `long` (published as _Int64_ or _Int64_ (if `sizeof(int) == 8` then _Int64_))
* `unsigned long long`, `long long` (published as _Int64_)
* `float` (published as _Float_)
* `double` (published as _Float_)

**NOTE**: `long double` is not guaranteed

### <a name="SupportedTypes_String">String (String)</a>
* `string`

### <a name="SupportedTypes_ofBasic">openframeworks basic types</a>

#### Arithmetic\[2\]
* `ofVec2f` (published as _Float_ \* 2)

#### Arithmetic\[3\]
* `ofVec3f` (= `ofPoint`) (published as _Float_ \* 3)

#### Arithmetic\[4\]
* `ofVec4f` (published as _Float_ \* 4)
* `ofColor` (published as _Int32_ \* 4)
* `ofShortColor` (published as _Int32_ \* 4)
* `ofFloatColor` (published as _Float_ \* 4)
* `ofQuaternion` (published as _Float_ \* 4)
* `ofRectangle` (published as _Float_ \* 4)

#### Arithmetic\[9\]
* `ofMatrix3x3`  (published as _Float_ \* 9)

#### Arithmetic\[16\]
* `ofMatrix4x4` (publish as _Float_ \* 16)

#### Blob
* `ofBuffer`

### <a name="SupportedTypes_ofParameter">ofParameter with supported types</a>

* `ofParameter<SupportedType>`

**NOTE**: we only support subscribing ofParameterGroup. See [How to subscribe ofParameterGroup](API_Reference.md#Advanced_how_to_subscribe_ofParameterGroup)

### <a name="SupportedTypes_ArrayVector">array/vector of supported types</a>

* `SupportedType[size]`
* `vector<SupportedType>`

if you use `vector<SomeType> vec;`, when `vec` will be resized every receiving OSC messages.

**NOTE**: do NOT use `vector<vector<SupportedType>>`, `vector<SupportedType>[size]`

### <a name="SupportedTypes_Callback">Callback</a>
* `T (\*callback)(ofxOscMessage &)`;
* pair of `U &that`, `T (U::\*callback)(ofxOscMessage &)`;
* pair of `U \*that`, `T (U::\*callback)(ofxOscMessage &)`;
* `std::function<void(ofxOscMessage &)>`

## <a name="UpdateHistory">Update history</a>

### 2016/01/02 [ver 0.2.0](releases/tag/v2_0_0) release

* *after this release, we will only test on oF0.9.0~*
* new feature: multi-subscribe, multi-publish
	* `ofxSubscribeOsc` returns `ofxSubscriberIdentifier`
	* `ofxPublishOsc` returns `ofxPublisherIdentifier`
	* TODO: API Reference
* add iterators to [ofxOscSubscriberManager](API_Reference.md#Advanced_ofxOscSubscriberManager)
* add iterators to [ofxOscPublisherManager](API_Reference.md#Advanced_ofxOscPublisherManager)
* add all port operation to ofxUnsubscribeOsc, ofxNotifyToSubscribedOsc, ofxRemoveLeakedOscPicker
* add ofxSetLeakedOscPickerAll
* add ofxSubscribeOsc with `std::initializer_list<int> port` and `std::initializer_list<std::string> addresses`
* add iterators to [ofxOscPublisherManager](API_Reference.md#Advanced_ofxOscPublisherManager)
* add all port operation to ofxUnpublishOsc, ofxUnregisterPublishingOsc
* add feature publishing r-value. (i.e., you can do `ofxPublishOsc(host, port, "/bar", "value!!")`)
* add `const` to lambda callback (proposed by [satoruhiga](https://github.com/satoruhiga). thanks!!)
* add useful macro `SubscribeOsc(port, name)` is same as `ofxSubscribeOsc(port, "/name", name)` (porposed by [hanasaan](https://github.com/hanasaan). thanks!!)
* add `std::` prefix
* cleaning up conditional macro about oF0.8.x
* some bugfix around lambda
* TODO: update some API Documentations

### 2016/01/02 [ver 0.1.2](releases/tag/v0_1_2) release

* this is *final update added new feature, with oF0.8.4 support*
* after this release, "ver 0.1.x will only bugfie about supporting oF0.8.4
* add new feature ofxNotifyToSubscribedOsc (proposed by [satcy](https://github.com/satcy). thanks!!)
* some bugfix

### [Older update histories](Update_History.md)


#### about Versioning

ofxPubSubOsc uses Mood Versioning. maybe, 1.0.0. will not come.

## <a name="License">License</a>

MIT License.

## <a name="Author">Author</a>

* ISHII 2bit [bufferRenaiss co., ltd.]
* ishii[at]buffer-renaiss.com

## <a name="SuppotingContributor">Supporting Contributor</a>

* [HIEDA Naoto](https://github.com/micuat)

## <a name="SpecialThanks">Special Thanks</a>

* [HIGA Satoru](https://github.com/satoruhiga)
* [SHIMIZU Motoi](https://github.com/motoishmz)
* [IWATANI Nariaki](https://github.com/nariakiiwatani)
* [USAMI Takuto](https://github.com/usm916)
* [HORII Satoshi](https://github.com/satcy)

## <a name="AtTheLast">At the last</a>

Please create a new issue if there is a problem.
And please throw a pull request if you have a cool idea!!
