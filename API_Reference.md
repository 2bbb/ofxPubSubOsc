# ofxPubSubOsc API Reference

## TOC

* [API](#API)
	* [ofxSubscribeOsc](#API_ofxSubscribeOsc)
	* [ofxSetLeakedOscPicker](#API_ofxSetLeakedOscPicker)
	* [ofxPublishOsc](#API_ofxPublishOsc)
		* [ofxPublishOscIf](API_ofxPublishOscIf)
		* [ofxPublishAsArray](API_ofxPublishAsArray)
		* [ofxSetPublisherUsingBundle](API_ofxSetPublisherUsingBundle)
* [Advanced API](#AdvancedAPI)
	* [class ofxOscSubscriberManager](#Advanced_ofxOscSubscriberManager)
	* [class ofxOscSubscriber](#Advanced_ofxOscSubscriber)
	* [Legacy style pick up leaked OSC](#Advanced_LegacyStylePickUpLeakedOSCMessage)
	* [How to subscribe ofParameterGroup](#Advanced_how_to_subscribe_ofParameterGroup)
	* [class ofxOscPublisherManager](#Advanced_ofxOscPublisherManager)
	* [class ofxOscPublisher](#Advanced_ofxOscPublisher)

## <a name="API">API</a>

### <a name="API_ofxSubscribeOsc">ofxSubscribeOsc</a>

#### Reference type

* template \<typename T\> void ofxSubscribeOsc(int _port_, const string &_address_, T &_value_);

#### Setter Function / Method

setter function

* template \<typename T, typename R\> void ofxSubscribeOsc(int _port_, const string &_address_, R (*callback)(T &));

setter method

* template \<typename T, typename R, typename C\> void ofxSubscribeOsc(int _port_, const string &_address_, C &_that_, R (C::\*_callback_)(T));
* template \<typename T, typename R, typename C\> void ofxSubscribeOsc(int _port_, const string &_address_, C *_that_, R (C::\*_callback_)(T));

const setter method (if you don't change property)

* template \<typename T, typename R, typename C\> void ofxSubscribeOsc(int _port_, const string &_address_, const C &_that_, R (C::\*_callback_)(T) const);
* template \<typename T, typename R, typename C\> void ofxSubscribeOsc(int _port_, const string &_address_, const C *_that_, R (C::\*_callback_)(T) const);

#### Callback Function / Method receive ofxOscMessage

callback function

* template \<typename R\> void ofxSubscribeOsc(int _port_, const string &_address_, R (\*callback)(ofxOscMessage &));

callback method

* template \<typename C, typename R\> void ofxSubscribeOsc(int _port_, const string &_address_, C &_that_, R (C::\*_callback_)(ofxOscMessage &));
* template \<typename C, typename R\> void ofxSubscribeOsc(int _port_, const string &_address_, C \*_that_, R (C::\*_callback_)(ofxOscMessage &));

const callback method

* template \<typename C, typename R\> void ofxSubscribeOsc(int _port_, const string * &_address_, const C &_that_, R (C::\*_callback_)(ofxOscMessage &) const);
* template \<typename C, typename R\> void ofxSubscribeOsc(int _port_, const string &_address_, const C \*_that_, R (C::\*_callback_)(ofxOscMessage &) const);
	* TODO

<a name="API_lambda_callback">lambda callback</a>

* void ofxSubscribeOsc(int _port_, const string &_address_, std::function<void(ofxOscMessage &)> &_callback_);
	* **(if you use 0.9.0~)**

bind value/function/method to OSC message has _address_ incoming from _port_.

if use function/method, then call `callback(mess)` or `(that.\*callback)(mess)` or `(that->\*callback)(mess)`, `std::function<void(ofxOscMessage &)>` when receive OSC message `mess`.

#### Unscribe

* void ofxUnsubscribeOSC(int _port_, const string &_address_);

unbind OSC message has _address_ incoming from _port_.

* void ofxUnsubscribeOSC(int _port_);

unbind all OSC messages incoming from _port_.

#### See [class ofxOscSubscriberManager](#Advanced_ofxOscSubscriberManager), [class ofxOscSubscriber](#Advanced_ofxOscSubscriber)

### <a name="API_ofxSetLeakedOscPicker">ofxSetLeakedOscPicker</a>

* void ofxSetLeakedOscPicker(int _port_, void (\*_callback_)(ofxOscMessage &))
* template \<typename C, typename R\> void ofxSetLeakedOscPicker(int _port_, C \*_that_, R (C::\*_callback_)(ofxOscMessage &)) 
* template \<typename C, typename R\> void ofxSetLeakedOscPicker(int _port_, C &_that_, R (C::\*_callback_)(ofxOscMessage &)) 
* template \<typename C, typename R\> void ofxSetLeakedOscPicker(int _port_, const C \*_that_, R (C::\*_callback_)(ofxOscMessage &) const) 
* template \<typename C, typename R\> void ofxSetLeakedOscPicker(int _port_, const C &_that_, R (C::\*_callback_)(ofxOscMessage &) const) 

* void ofxSetLeakedOscPicker(int _port_, std::function<void(ofxOscMessage &)> &_callback_)
	* **if you use 0.9.0~**

set callback for port. this callback is kick when receive OSC message has not binded address.

* void ofxRemoveLeakedOscPicker(int _port_)

remove callback.

#### See [Legacy style pick up leaked OSC](#Advanced_LegacyStylePickUpLeakedOSCMessage)

### <a name="API_ofxPublishOsc">ofxPublishOsc</a>

* template \<typename T\> void ofxPublishOsc(const string &_ip_, int _port_, const string &_address_, T &_value_, bool _whenValueIsChanged_ = **true**);
* template \<typename T\> void ofxPublishOsc(const string &_ip_, int _port_, const string &_address_, T (\*_getter_)(), bool _whenValueIsChanged_ = **true**);
* template \<typename T, typename U\> void ofxPublishOsc(const string &_ip_, int _port_, const string &_address_, U \*_that_, T (U::\*_getter_)(), bool _whenValueIsChanged_ = **true**);

publish _value_ / _getter()_ / _(that.\*getter)()_ to OSC message has _address_ to _ip:port_. if _whenValueIsChanged_ is set to **false**, then we send binded value **every frame** after `App::update`.

#### <a name="API_ofxPublishOscIf">ofxPublishOscIf</a>

* template \<...\> void ofxPublishOscIf(bool &_condition_, const string &_ip_, int _port_, const string &_address_, ...);
* template \<...\> void ofxPublishOscIf(bool (*_condition_)(), const string &_ip_, int _port_, const string &_address_, ...);
* template \<..., typename C\> void ofxPublishOscIf(C \*_condition_, bool (C::\*_method_)(), const string &_ip_, int _port_, const string &_address_, ...);
* template \<..., typename C\> void ofxPublishOscIf(C &_condition_, bool (C::\*_method_)(), const string &_ip_, int _port_, const string &_address_, ...);

if _condition_ / _condition()_ / _(condition.*method)()_ is true then publish arguments are same pattern as _ofxPublishOsc_. (only _whenValueChanged_ is eliminated. _whenValueChanged_ is always true.)

* void ofxUnpublishOSC(const string &_ip_, int _port_, const string &_address_);

unpublish OSC message has _address_ is send to _ip:port_.

* void ofxUnpublishOSC(const string &_ip_, int _port_);

unpublish all OSC messages is send to _ip:port_.

#### <a name="API_ofxPublishAsArray">ofxPublishAsArray</a>

* template \<typename T, size_t size\> typename array_type<T, size>::type ofxPublishAsArray(T \*ptr);
	* where `array_type<T, size>::type` is `T (&)[size]`

cast value is typed `T *` -> value is typed `T (&)[size]`

* template \<typename T, size_t size\> typename array_type<T, size>::fun ofxPublishAsArray(T \*(\*getter)());
	* where `array_type<T, size>::fun` is `array_type<T, size>::type (*)()`

cast function returns `T *` -> function returns `T (&)[size]`

* template \<typename T, size_t size\> typename array_method<T, size, U>::method ofxPublishAsArray(T \*(U::\*getter)());
	* where `array_method<T, size, U>::method` is `array_type<T, size>::type (U:*)()`

cast method of `U` returns `T *` -> method of `U` returns `T (&)[size]`

##### Example

```cpp
struct P {
	int *getFooPtr() { return foo; }
private:
	int foo[8];
};

P p;
float *bar = new float[12];

...

ofxPublishOsc("localhost", 9005, "/foo", p, ofxPublishAsArray<int, 8>(&P::getFooPtr));
ofxPublishOsc("localhost", 9005, "/bar", ofxPublishAsArray<float, 12>(bar));
```

#### <a name="API_ofxSetPublisherUsingBundle">ofxSetPublisherUsingBundle</a>

* ofxSetPublisherUsingBundle(bool _bUseBundle_)

set to send messages with bundle

#### See [class ofxOscPublisherManager](#Advanced_ofxOscPublisherManager), [class ofxOscPublisher](#Advanced_ofxOscPublisher)

## <a name="AdvancedAPI">Advanced API</a>

### <a name="Advanced_ofxOscSubscriberManager">class ofxOscSubscriberManager</a>

* static ofxOscSubscriberManager &getSharedInstance();

return sharedInstance.

* static ofxOscSubscriber &getOscSubscriber(int _port_);

return OSC subscriber is binded to _port_.

### <a name="Advanced_ofxOscSubscriber">class ofxOscSubscriber</a>

* template \<typename T\> void subscribe(const string &_address_, T &_value_);
* void subscribe(const string &_address_, void (*callback)(ofxOscMessage &));
* template \<typename T\> void subscribe(const string &_address_, T &_that_, void (T::*_callback_)(ofxOscMessage &));
* template \<typename T\> void subscribe(const string &_address_, T * _that_, void (T::*_callback_)(ofxOscMessage &));

bind value/function/method to OSC message has _address_.

* void unsubscribe(const string &_address_);

unbind OSC message has _address_.

* void unsubscribe();

unbind all OSC messages.

#### <a name="Advanced_how_to_subscribe_ofParameterGroup">How to subscribe ofParameterGroup</a>

to subscribing value into ofParameterGroup, is different to others.

we need key for search parameter from parameter group.

so, we have to add key to first argument of OSC message.

for example, we setup parameter group and parameter like this:

```cpp
ofParameterGroup group;
ofParameter<float> float_x;
ofParameter<float> float_y;
ofParameter<ofColor> color;

...

float_x.setName("x");
float_y.setName("y");
color.setName("c");

group.add(float_y); // float_y has index 0 and name "y"
group.add(float_x); // float_x has index 1 and name "x"
group.add(color); // color has index 2 and name "c"

ofxSubscribeOsc(9005, "/param_group", group);

```

then, we construct message like this:

```cpp
ofxOscMessage mx, my;

mx.setAddress("/param_group");
mx.addArgAsString("x");
mx.addArgAsFloat(1.0f);
sender.sendMessage(mx); // this value set to float_x

my.setAddress("/param_group");
my.addArgAsString("y");
my.addArgAsFloat(2.0f);
sender.sendMessage(my); // this value set to float_y

ofxOscMessage m_0, m_1;

m0.setAddress("/param_group");
m0.addArgAsInt(0);
m0.addArgAsFloat(1.0f);
sender.sendMessage(m0); // !! this value set to float_y

m1.setAddress("/param_group");
m1.addArgAsInt(1);
m1.addArgAsFloat(2.0f);
sender.sendMessage(m1); // !! this value set to float_x 

ofxOscMessage mc;
mc.setAddress("/param_group");
mc.addArgAsString("c");
mc.addArgAsInt(255);
mc.addArgAsInt(0);
mc.addArgAsInt(0);
mc.addArgAsInt(255);
sender.sendMessage(mc); // color will set to red

```

#### <a name="Advanced_setLeakPicker">setLeakPicker</a>

* void setLeakPicker(void (\*_callback_)(ofxOscMessage &));
* template \<typename T\> void setLeakPicker(T \*_that_, void (T::\*_callback_)(ofxOscMessage &));
* template \<typename T\> void setLeakPicker(T &_that_, void (T::\*_callback_)(ofxOscMessage &));

set callback for port. this callback is kick when receive OSC message has not binded address.

* void removeLeakPicker();

remove callback picks leaked message.

#### <a name="Advanced_LegacyStylePickUpLeakedOSCMessage">How to "Legacy style pick up leaked OSC"</a>

if you don't use [setLeakPicker](#Advanced_setLeakPicker) to a `port`, then you can pick leaked messages manually like this:

```cpp
while(ofxGetOscSubScriber(port).hasWaitingLeakedOscMessages()) {
	ofxOscMessage m;
	ofxGetOscSubscriber(9005).getNextLeakedOscMessage(m);
	ofLogNotice() << m.getAddress();
}
```

please note that argument of `getNextLeakedOscMessage` is not as address (i.e. `ofxOscMessage *`). you can pass `ofxOscMessage` and we will receive as `ofxOscMessage &` and set something.
### <a name="Advanced_ofxOscPublisherManager">class ofxOscPublisherManager</a>

* static ofxOscPublisherManager &getSharedInstance();

return sharedInstance.

* static ofxOscPublisher &getOscPublisher(const string &_ip_, int _port_);

return OSC publisher send to _ip:port_.

### <a name="Advanced_ofxOscPublisher">class ofxOscPublisher</a>

* template \<typename T\> void publish(const string &_address_, T &_value_, bool _whenValueIsChanged_ = **true**);
* template \<typename T\> void publish(const string &_address_, T (*_getter_)(), bool _whenValueIsChanged_ = **true**);
* template \<typename T, typename U\> void publish(const string &_address_, U \*_that_, T (U::*_getter_)(), bool _whenValueIsChanged_ = **true**);

publish _value_ / _getter()_ / _(that.*getter)()_ to OSC message has _address_ to _ip:port_. if _whenValueIsChanged_ is set to **false**, then we send binded value **every frame** after `App::update`.

* void unpublish(const string &_address_);

unpublish OSC message has _address_.

* void ofxUnpublishOSC();

unpublish all OSC messages.
