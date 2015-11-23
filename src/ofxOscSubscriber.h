//
//  ofxOscSubscriber.h
//
//  Created by ISHII 2bit on 2015/05/10.
//
//

#pragma once

#include "ofMain.h"
#include "ofxOsc.h"

#include "details/ofxpubsubosc_settings.h"
#include "details/ofxpubsubosc_type_traits.h"

namespace ofx {
    using namespace ofxpubsubosc;
    
    class OscSubscriberManager {
    private:
        struct SetImplementation {
        protected:
#define define_set_arithmetic(type) \
            inline void set(ofxOscMessage &m, type &v, std::size_t offset = 0) { \
                if(m.getArgType(offset) == OFXOSC_TYPE_INT32) v = m.getArgAsInt32(offset); \
                else if(m.getArgType(offset) == OFXOSC_TYPE_INT64) v = m.getArgAsInt64(offset); \
                else if(m.getArgType(offset) == OFXOSC_TYPE_FLOAT) v = m.getArgAsFloat(offset); \
            }
            
            define_set_arithmetic(bool);
            define_set_arithmetic(char);
            define_set_arithmetic(unsigned char);
            define_set_arithmetic(short);
            define_set_arithmetic(unsigned short);
            define_set_arithmetic(int);
            define_set_arithmetic(unsigned int);
            define_set_arithmetic(long);
            define_set_arithmetic(unsigned long);
            define_set_arithmetic(long long);
            define_set_arithmetic(unsigned long long);
            
            define_set_arithmetic(float);
            define_set_arithmetic(double);
#undef define_set_arithmetic
            
            inline void set(ofxOscMessage &m, std::string &v, std::size_t offset = 0) {
                v = m.getArgAsString(offset);
            }
            
#if ENABLE_OF_BUFFER
            inline void set(ofxOscMessage &m, ofBuffer &v, std::size_t offset = 0) {
                v = m.getArgAsBlob(offset);
            }
#endif
            
            inline void set(ofxOscMessage &m, ofColor &v, std::size_t offset = 0)      { setColor<unsigned char>(m, v, 255, offset); }
            inline void set(ofxOscMessage &m, ofShortColor &v, std::size_t offset = 0) { setColor<unsigned short>(m, v, 65535, offset); }
            inline void set(ofxOscMessage &m, ofFloatColor &v, std::size_t offset = 0) { setColor<float>(m, v, 1.0f, offset); }
            
            template <typename U>
            inline void setColor(ofxOscMessage &m, ofColor_<U> &v, U defaultValue, std::size_t offset = 0) {
                if(m.getNumArgs() == 1) {
                    set(m, v.r, offset);
                    set(m, v.g, offset);
                    set(m, v.b, offset);
                    v.a = defaultValue;
                } else if(m.getNumArgs() == 3) {
                    set(m, v.r, offset + 0);
                    set(m, v.g, offset + 1);
                    set(m, v.b, offset + 2);
                    v.a = defaultValue;
                } else {
                    set(m, v.r, offset + 0);
                    set(m, v.g, offset + 1);
                    set(m, v.b, offset + 2);
                    set(m, v.a, offset + 3);
                }
            }
            
            inline void set(ofxOscMessage &m, ofVec2f &v, std::size_t offset = 0)      { setVec<2>(m, v, offset); }
            inline void set(ofxOscMessage &m, ofVec3f &v, std::size_t offset = 0)      { setVec<3>(m, v, offset); }
            inline void set(ofxOscMessage &m, ofVec4f &v, std::size_t offset = 0)      { setVec<4>(m, v, offset); }
            inline void set(ofxOscMessage &m, ofQuaternion &v, std::size_t offset = 0) { setVec<4>(m, v, offset); }
            inline void set(ofxOscMessage &m, ofMatrix3x3 &v, std::size_t offset = 0)  { setVec<9>(m, v, offset); }
            
