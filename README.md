# ofxPubSubOsc

publish/subscribe OSC message.

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

## API

### ofxSubscribeOsc

* template \<typename T\> ofxSubscribeOsc(int _port_, const string &_address_, T &_value_);
* ofxSubscribeOsc(int _port_, const string &_address_, void (*callback)(ofxOscMessage &));
* template \<typename T\> ofxSubscribeOsc(int _port_, const string &_address_, T &_that_, void (T::*_callback_)(ofxOscMessage &));
* template \<typename T\> ofxSubscribeOsc(int _port_, const string &_address_, T * _that_, void (T::*_callback_)(ofxOscMessage &));

bind value/function/method to OSC message has _address_ incoming from _port_.

if use function/method, then call `callback(mess)` or `that.*callback(mess)` when receive OSC message `mess`.

* ofxUnsubscribeOSC(int _port_, const string &_address_);

unbind OSC message has _address_ incoming from _port_.

* ofxUnsubscribeOSC(int _port_);

unbind all OSC messages incoming from _port_.

### ofxPublishOsc

* template \<typename T\> ofxPublishOsc(const string &_ip_, int _port_, const string &_address_, T &_value_);
* template \<typename T\> ofxPublishOsc(const string &_ip_, int _port_, const string &_address_, T (*_getter_)());
* template \<typename T, typename U\> ofxPublishOsc(const string &_ip_, int _port_, const string &_address_, U \*_that_, T (U::*_getter_)());

publish _value_ / _getter()_ / _that.*getter()_ to OSC message has _address_ to _ip:port_.


* ofxUnpublishOSC(int _port_, const string &_address_);

unpublish OSC message has _address_ is send to _ip:port_.

* ofxUnpublishOSC(int _port_);

unpublish all OSC messages is send to _ip:port_.

## Update history

### 2015/05/11 ver 0.03 release

* add subscribe with callback

### 2015/05/11 ver 0.02 release

* rename from ofxOscSubscriber
* add ofxOscPublisher

### 2015/05/09 ver 0.01 release

## License

MIT License.

## Author

* ISHII 2bit [bufferRenaiss co., ltd.]
* ishii[at]buffer-renaiss.com

## Special Thanks

* [HIGA Satoru](http://github.com/satoruhiga)
* [SHIMIZU Motoi](http://github.com/motoishmz)
* [IWATANI Nariaki](http://github.com/nariakiiwatani)
* [HIEDA Naoto](http://github.com/micuat)

## At the last

Please create new issue, if there is a problem.
And please throw pull request, if you have a cool idea!!