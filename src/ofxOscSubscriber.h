//
//  ofxOscSubscriber.h
//
//  Created by ISHII 2bit on 2015/05/10.
//
//

#pragma once

#include "ofMain.h"
#include "ofxOsc.h"

#include "details/ofxpubsubosc_type_traits.h"

namespace ofx {
    using namespace ofxpubsubosc;
    
    class OscSubscriberManager {
        struct AbstractParameter {
            virtual void read(ofxOscMessage &message) {}
        };
        
        struct SetImplementation {
        protected:
#define define_set_arithmetic(type) \
            inline void set(ofxOscMessage &m, type &v, size_t offset = 0) { \
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
            
            define_set_arithmetic(float);
            define_set_arithmetic(double);
#undef define_set_arithmetic
            
            inline void set(ofxOscMessage &m, string &v, size_t offset = 0) {
                v = m.getArgAsString(offset);
            }
            
            inline void set(ofxOscMessage &m, ofBuffer &v, size_t offset = 0) {
                v = m.getArgAsBlob(offset);
            }
            
            inline void set(ofxOscMessage &m, ofColor &v, size_t offset = 0)      { setColor<unsigned char>(m, v, 255, offset); }
            inline void set(ofxOscMessage &m, ofShortColor &v, size_t offset = 0) { setColor<unsigned short>(m, v, 65535, offset); }
            inline void set(ofxOscMessage &m, ofFloatColor &v, size_t offset = 0) { setColor<float>(m, v, 1.0f, offset); }
            
            template <typename U>
            inline void setColor(ofxOscMessage &m, ofColor_<U> &v, U defaultValue, size_t offset = 0) {
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
            
            inline void set(ofxOscMessage &m, ofVec2f &v, size_t offset = 0)      { setVec<2>(m, v, offset); }
            inline void set(ofxOscMessage &m, ofVec3f &v, size_t offset = 0)      { setVec<3>(m, v, offset); }
            inline void set(ofxOscMessage &m, ofVec4f &v, size_t offset = 0)      { setVec<4>(m, v, offset); }
            inline void set(ofxOscMessage &m, ofQuaternion &v, size_t offset = 0) { setVec<4>(m, v, offset); }
            inline void set(ofxOscMessage &m, ofMatrix3x3 &v, size_t offset = 0)  { setVec<9>(m, v, offset); }
            
            template <size_t n, typename U>
            inline void setVec(ofxOscMessage &m, U &v, size_t offset = 0) {
                for(int i = 0; i < min(static_cast<size_t>(m.getNumArgs() - offset), n); i++) {
                    set(m, v[i], offset + i);
                }
            }
            
            inline void set(ofxOscMessage &m, ofMatrix4x4 &v, size_t offset = 0) {
                for(int j = 0; j < 4; j++) for(int i = 0; i < 4; i++) {
                    set(m, v(i, j), offset + 4 * j + i);
                }
            }
            
            inline void set(ofxOscMessage &m, ofRectangle &v, size_t offset = 0) {
                set(m, v.x,      offset + 0);
                set(m, v.y,      offset + 1);
                set(m, v.width,  offset + 2);
                set(m, v.height, offset + 3);
            }
            
            template <typename U, size_t size>
            inline void set(ofxOscMessage &m, U (&v)[size], size_t offset = 0) {
                for(int i = 0; i < min(size, (m.getNumArgs() - offset) / ofxpubsubosc::type_traits<U>::size); i++) {
                    set(m, v[i], offset + i * ofxpubsubosc::type_traits<U>::size);
                }
            }
            
            template <typename U>
            inline void set(ofxOscMessage &m, vector<U> &v, size_t offset = 0) {
                size_t num = (m.getNumArgs() - offset) / ofxpubsubosc::type_traits<U>::size;
                if(v.size() < num) v.resize(num);
                for(int i = 0; i < min(num, v.size()); i++) {
                    set(m, v[i], offset + i * ofxpubsubosc::type_traits<U>::size);
                }
            }
        };
        
        template <typename T>
        struct Parameter : AbstractParameter, SetImplementation {
            Parameter(T &t) : t(t) {}
            virtual void read(ofxOscMessage &message) { set(message, t); }
        
        private:
            
            T &t;
        };
        
        struct CallbackParameter : AbstractParameter, SetImplementation {
        public:
            typedef void (*Callback)(ofxOscMessage &);
            CallbackParameter(Callback callback)
            : callback(callback) {}
            
            virtual void read(ofxOscMessage &message) { callback(message); }

        private:
            Callback callback;
        };

        template <typename T>
        struct MethodCallbackParameter : AbstractParameter, SetImplementation {
            typedef void (T::*Callback)(ofxOscMessage &);
            MethodCallbackParameter(T &that, Callback callback)
            : that(that), callback(callback) {}
            MethodCallbackParameter(T *that, Callback callback)
            : that(*that), callback(callback) {}
            
            virtual void read(ofxOscMessage &message) { (that.*callback)(message); }
            
        private:
            Callback callback;
            T &that;
        };

        typedef shared_ptr<AbstractParameter> ParameterRef;
        typedef shared_ptr<ofxOscReceiver> OscReceiverRef;
        typedef map<string, ParameterRef> Targets;
    public:
        class OscSubscriber {
        public:
            inline void subscribe(const string &address, ParameterRef ref) {
                Targets::iterator it = targets.find(address);
                if(it == targets.end()) {
                    targets.insert(make_pair(address, ref));
                } else {
                    it->second = ref;
                }
            }
            
            template <typename T>
            inline void subscribe(const string &address, T &value) {
                subscribe(address, ParameterRef(new Parameter<T>(value)));
            }
            
            inline void subscribe(const string &address, void (*callback)(ofxOscMessage &)) {
                subscribe(address, ParameterRef(new CallbackParameter(callback)));
            }
            
            template <typename T>
            inline void subscribe(const string &address, T &that, void (T::*callback)(ofxOscMessage &)) {
                subscribe(address, ParameterRef(new MethodCallbackParameter<T>(that, callback)));
            }
            
            template <typename T>
            inline void subscribe(const string &address, T *that, void (T::*callback)(ofxOscMessage &)) {
                subscribe(address, ParameterRef(new MethodCallbackParameter<T>(that, callback)));
            }
            
            inline void unsubscribe(const string &address) {
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
            
            template <typename T>
            inline void setLeakPicker(T &that, void (T::*callback)(ofxOscMessage &)) {
                setLeakPicker(ParameterRef(new MethodCallbackParameter<T>(that, callback)));
            }
            
            template <typename T>
            inline void setLeakPicker(T *that, void (T::*callback)(ofxOscMessage &)) {
                setLeakPicker(ParameterRef(new MethodCallbackParameter<T>(that, callback)));
            }
            
            inline void removeLeakPicker() {
                leakPicker.reset();
            }
            
            inline bool isSubscribed() const {
                return targets.empty();
            }
            
            inline bool isSubscribed(const string &address) const {
                return isSubscribed() && (targets.find(address) != targets.end());
            }
            
            inline bool isLeakedOscCovered() const {
                return static_cast<bool>(leakPicker);
            }
            
            void clearLeakedOscMessages() {
                queue<ofxOscMessage> empty;
                swap(leakedOscMessages, empty);
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
            
            typedef shared_ptr<OscSubscriber> Ref;
        private:
            OscSubscriber(int port)
            : port(port) {
                receiver.setup(port);
            }
            
            void update() {
                clearLeakedOscMessages();
                ofxOscMessage m;
                while(receiver.hasWaitingMessages()) {
                    receiver.getNextMessage(&m);
                    const string &address = m.getAddress();
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
            queue<ofxOscMessage> leakedOscMessages;
            
            friend class OscSubscriberManager;
        };
        
        static OscSubscriberManager &getSharedInstance() {
            static OscSubscriberManager *sharedInstance = new OscSubscriberManager;
            return *sharedInstance;
        }
        
        static OscSubscriber &getOscSubscriber(int port) {
            OscSubscribers &managers = getSharedInstance().managers;
            if(managers.find(port) == managers.end()) {
                managers.insert(make_pair(port, OscSubscriber::Ref(new OscSubscriber(port))));
            }
            return *(managers[port].get());
        }
        
    private:
        typedef map<int, OscSubscriber::Ref> OscSubscribers;
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
    };
};

typedef ofx::OscSubscriberManager ofxOscSubscriberManager;
typedef ofxOscSubscriberManager::OscSubscriber ofxOscSubscriber;

inline ofxOscSubscriber &ofxGetOscSubscriber(int port) {
    return ofxOscSubscriberManager::getOscSubscriber(port);
}

#pragma mark interface about subscribe

template <typename T>
inline void ofxSubscribeOsc(int port, const string &address, T &value) {
    ofxGetOscSubscriber(port).subscribe(address, value);
}

inline void ofxSubscribeOsc(int port, const string &address, void (*callback)(ofxOscMessage &)) {
    ofxGetOscSubscriber(port).subscribe(address, callback);
}

template <typename T>
inline void ofxSubscribeOsc(int port, const string &address, T &that, void (T::*callback)(ofxOscMessage &)) {
    ofxGetOscSubscriber(port).subscribe(address, that, callback);
}

template <typename T>
inline void ofxSubscribeOsc(int port, const string &address, T *that, void (T::*callback)(ofxOscMessage &)) {
    ofxGetOscSubscriber(port).subscribe(address, that, callback);
}

inline void ofxUnsubscribeOsc(int port, const string &address) {
    ofxGetOscSubscriber(port).unsubscribe(address);
}

inline void ofxUnsubscribeOsc(int port) {
    ofxGetOscSubscriber(port).unsubscribe();
}

#pragma mark interface about leaked osc

inline void ofxSetLeakedOscPicker(int port, void (*callback)(ofxOscMessage &)) {
    ofxGetOscSubscriber(port).setLeakPicker(callback);
}

template <typename T>
inline void ofxSetLeakedOscPicker(int port, T *that, void (T::*callback)(ofxOscMessage &)) {
    ofxGetOscSubscriber(port).setLeakPicker(that, callback);
}

template <typename T>
inline void ofxSetLeakedOscPicker(int port, T &that, void (T::*callback)(ofxOscMessage &)) {
    ofxGetOscSubscriber(port).setLeakPicker(that, callback);
}

inline void ofxRemoveLeakedOscPicker(int port) {
    ofxGetOscSubscriber(port).removeLeakPicker();
}
