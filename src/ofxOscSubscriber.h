//
//  ofxOscSubscriber.h
//  VJ2D
//
//  Created by ISHII 2bit on 2015/05/10.
//
//

#pragma once

#include "ofMain.h"
#include "ofxOsc.h"

namespace ofx {
    struct AbstractParameter {
    public:
        virtual void set(ofxOscMessage &message) {}
    };
    
    template <typename T>
    struct Parameter : AbstractParameter {
        Parameter(T &t) : t(t) {}
        virtual void set(ofxOscMessage &message) {
            set(t, message);
        }
        
    private:
#define define_set_type(type, Type) \
        inline void set(type &v, ofxOscMessage &m, size_t offset = 0) { \
            v = m.getArgAs##Type(offset); \
        }
        define_set_type(bool, Int32);
        define_set_type(char, Int32);
        define_set_type(unsigned char, Int32);
        define_set_type(int, Int32);
        define_set_type(unsigned int, Int32);
        
        define_set_type(long, Int64);
        define_set_type(unsigned long, Int64);
        
        define_set_type(float, Float);
        define_set_type(double, Float);
#undef define_set_type
        void set(string &v, ofxOscMessage &m, size_t offset = 0) {
            v = m.getArgAsString(offset);
        }
        
        void set(ofColor &v, ofxOscMessage &m, size_t offset = 0)
            { setColor(m, &ofxOscMessage::getArgAsInt32, 255, offset); }
        void set(ofFloatColor &v, ofxOscMessage &m, size_t offset = 0)
            { setColor(m, &ofxOscMessage::getArgAsFloat, 1.0f, offset); }
        template <typename U>
        void setColor(ofxOscMessage &m, U (ofxOscMessage::*getArg)(int), U defaultValue, size_t offset = 0) {
            if(m.getNumArgs() == 1) {
                t.r = m.*getArg(offset);
                t.g = m.*getArg(offset);
                t.b = m.*getArg(offset);
                t.a = defaultValue;
            } else if(m.getNumArgs() == 3) {
                t.r = m.*getArg(offset + 0);
                t.g = m.*getArg(offset + 1);
                t.b = m.*getArg(offset + 2);
                t.a = defaultValue;
            } else if(m.getNumArgs() == 4) {
                t.r = m.*getArg(offset + 0);
                t.g = m.*getArg(offset + 1);
                t.b = m.*getArg(offset + 2);
                t.a = m.*getArg(offset + 3);
            }
        }
        
        inline void set(ofVec2f &v, ofxOscMessage &message, size_t offset = 0)
            { setVec<2>(message, offset); }
        inline void set(ofVec3f &v, ofxOscMessage &message, size_t offset = 0)
            { setVec<3>(message, offset); }
        inline void set(ofVec4f &v, ofxOscMessage &message, size_t offset = 0)
            { setVec<4>(message, offset); }
        template <size_t n>
        inline void setVec(ofxOscMessage &message, size_t offset = 0) {
            for(int i = offset; i < offset + min(message.getNumArgs(), static_cast<int>(n)); i++) {
                t[i] = message.getArgAsFloat(i);
            }
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
        
        inline void set(ofRectangle &v, ofxOscMessage &message, size_t offset = 0) {
            v.x      = message.getArgAsFloat(offset + 0);
            v.y      = message.getArgAsFloat(offset + 1);
            v.width  = message.getArgAsFloat(offset + 2);
            v.height = message.getArgAsFloat(offset + 3);
        }
        
        T &t;
    };
    
    class OscSubscriber {
        typedef map<string, shared_ptr<AbstractParameter> > Targets;
        typedef map<int, pair<shared_ptr<ofxOscReceiver>, Targets> > TargetsMap;
    public:
        static OscSubscriber &getSharedInstance() {
            return *(sharedInstance ?: sharedInstance = new OscSubscriber);
        }
        
        template <typename T>
        void subscribe(int port, const string &address, T &value) {
            if(targetsMap.find(port) == targetsMap.end()) {
                shared_ptr<ofxOscReceiver> receiver(new ofxOscReceiver);
                receiver->setup(port);
                targetsMap.insert(make_pair(port, make_pair(receiver, Targets())));
            }
            targetsMap[port].second.insert(make_pair(address, shared_ptr<AbstractParameter>(new Parameter<T>(value))));
        }
        
        void unsubscribe(int port, const string &address) {
            if(targetsMap.find(port) == targetsMap.end()) return;
            targetsMap[port].second.erase(address);
        }
        
        void unsubscribe(int port) {
            targetsMap.erase(port);
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
            ofAddListener(ofEvents().update, this, &OscSubscriber::update);
        }
        virtual ~OscSubscriber() {
            ofRemoveListener(ofEvents().update, this, &OscSubscriber::update);
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