            template <std::size_t n, typename U>
            inline void setVec(ofxOscMessage &m, U &v, std::size_t offset = 0) {
                for(int i = 0; i < min(static_cast<std::size_t>(m.getNumArgs() - offset), n); i++) {
                    set(m, v[i], offset + i);
                }
            }
            
            inline void set(ofxOscMessage &m, ofMatrix4x4 &v, std::size_t offset = 0) {
                for(int j = 0; j < 4; j++) for(int i = 0; i < 4; i++) {
                    set(m, v(i, j), offset + 4 * j + i);
                }
            }
            
            inline void set(ofxOscMessage &m, ofRectangle &v, std::size_t offset = 0) {
                set(m, v.x,      offset + 0);
                set(m, v.y,      offset + 1);
                set(m, v.width,  offset + 2);
                set(m, v.height, offset + 3);
            }
            
            template <typename U, std::size_t size>
            inline void set(ofxOscMessage &m, U (&v)[size], std::size_t offset = 0) {
                for(int i = 0; i < min(size, (m.getNumArgs() - offset) / ofxpubsubosc::type_traits<U>::size); i++) {
                    set(m, v[i], offset + i * ofxpubsubosc::type_traits<U>::size);
                }
            }
            
            template <typename U>
            inline void set(ofxOscMessage &m, vector<U> &v, std::size_t offset = 0) {
                std::size_t num = (m.getNumArgs() - offset) / ofxpubsubosc::type_traits<U>::size;
                if(v.size() != num) v.resize(num);
                for(int i = 0; i < num; i++) {
                    set(m, v[i], offset + i * ofxpubsubosc::type_traits<U>::size);
                }
            }
            
#pragma mark ofParameter<T> / ofParameterGroup
            
            template <typename U>
            inline void set(ofxOscMessage &m, ofParameter<U> &p, std::size_t offset = 0) {
                U u;
                set(m, u, offset);
                p.set(u);
            }

            inline void set(ofxOscMessage &m, ofAbstractParameter &p, std::size_t offset = 0) {
#define type_convert(type_) if(p.type() == typeid(ofParameter<type_>).name()) { set(m, p.cast<type_>(), offset); return; }
                type_convert(float);
                type_convert(double);
                type_convert(int);
                type_convert(unsigned int);
                type_convert(long);
                type_convert(unsigned long);
                type_convert(ofColor);
                type_convert(ofRectangle);
                type_convert(ofVec2f);
                type_convert(ofVec3f);
                type_convert(ofVec4f);
                type_convert(ofQuaternion);
                type_convert(ofMatrix3x3);
                type_convert(ofMatrix4x4);
                
                type_convert(ofFloatColor);
                type_convert(ofShortColor);
                
                type_convert(bool);
                type_convert(char);
                type_convert(unsigned char);
                type_convert(short);
                type_convert(unsigned short);
                type_convert(long long);
                type_convert(unsigned long long);
                
#if ENABLE_OF_BUFFER
                type_convert(ofBuffer);
#endif
                ofLogWarning("ofxOscSubscriber") << "ofAbstractParameter: Unknown type \"" << p.type() << "\", bind to " << m.getAddress() << ". we ignored.";
#undef type_convert
            }
            
