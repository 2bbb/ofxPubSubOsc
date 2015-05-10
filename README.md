# ofxOscSubscriber

subscribe OSC message.

## How to use?

* template \<typename T\> ofxSubscribeOSC(int _port_, const string &_address_, T &value);

bind value to OSC message has _address_ incoming from _port_.


* ofxUnsubscribeOSC(int _port_, const string &_address_);

unbind OSC message has _address_ incoming from _port_.

* ofxUnsubscribeOSC(int _port_);

unbind all OSC messages incoming from _port_.

## Update history

### 2015/05/09 ver 0.01 release

## License

MIT License.

## Author

* ISHII 2bit [bufferRenaiss co., ltd.]
* ishii[at]buffer-renaiss.com

## At the last

Please create new issue, if there is a problem.
And please throw pull request, if you have a cool idea!!