# ofxPubSubOsc Update History

## 2016/01/25 [ver 0.2.2](../../releases/tag/v0_2_2), [ver 0.1.3](../../releases/tag/v0_1_3) release

* bugfix: about `ofQuaternion`'s `operator>>`, `operator<<` (issued by [musiko](https://github.com/musiko). thanks!!!)

## 2016/01/04 [ver 0.2.1](../../releases/tag/v0_2_1) release

* **critical bugfix**, hehe
* enable subscribe lambda has argument is not ofxOscMessage.
* enable subscribe method has no argument
* update exmaples (xcodeproj) for oF0.9.0
* some cleaning source and doxygen

## 2016/01/02 [ver 0.2.0](../../releases/tag/v0_2_0) release *(this version is broken)*

* *after this release, we will only test on oF0.9.0~*
* add iterators to [ofxOscSubscriberManager](API_Reference.md#Advanced_ofxOscSubscriberManager)
* add iterators to [ofxOscPublisherManager](API_Reference.md#Advanced_ofxOscPublisherManager)
* add all port operation to ofxUnsubscribeOsc, ofxNotifyToSubscribedOsc, ofxRemoveLeakedOscPicker
* add ofxSetLeakedOscPickerAll
* add ofxSubscribeOsc with `std::initializer_list<int> port` and `std::initializer_list<std::string> addresses`
* add iterators to [ofxOscPublisherManager](API_Reference.md#Advanced_ofxOscPublisherManager)
* add all port operation to ofxUnpublishOsc, ofxUnregisterPublishingOsc
* add feature publishing r-value. (i.e., you can do `ofxPublishOsc(host, port, "/bar", "value!!")`)
* add useful macro `SubscribeOsc(port, name)` is same as `ofxSubscribeOsc(port, "/name", name)` (porposed by [hanasaan](https://github.com/hanasaan). thanks!!)
* add `std::` prefix
* cleaning up conditional macro about oF0.8.x
* some bugfix around lambda
* TODO: update some API Documentations

## 2016/01/02 [ver 0.1.2](../../releases/tag/v0_1_2) release

* this is *final update added new feature, with oF0.8.4 support*
* after this release, "ver 0.1.x will only bugfie about supporting oF0.8.4
* add new feature ofxNotifyToSubscribedOsc (proposed by [satcy](http://github.com/satcy/). thanks!!)
* some bugfix

## 2015/09/17 [ver 0.1.1](../../releases/tag/v0_1_1) release

* support [ofParameter](#SupportedTypes_ofParameter)
* support [ofParameterGroup](API_Reference.md#Advanced_how_to_subscribe_ofParameterGroup) for ofxSubscirbeOsc
* add [ofxRegisterPublishingOsc](#ofxRegisterPublishingOsc)
* add [ofxSetPublisherUsingBundle](API_Reference.md#ofxSetPublisherUsingBundle)
* support 0.8.1 (not support ofBuffer as blob)
* update README and [API_Reference.md](API_Reference.md)

## 2015/08/31 [ver 0.1.0](../../releases/tag/v0_1_0) release

* add [lambda function callback](API_Reference.md#API_lambda_callback) for ofxSubscirbeOsc
* update README and [API_Reference.md](API_Reference.md)
* some bugfix

## 2015/06/10 [ver 0.0.8](../../releases/tag/v0_0_8) release

* add [ofxPublishAsArray](API_Reference.md#API_ofxPublishAsArray)
* enable to use in publish `T (U::*)() const`
* update README and [API_Reference.md](API_Reference.md)
* add [Legacy style pick up leaked OSC](API_Reference.md#Advanced_LegacyStylePickUpLeakedOSCMessage)
* add some doxygen texts
* some bugfix

## 2015/05/19 [ver 0.0.7](../../releases/tag/v0_0_7) release

* add [ofxPublishOscIf](API_Reference.md#API_ofxPublishOscIf)
* some bugfix

## 2015/05/17 [ver 0.0.6](../../releases/tag/v0_0_6) release

* add publish ofBuffer as blob
* some bugfix
* big change on inner class structure

## 2015/05/15 [ver 0.0.5](../../releases/tag/v0_0_5) release

* add system about [pick up leaked OSC messages](API_Reference.md#API_ofxSetLeakedOscPicker)
* some bugfix
* add examples

## 2015/05/11 [ver 0.0.4](../../releases/tag/v0_0_4) release

* support ofMatrix3x3/4x4, ofQuaternion, ofBuffer (only subscribe now), vector<ofXXX>, ofXXX[]
* bugfix for Visual Studio

## 2015/05/11 [ver 0.0.3](../../releases/tag/v0_0_3) release

* add subscribe with callback

## 2015/05/11 [ver 0.0.2](../../releases/tag/v0_0_2) release

* rename from ofxOscSubscriber
* add ofxOscPublisher

## 2015/05/09 [ver 0.0.1](../../releases/tag/v0_0_1) release

* initial