            inline void set(ofxOscMessage &m, ofParameterGroup &pg, std::size_t offset = 0) {
                if(m.getArgType(0) == OFXOSC_TYPE_INT32) {
                    if(pg.size() <= m.getArgAsInt32(0)) {
                        ofLogWarning("ofxOscSubscriber") << "ofAbstractParameterGroup: not contain index \"" << m.getArgAsInt32(0) << "\"";
                        return;
                    }
                    set(m, pg.get(m.getArgAsInt32(0)), offset + 1);
                } else if(m.getArgType(0) == OFXOSC_TYPE_INT64) {
                    if(pg.size() <= m.getArgAsInt64(0)) {
                        ofLogWarning("ofxOscSubscriber") << "ofAbstractParameterGroup: not contain index \"" << m.getArgAsInt64(0) << "\"";
                        return;
                    }
                    set(m, pg.get(m.getArgAsInt64(0)), offset + 1);
                } else if(m.getArgType(0) == OFXOSC_TYPE_STRING) {
                    if(!pg.contains(m.getArgAsString(0))) {
                        ofLogWarning("ofxOscSubscriber") << "ofAbstractParameterGroup: not contain key \"" << m.getArgAsString(0) << "\"";
                        return;
                    }
                    set(m, pg.get(m.getArgAsString(0)), offset + 1);
                }
            }
        };
        
#pragma mark Parameter
        
        struct AbstractParameter {
            virtual void read(ofxOscMessage &message) = 0;
        };
        
        template <typename T>
        struct Parameter : AbstractParameter, SetImplementation {
            Parameter(T &t) : t(t) {}
            virtual void read(ofxOscMessage &message) { set(message, t); }
        
        private:
            
            T &t;
        };
        
        template <typename T, typename R>
        struct SetterFunctionParameter : AbstractParameter, SetImplementation {
            SetterFunctionParameter(R (*setter)(T)) : setter(setter) {};
            virtual void read(ofxOscMessage &message) {
                typename remove_const_reference<T>::type t;
                set(message, t);
                setter(t);
            }
            
        private:
            R (*setter)(T);
        };
        
        template <typename T, typename C, typename R>
        struct SetterMethodParameter : AbstractParameter, SetImplementation {
            SetterMethodParameter(C &that, R (C::*setter)(T))
            : that(that)
            , setter(setter) {};
            
            virtual void read(ofxOscMessage &message) {
                typename remove_const_reference<T>::type t;
                set(message, t);
                (that.*setter)(t);
            }
            
        private:
            C &that;
            R (C::*setter)(T);
        };

        template <typename T, typename C, typename R>
        struct ConstSetterMethodParameter : AbstractParameter, SetImplementation {
            ConstSetterMethodParameter(const C &that, R (C::*setter)(T) const)
            : that(that)
            , setter(setter) {};
            
            virtual void read(ofxOscMessage &message) {
                typename remove_const_reference<T>::type t;
                set(message, t);
                (that.*setter)(t);
            }
            
        private:
            const C &that;
            R (C::*setter)(T) const;
        };

#pragma mark callbacks
        
        struct CallbackParameter : AbstractParameter, SetImplementation {
        public:
            typedef void (*Callback)(ofxOscMessage &);
            CallbackParameter(Callback callback)
            : callback(callback) {}
            
            virtual void read(ofxOscMessage &message) { callback(message); }

        private:
            Callback callback;
        };

        template <typename C, typename R>
        struct MethodCallbackParameter : AbstractParameter, SetImplementation {
            typedef R (C::*Callback)(ofxOscMessage &);
            MethodCallbackParameter(C &that, Callback callback)
            : that(that), callback(callback) {}
            
            virtual void read(ofxOscMessage &message) { (that.*callback)(message); }
            
        private:
            Callback callback;
            C &that;
        };

        template <typename C, typename R>
        struct ConstMethodCallbackParameter : AbstractParameter, SetImplementation {
            typedef R (C::*Callback)(ofxOscMessage &) const;
            ConstMethodCallbackParameter(const C &that, Callback callback)
            : that(that), callback(callback) {}
            
            virtual void read(ofxOscMessage &message) { (that.*callback)(message); }
            
        private:
            Callback callback;
            const C &that;
        };
        
#if ENABLE_FUNCTIONAL
        struct LambdaCallbackParameter : AbstractParameter, SetImplementation {
        public:
            typedef std::function<void(ofxOscMessage &)> Callback;
            LambdaCallbackParameter(Callback &callback)
            : callback(callback) {}
            
