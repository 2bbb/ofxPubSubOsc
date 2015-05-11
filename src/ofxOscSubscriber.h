//
//  ofxOscSubscriber.h
//
//  Created by ISHII 2bit on 2015/05/10.
//
//

#pragma once

#include "ofMain.h"
#include "ofxOsc.h"

namespace ofx {
    class OscSubscriber {
        struct AbstractParameter {
            virtual void set(ofxOscMessage &message) {}
        };
        
        template <typename T>
        struct Parameter : AbstractParameter {
            Parameter(T &t) : t(t) {}
            virtual void set(ofxOscMessage &message) { set(t, message); }
        
        private:
#define define_set_arithmetic(type) \
inline void set(type &v, ofxOscMessage &m, size_t offset = 0) { \
if(m.getArgType(offset) == OFXOSC_TYPE_INT32) v = m.getArgAsInt32(offset); \
else if(m.getArgType(offset) == OFXOSC_TYPE_INT64) v = m.getArgAsInt64(offset); \
else if(m.getArgType(offset) == OFXOSC_TYPE_FLOAT) v = m.getArgAsFloat(offset); \
}
            define_set_arithmetic(bool);
            define_set_arithmetic(char);
            define_set_arithmetic(unsigned char);
            define_set_arithmetic(int);
            define_set_arithmetic(unsigned int);
            
            define_set_arithmetic(long);
            define_set_arithmetic(unsigned long);
            
            define_set_arithmetic(float);
            define_set_arithmetic(double);
#undef define_set_arithmetic
            
            void set(string &v, ofxOscMessage &m, size_t offset = 0) {
                v = m.getArgAsString(offset);
            }
            
            void set(ofColor &v, ofxOscMessage &m, size_t offset = 0) { setColor(m, 255, offset); }
            void set(ofFloatColor &v, ofxOscMessage &m, size_t offset = 0) { setColor(m, 1.0f, offset); }
            template <typename U>
            void setColor(ofxOscMessage &m, U defaultValue, size_t offset = 0) {
                if(m.getNumArgs() == 1) {
                    set(t.r, m, offset);
                    set(t.g, m, offset);
                    set(t.b, m, offset);
                    t.a = defaultValue;
                } else if(m.getNumArgs() == 3) {
                    set(t.r, m, offset + 0);
                    set(t.g, m, offset + 1);
                    set(t.b, m, offset + 2);
                    t.a = defaultValue;
                } else if(m.getNumArgs() == 4) {
                    set(t.r, m, offset + 0);
                    set(t.g, m, offset + 1);
                    set(t.b, m, offset + 2);
                    set(t.a, m, offset + 3);
                }
            }
            
            inline void set(ofVec2f &v, ofxOscMessage &message, size_t offset = 0) { setVec<2>(message, offset); }
            inline void set(ofVec3f &v, ofxOscMessage &message, size_t offset = 0) { setVec<3>(message, offset); }
            inline void set(ofVec4f &v, ofxOscMessage &message, size_t offset = 0) { setVec<4>(message, offset); }
            template <size_t n>
            inline void setVec(ofxOscMessage &message, size_t offset = 0) {
                for(int i = 0; i < min(static_cast<size_t>(message.getNumArgs()), n); i++) {
                    set(t[i], message, offset + i);
                }
            }
            
            inline void set(ofRectangle &v, ofxOscMessage &message, size_t offset = 0) {
                set(v.x, message, offset + 0);
                set(v.y, message, offset + 1);
                set(v.width, message, offset + 2);
                set(v.height, message, offset + 3);
            }
            
            template <typename U, size_t size>
            inline void set(U v[size], ofxOscMessage &m, size_t offset = 0) {
                for(int i = 0; i < size; i++) set(v[i], m, offset + i);
            }
            
            template <typename U>
            inline void set(vector<U> &v, ofxOscMessage &m, size_t offset = 0) {
                if(v.size() < m.getNumArgs()) { v.resize(m.getNumArgs()); }
                for(int i = 0; i < v.size(); i++) { set(v[i], m, offset + i); }
            }
            
            T &t;
        };
        
        struct CallbackParameter : AbstractParameter {
        private:
            typedef void (*Callback)(ofxOscMessage &);
        public:
            CallbackParameter(Callback callback)
            : callback(callback) {}
            
            virtual void set(ofxOscMessage &message) {
                callback(message);
            }

        private:
            Callback callback;
        };

        template <typename T>
        struct MethodCallbackParameter : AbstractParameter {
        private:
            typedef void (T::*Callback)(ofxOscMessage &);
        public:
            MethodCallbackParameter(T &that, Callback callback)
            : that(that), callback(callback) {}
            MethodCallbackParameter(T *that, Callback callback)
            : that(*that), callback(callback) {}
            
