# ofxPubSubOsc API Reference

## TOC

* [API](#API)
	* [ofxSubscribeOsc](#API_ofxSubscribeOsc)
	* [ofxSetLeakedOscPicker](#API_ofxSetLeakedOscPicker)
	* [ofxPublishOsc](#API_ofxPublishOsc)
* [Advanced API](#AdvancedAPI)
	* [class ofxOscSubscriberManager](#Advanced_ofxOscSubscriberManager)
	* [class ofxOscSubscriber](#Advanced_ofxOscSubscriber)
	* [Legacy style pick up leaked OSC](#Advanced_LegacyStylePickUpLeakedOSCMessage)
	* [class ofxOscPublisherManager](#Advanced_ofxOscPublisherManager)
	* [class ofxOscPublisher](#Advanced_ofxOscPublisher)

## <a name="API">API</a>

### <a name="API_ofxSubscribeOsc">ofxSubscribeOsc</a>

* template \<typename T\> void ofxSubscribeOsc(int _port_, const string &_address_, T &_value_);
* void ofxSubscribeOsc(int _port_, const string &_address_, void (*callback)(ofxOscMessage &));
* template \<typename T\> void ofxSubscribeOsc(int _port_, const string &_address_, T &_that_, void (T::*_callback_)(ofxOscMessage &));
* template \<typename T\> void ofxSubscribeOsc(int _port_, const string &_address_, T * _that_, void (T::*_callback_)(ofxOscMessage &));

bind value/function/method to OSC message has _address_ incoming from _port_.

if use function/method, then call `callback(mess)` or `(that.*callback)(mess)` when receive OSC message `mess`.

* void ofxUnsubscribeOSC(int _port_, const string &_address_);

unbind OSC message has _address_ incoming from _port_.

* void ofxUnsubscribeOSC(int _port_);

unbind all OSC messages incoming from _port_.

#### See [class ofxOscSubscriberManager](#Advanced_ofxOscSubscriberManager), [class ofxOscSubscriber](#Advanced_ofxOscSubscriber)

### <a name="API_ofxSetLeakedOscPicker">ofxSetLeakedOscPicker</a>

* void ofxSetLeakedOscPicker(int _port_, void (\*_callback_)(ofxOscMessage &))
* template \<typename T\> void ofxSetLeakedOscPicker(int _port_, T \*_that_, void (T::\*_callback_)(ofxOscMessage &)) 
* template \<typename T\> void ofxSetLeakedOscPicker(int _port_, T &_that_, void (T::\*_callback_)(ofxOscMessage &)) 

set callback for port. this callback is kick when receive OSC message has not binded address.

* void ofxRemoveLeakedOscPicker(int _port_)

remove callback.

#### See [Legacy style pick up leaked OSC](#Advanced_LegacyStylePickUpLeakedOSCMessage)

### <a name="API_ofxPublishOsc">ofxPublishOsc</a>

* template \<typename T\> void ofxPublishOsc(const string &_ip_, int _port_, const string &_address_, T &_value_, bool _whenValueIsChanged_ = **true**);
* template \<typename T\> void ofxPublishOsc(const string &_ip_, int _port_, const string &_address_, T (*_getter_)(), bool _whenValueIsChanged_ = **true**);
* template \<typename T, typename U\> void ofxPublishOsc(const string &_ip_, int _port_, const string &_address_, U \*_that_, T (U::*_getter_)(), bool _whenValueIsChanged_ = **true**);

publish _value_ / _getter()_ / _(that.*getter)()_ to OSC message has _address_ to _ip:port_. if _whenValueIsChanged_ is set to **false**, then we send binded value **every frame** after `App::update`.


* void ofxUnpublishOSC(const string &_ip_, int _port_, const string &_address_);

unpublish OSC message has _address_ is send to _ip:port_.

* void ofxUnpublishOSC(const string &_ip_, int _port_);

unpublish all OSC messages is send to _ip:port_.

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

* void setLeakPicker(void (\*_callback_)(ofxOscMessage &));
* template \<typename T\> void setLeakPicker(T \*_that_, void (T::\*_callback_)(ofxOscMessage &));
* template \<typename T\> void setLeakPicker(T &_that_, void (T::\*_callback_)(ofxOscMessage &));

set callback for port. this callback is kick when receive OSC message has not binded address.

* void removeLeakPicker();

remove callback picks leaked message.

#### <a name="Advanced_LegacyStylePickUpLeakedOSCMessage">How to "Legacy style pick up leaked OSC"</a>

TODO

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