            virtual void read(ofxOscMessage &message) { callback(message); }
            
        private:
            Callback callback;
        };
#endif
        typedef std::shared_ptr<AbstractParameter> ParameterRef;
        typedef std::shared_ptr<ofxOscReceiver> OscReceiverRef;
        typedef std::map<std::string, ParameterRef> Targets;
        
    public:
        class OscSubscriber {
        public:
            inline void subscribe(const std::string &address, ParameterRef ref) {
                Targets::iterator it = targets.find(address);
                if(it == targets.end()) {
                    targets.insert(std::make_pair(address, ref));
                } else {
                    it->second = ref;
                }
            }
            
            template <typename T>
            inline void subscribe(const std::string &address, T &value) {
                subscribe(address, ParameterRef(new Parameter<T>(value)));
            }
            
            template <typename T, typename R>
            inline typename is_not_ofxoscmessage<R>::type subscribe(const std::string &address, R (*setter)(T)) {
                subscribe(address, ParameterRef(new SetterFunctionParameter<T, R>(setter)));
            }
            
            template <typename T, typename C, typename R>
            inline typename is_not_ofxoscmessage<R>::type subscribe(const std::string &address, C &that, R (C::*setter)(T)) {
                subscribe(address, ParameterRef(new SetterMethodParameter<T, C, R>(that, setter)));
            }
            
            template <typename T, typename C, typename R>
            inline typename is_not_ofxoscmessage<R>::type subscribe(const std::string &address, const C &that, R (C::*setter)(T) const) {
                subscribe(address, ParameterRef(new ConstSetterMethodParameter<T, C, R>(that, setter)));
            }
            
            template <typename T, typename C, typename R>
            inline typename is_not_ofxoscmessage<R>::type subscribe(const std::string &address, C *that, R (C::*setter)(T)) {
                subscribe(address, ParameterRef(new SetterMethodParameter<T, C, R>(*that, setter)));
            }

            inline void subscribe(const std::string &address, void (*callback)(ofxOscMessage &)) {
                subscribe(address, ParameterRef(new CallbackParameter(callback)));
            }
            
            template <typename C, typename R>
            inline void subscribe(const std::string &address, C &that, R (C::*callback)(ofxOscMessage &)) {
                subscribe(address, ParameterRef(new MethodCallbackParameter<C, R>(that, callback)));
            }
            
#if ENABLE_FUNCTIONAL
            inline void subscribe(const std::string &address, std::function<void(ofxOscMessage &)> callback) {
                subscribe(address, ParameterRef(new LambdaCallbackParameter(callback)));
            }
#endif
            inline void unsubscribe(const std::string &address) {
                targets.erase(address);
            }
            
            inline void unsubscribe() {
                targets.clear();
            }
            
            inline void setLeakPicker(ParameterRef ref) {
                leakPicker = ref;
            }
            
            inline void setLeakPicker(void (*callback)(ofxOscMessage &)) {
                setLeakPicker(ParameterRef(new CallbackParameter(callback)));
            }
            
            template <typename C, typename R>
            inline void setLeakPicker(C &that, R (C::*callback)(ofxOscMessage &)) {
                setLeakPicker(ParameterRef(new MethodCallbackParameter<C, R>(that, callback)));
            }
            
            template <typename C, typename R>
            inline void setLeakPicker(const C &that, R (C::*callback)(ofxOscMessage &) const) {
                setLeakPicker(ParameterRef(new ConstMethodCallbackParameter<C, R>(that, callback)));
            }
            
#if ENABLE_FUNCTIONAL
            inline void setLeakPicker(std::function<void(ofxOscMessage &)> &callback) {
                setLeakPicker(ParameterRef(new LambdaCallbackParameter(callback)));
            }
#endif
            
            inline void removeLeakPicker() {
                leakPicker.reset();
            }
            
            inline bool isSubscribed() const {
                return targets.empty();
            }
            