            virtual void set(ofxOscMessage &message) {
                that.*callback(message);
            }
            
        private:
            Callback callback;
            T &that;
        };

        typedef shared_ptr<AbstractParameter> ParameterRef;
        typedef shared_ptr<ofxOscReceiver> OscReceiverRef;
        typedef map<string, ParameterRef> Targets;
        typedef map<int, pair<OscReceiverRef, Targets> > TargetsMap;
        
    public:
        static OscSubscriber &getSharedInstance() {
            return *(sharedInstance ?: sharedInstance = new OscSubscriber);
        }
        
        template <typename T>
        void subscribe(int port, const string &address, T &value) {
            if(targetsMap.find(port) == targetsMap.end()) {
                OscReceiverRef receiver(new ofxOscReceiver);
                receiver->setup(port);
                targetsMap.insert(make_pair(port, make_pair(receiver, Targets())));
            }
            targetsMap[port].second.insert(make_pair(address, ParameterRef(new Parameter<T>(value))));
        }
        
        void subscribe(int port, const string &address, void (*callback)(ofxOscMessage &)) {
            if(targetsMap.find(port) == targetsMap.end()) {
                OscReceiverRef receiver(new ofxOscReceiver);
                receiver->setup(port);
                targetsMap.insert(make_pair(port, make_pair(receiver, Targets())));
            }
            targetsMap[port].second.insert(make_pair(address, ParameterRef(new CallbackParameter(callback))));
        }
        
        template <typename T>
        void subscribe(int port, const string &address, T &that, void (T::*callback)(ofxOscMessage &)) {
            if(targetsMap.find(port) == targetsMap.end()) {
                OscReceiverRef receiver(new ofxOscReceiver);
                receiver->setup(port);
                targetsMap.insert(make_pair(port, make_pair(receiver, Targets())));
            }
            targetsMap[port].second.insert(make_pair(address, ParameterRef(new MethodCallbackParameter<T>(that, callback))));
        }

        template <typename T>
        void subscribe(int port, const string &address, T *that, void (T::*callback)(ofxOscMessage &)) {
            if(targetsMap.find(port) == targetsMap.end()) {
                OscReceiverRef receiver(new ofxOscReceiver);
                receiver->setup(port);
                targetsMap.insert(make_pair(port, make_pair(receiver, Targets())));
            }
            targetsMap[port].second.insert(make_pair(address, ParameterRef(new MethodCallbackParameter<T>(that, callback))));
        }
        
        void unsubscribe(int port, const string &address) {
            if(targetsMap.find(port) == targetsMap.end()) return;
            targetsMap[port].second.erase(address);
        }
        
        void unsubscribe(int port) {
            targetsMap.erase(port);
        }
        
        inline bool isSubscribed(int port) const {
            return targetsMap.find(port) != targetsMap.end();
        }

        inline bool isSubscribed(int port, const string &address) const {
            return isSubscribed(port) && (targetsMap.at(port).second.find(address) != targetsMap.at(port).second.end());
        }
        
    private:
        void update(ofEventArgs &args) {
            for(TargetsMap::iterator _ = targetsMap.begin(); _ != targetsMap.end(); _++) {
                Targets &targets = _->second.second;
                ofxOscReceiver *receiver = _->second.first.get();
                ofxOscMessage m;
                while(receiver->hasWaitingMessages()) {
                    receiver->getNextMessage(&m);
                    const string &address = m.getAddress();
                    if(targets.find(address) != targets.end()) { targets[address]->set(m); }
                }
            }
        }
        
        OscSubscriber() {
            ofAddListener(ofEvents().update, this, &OscSubscriber::update, OF_EVENT_ORDER_BEFORE_APP);
        }
        virtual ~OscSubscriber() {
            ofRemoveListener(ofEvents().update, this, &OscSubscriber::update, OF_EVENT_ORDER_BEFORE_APP);
        }
        TargetsMap targetsMap;
        static OscSubscriber *sharedInstance;
    };
    OscSubscriber *OscSubscriber::sharedInstance = NULL;
};

typedef ofx::OscSubscriber ofxOscSubscriber;

template <typename T>
inline void ofxSubscribeOsc(int port, const string &address, T &value) {
    ofxOscSubscriber::getSharedInstance().subscribe(port, address, value);
}

inline void ofxUnsubscribeOsc(int port, const string &address) {
    ofxOscSubscriber::getSharedInstance().unsubscribe(port, address);
}

inline void ofxUnsubscribeOsc(int port) {
    ofxOscSubscriber::getSharedInstance().unsubscribe(port);
}