            inline bool isSubscribed(const std::string &address) const {
                return isSubscribed() && (targets.find(address) != targets.end());
            }
            
            inline bool isLeakedOscCovered() const {
                return static_cast<bool>(leakPicker);
            }
            
            void clearLeakedOscMessages() {
                std::queue<ofxOscMessage> empty;
                std::swap(leakedOscMessages, empty);
            }
            
            inline bool hasWaitingLeakedOscMessages() const {
                return !static_cast<bool>(leakPicker) && !leakedOscMessages.empty();
            }
            
            inline bool getNextLeakedOscMessage(ofxOscMessage &m) {
                if(hasWaitingLeakedOscMessages()) {
                    m.copy(leakedOscMessages.front());
                    leakedOscMessages.pop();
                    return true;
                } else {
                    return false;
                }
            }
            
            void notify(ofxOscMessage & m) {
                const std::string &address = m.getAddress();
                if(targets.find(address) != targets.end()) {
                    targets[address]->read(m);
                }
            }
            
            typedef  std::shared_ptr<OscSubscriber> Ref;
        private:
            OscSubscriber(int port)
            : port(port) {
                receiver.setup(port);
            }
            
            void update() {
                clearLeakedOscMessages();
                ofxOscMessage m;
                while(receiver.hasWaitingMessages()) {
#if OF_VERSION_MINOR < 9
                    receiver.getNextMessage(&m);
#else
                    receiver.getNextMessage(m);
#endif
                    const std::string &address = m.getAddress();
                    if(targets.find(address) != targets.end()) {
                        targets[address]->read(m);
                    } else {
                        if(leakPicker) {
                            leakPicker->read(m);
                        } else {
                            leakedOscMessages.push(m);
                        }
                    }
                }
            }
            
            int port;
            ofxOscReceiver receiver;
            Targets targets;
            ParameterRef leakPicker;
            std::queue<ofxOscMessage> leakedOscMessages;
            
            friend class OscSubscriberManager;
        };
        
        static OscSubscriberManager &getSharedInstance() {
            static OscSubscriberManager *sharedInstance = new OscSubscriberManager;
            return *sharedInstance;
        }
        
        static OscSubscriber &getOscSubscriber(int port) {
            OscSubscribers &managers = getSharedInstance().managers;
            if(managers.find(port) == managers.end()) {
                managers.insert(std::make_pair(port, OscSubscriber::Ref(new OscSubscriber(port))));
            }
            return *(managers[port].get());
        }
        
    private:
        typedef std::map<int, OscSubscriber::Ref> OscSubscribers;
        void update(ofEventArgs &args) {
            for(OscSubscribers::iterator it = managers.begin(); it != managers.end(); ++it) {
                it->second->update();
            }
        }
        
        OscSubscriberManager() {
            ofAddListener(ofEvents().update, this, &OscSubscriberManager::update, OF_EVENT_ORDER_BEFORE_APP);
        }
        
        virtual ~OscSubscriberManager() {
            ofRemoveListener(ofEvents().update, this, &OscSubscriberManager::update, OF_EVENT_ORDER_BEFORE_APP);
        }
        OscSubscribers managers;
        
#pragma mark iterator
    public:
        typedef OscSubscribers::iterator iterator;
        typedef OscSubscribers::const_iterator const_iterator;
        typedef OscSubscribers::reverse_iterator reverse_iterator;
        typedef OscSubscribers::const_reverse_iterator const_reverse_iterator;
        
        iterator begin() { return managers.begin(); }
        iterator end() { return managers.end(); }
        
        const_iterator begin() const { return managers.cbegin(); }
        const_iterator end() const { return managers.cend(); }
        const_iterator cbegin() const {
#if __cplusplus <= 199711L
            return managers.begin();
#else
            return managers.cbegin();
#endif
        }
        const_iterator cend() const {
#if __cplusplus <= 199711L
            return managers.end();
#else
            return managers.cend();
#endif
        }
        
        reverse_iterator rbegin() { return managers.rbegin(); }
        reverse_iterator rend() { return managers.rend(); }
        
        const_reverse_iterator rbegin() const { return managers.crbegin(); }
        const_reverse_iterator rend() const { return managers.crend(); }
        const_reverse_iterator crbegin() const {
#if __cplusplus <= 199711L
            return managers.rbegin();
#else
            return managers.crbegin();
#endif
        }
        const_reverse_iterator crend() const {
#if __cplusplus <= 199711L
            return managers.rend();
#else
            return managers.crend();
#endif
        }
    };
};

typedef ofx::OscSubscriberManager ofxOscSubscriberManager;
typedef ofxOscSubscriberManager::OscSubscriber ofxOscSubscriber;

/// \brief get a OscSubscriberManager.
/// \returns ofxOscSubscriberManager

inline ofxOscSubscriberManager &ofxGetOscSubscriberManager() {
    return ofxOscSubscriberManager::getSharedInstance();
}

/// \brief get a OscSubscriber.
/// \param port binded port is typed int
/// \returns ofxOscSubscriber binded to port

inline ofxOscSubscriber &ofxGetOscSubscriber(int port) {
    return ofxOscSubscriberManager::getOscSubscriber(port);
}

#pragma mark interface about subscribe

/// \name ofxSubscribeOsc
/// \{

/// \brief bind a referece of value to the argument(s) of OSC messages with an address pattern _address_ incoming to _port_.
/// template parameter T is suggested by value
/// \param port binded port is typed int
/// \param address osc address is typed const std::string &
/// \param value reference of value is typed T &
/// \returns void

#pragma mark reference

template <typename T>
inline void ofxSubscribeOsc(int port, const std::string &address, T &value) {
    ofxGetOscSubscriber(port).subscribe(address, value);
}

#pragma mark setter function/method

template <typename T, typename R>
inline typename ofxpubsubosc::is_not_ofxoscmessage<T>::type ofxSubscribeOsc(int port, const std::string &address, R (*callback)(T)) {
    ofxGetOscSubscriber(port).subscribe(address, callback);
}

template <typename T, typename C, typename R>
inline typename ofxpubsubosc::is_not_ofxoscmessage<T>::type ofxSubscribeOsc(int port, const std::string &address, C &that, R (C::*callback)(T)) {
    ofxGetOscSubscriber(port).subscribe(address, that, callback);
}

template <typename T, typename C, typename R>
inline typename ofxpubsubosc::is_not_ofxoscmessage<T>::type ofxSubscribeOsc(int port, const std::string &address, C *that, R (C::*callback)(T)) {
    ofxGetOscSubscriber(port).subscribe(address, *that, callback);
}

template <typename T, typename C, typename R>
inline typename ofxpubsubosc::is_not_ofxoscmessage<T>::type ofxSubscribeOsc(int port, const std::string &address, const C &that, R (C::*callback)(T) const) {
    ofxGetOscSubscriber(port).subscribe(address, that, callback);
}

template <typename T, typename C, typename R>
inline typename ofxpubsubosc::is_not_ofxoscmessage<T>::type ofxSubscribeOsc(int port, const std::string &address, const C * const that, R (C::*callback)(T) const) {
    ofxGetOscSubscriber(port).subscribe(address, *that, callback);
}

#pragma mark callback function/method

/// \brief bind a callback to the OSC messages with an address pattern _address_ incoming to _port_.
/// \param port binded port is typed int
/// \param address osc address is typed const std::string &
/// \param callback is kicked when receive a message to address
/// \returns void

inline void ofxSubscribeOsc(int port, const std::string &address, void (*callback)(ofxOscMessage &)) {
    ofxGetOscSubscriber(port).subscribe(address, callback);
}

/// \brief bind a callback to the OSC messages with an address pattern _address_ incoming to _port_.
/// template parameter C is suggested by that & callback
/// \param port binded port is typed int
/// \param address osc address is typed const std::string &
/// \param that this object is typed T&, will bind with next argument of parameter method. is called as (that.*getter)(message) when receive a message.
/// \param callback has argument ofxOscMessage &
/// \returns void

template <typename C, typename R>
inline void ofxSubscribeOsc(int port, const std::string &address, C &that, R (C::*callback)(ofxOscMessage &)) {
    ofxGetOscSubscriber(port).subscribe(address, that, callback);
}

/// \brief bind a callback to the OSC messages with an address pattern _address_ incoming to _port_.
/// template parameter C is suggested by that & callback
/// \param port binded port is typed int
/// \param address osc address is typed const std::string &
/// \param that this object is typed T*, will bind with next argument of parameter method. is called as (that->*getter)(message) when receive a message.
/// \param callback has argument ofxOscMessage &
/// \returns void

template <typename C, typename R>
inline void ofxSubscribeOsc(int port, const std::string &address, C *that, R (C::*callback)(ofxOscMessage &)) {
    ofxGetOscSubscriber(port).subscribe(address, *that, callback);
}

/// \brief bind a callback to the OSC messages with an address pattern _address_ incoming to _port_.
/// template parameter C is suggested by that & callback
/// \param port binded port is typed int
/// \param address osc address is typed const std::string &
/// \param that this object is typed T&, will bind with next argument of parameter method. is called as (that.*getter)(message) when receive a message.
/// \param callback has argument ofxOscMessage &
/// \returns void

template <typename C, typename R>
inline void ofxSubscribeOsc(int port, const std::string &address, const C &that, R (C::*callback)(ofxOscMessage &) const) {
    ofxGetOscSubscriber(port).subscribe(address, that, callback);
}

/// \brief bind a callback to the OSC messages with an address pattern _address_ incoming to _port_.
/// template parameter C is suggested by that & callback
/// \param port binded port is typed int
/// \param address osc address is typed const std::string &
/// \param that this object is typed T*, will bind with next argument of parameter method. is called as (that->*getter)(message) when receive a message.
/// \param callback has argument ofxOscMessage &
/// \returns void

template <typename C, typename R>
inline void ofxSubscribeOsc(int port, const std::string &address, const C *that, R (C::*callback)(ofxOscMessage &) const) {
    ofxGetOscSubscriber(port).subscribe(address, *that, callback);
}

#if ENABLE_FUNCTIONAL
template <typename C, typename R>
inline void ofxSubscribeOsc(int port, const std::string &address, std::function<void(ofxOscMessage &)> &callback) {
    ofxGetOscSubscriber(port).subscribe(address, callback);
}
#endif

/// \}

#pragma mark unsubscribe

/// \name ofxUnsubscribeOsc
/// \{

/// \brief unbind from OSC messages with an address pattern _address_ incoming to _port_.
/// \param port binded port is typed int
/// \param address osc address is typed const std::string &
/// \returns void

inline void ofxUnsubscribeOsc(int port, const std::string &address) {
    ofxGetOscSubscriber(port).unsubscribe(address);
}

/// \brief unbind from OSC messages with any address patterns incoming to _port_.
/// \param port binded port is typed int
/// \returns void

inline void ofxUnsubscribeOsc(int port) {
    ofxGetOscSubscriber(port).unsubscribe();
}

/// \brief unbind from all OSC messages.
/// \returns void

inline void ofxUnsubscribeOsc() {
    ofxOscSubscriberManager &manager = ofxGetOscSubscriberManager();
    ofxOscSubscriberManager::iterator it  = manager.begin(),
                                      end = manager.end();
    for(; it != end; it++) {
        it->second->unsubscribe();
    }
}

/// \}

#pragma mark notify messages manually

inline void ofxNotifyToSubscribedOsc(int port, ofxOscMessage &m) {
    ofxGetOscSubscriber(port).notify(m);
}

inline void ofxNotifyToSubscribedOsc(ofxOscMessage &m) {
    ofxOscSubscriberManager &manager = ofxGetOscSubscriberManager();
    ofxOscSubscriberManager::iterator it  = manager.begin(),
                                      end = manager.end();
    for(; it != end; it++) {
        it->second->notify(m);
    }
}

#pragma mark interface about leaked osc

/// \name ofxSetLeakedOscPicker
/// \{

/// \brief bind a callback to the OSC messages with are not match other patterns incoming to port.
/// \param port binded port is typed int
/// \callback is kicked when receive a leaked addresses
/// \returns void

template <typename R>
inline void ofxSetLeakedOscPicker(int port, R (*callback)(ofxOscMessage &)) {
    ofxGetOscSubscriber(port).setLeakPicker(callback);
}

/// \brief bind a callback to the OSC messages with are not match other patterns incoming to port.
/// \param port binded port is typed int
/// \param that this object is typed C&, will bind with next argument of parameter method. is called as (that.*getter)(message) when receive a message.
/// \callback is kicked when receive a leaked addresses
/// \returns void

template <typename C, typename R>
inline void ofxSetLeakedOscPicker(int port, C &that, R (C::*callback)(ofxOscMessage &)) {
    ofxGetOscSubscriber(port).setLeakPicker(that, callback);
}

/// \brief bind a callback to the OSC messages with are not match other patterns incoming to port.
/// \param port binded port is typed int
/// \param that this object is typed C*, will bind with next argument of parameter method. is called as (that->*getter)(message) when receive a message.
/// \callback is kicked when receive a leaked addresses
/// \returns void

template <typename C, typename R>
inline void ofxSetLeakedOscPicker(int port, C *that, R (C::*callback)(ofxOscMessage &)) {
    ofxGetOscSubscriber(port).setLeakPicker(*that, callback);
}

/// \brief bind a callback to the OSC messages with are not match other patterns incoming to port.
/// \param port binded port is typed int
/// \param that this object is typed C&, will bind with next argument of parameter method. is called as (that.*getter)(message) when receive a message.
/// \callback is kicked when receive a leaked addresses
/// \returns void

template <typename C, typename R>
inline void ofxSetLeakedOscPicker(int port, const C &that, R (C::*callback)(ofxOscMessage &) const) {
    ofxGetOscSubscriber(port).setLeakPicker(that, callback);
}

/// \brief bind a callback to the OSC messages with are not match other patterns incoming to port.
/// \param port binded port is typed int
/// \param that this object is typed C*, will bind with next argument of parameter method. is called as (that->*getter)(message) when receive a message.
/// \callback is kicked when receive a leaked addresses
/// \returns void

template <typename C, typename R>
inline void ofxSetLeakedOscPicker(int port, const C *that, R (C::*callback)(ofxOscMessage &) const) {
    ofxGetOscSubscriber(port).setLeakPicker(*that, callback);
}

#if ENABLE_FUNCTIONAL
inline void ofxSetLeakedOscPicker(int port, std::function<void(ofxOscMessage &)> &callback) {
    ofxGetOscSubscriber(port).setLeakPicker(callback);
}
#endif

/// \brief remove a callback receives messages has leaked patterns incoming to port.
/// \param port binded port is typed int
/// \returns void

inline void ofxRemoveLeakedOscPicker(int port) {
    ofxGetOscSubscriber(port).removeLeakPicker();
}

inline void ofxRemoveLeakedOscPicker() {
    ofxOscSubscriberManager &manager = ofxGetOscSubscriberManager();
    ofxOscSubscriberManager::iterator it  = manager.begin(),
                                      end = manager.end();
    for(; it != end; it++) {
        it->second->removeLeakPicker();
    }
}

/// \}
