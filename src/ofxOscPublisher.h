//
//  ofxOscPublisher.h
//
//  Created by ISHII 2bit on 2015/05/11.
//
//

#pragma once

#include "ofMain.h"
#include "ofxOsc.h"

namespace ofx {
    namespace {
        template <typename T>
        struct add_reference_if_non_arithmetic {
            typedef T& type;
        };
        
#define define_add_reference_if_non_arithmetic(T) \
template <> \
struct add_reference_if_non_arithmetic<T> { \
typedef T type; \
};
        define_add_reference_if_non_arithmetic(bool);
        define_add_reference_if_non_arithmetic(short);
        define_add_reference_if_non_arithmetic(unsigned short);
        define_add_reference_if_non_arithmetic(int);
        define_add_reference_if_non_arithmetic(unsigned int);
        define_add_reference_if_non_arithmetic(long);
        define_add_reference_if_non_arithmetic(unsigned long);
        define_add_reference_if_non_arithmetic(float);
        define_add_reference_if_non_arithmetic(double);
#undef define_add_reference_if_non_arithmetic
#define TypeRef(T) typename add_reference_if_non_arithmetic<T>::type
    };
    
    class OscPublisher {
        struct AbstractParameter {
            virtual void send(ofxOscSender &sender, const string &address) {}
        };
        
        struct SetImplementation {
        protected:
#define define_set_int(type) inline void set(ofxOscMessage &m, type v) const { m.addIntArg(v); }
            define_set_int(bool);
            define_set_int(short);
            define_set_int(unsigned short);
            define_set_int(int);
            define_set_int(unsigned int);
#undef define_set_int
#define define_set_int64(type) inline void set(ofxOscMessage &m, type v) const { m.addInt64Arg(v); }
            define_set_int64(long);
            define_set_int64(unsigned long);
#undef define_set_int64
#define define_set_float(type) inline void set(ofxOscMessage &m, type v) const { m.addFloatArg(v); }
            define_set_float(float);
            define_set_float(double);
#undef define_set_float
            inline void set(ofxOscMessage &m, const string &v) const { m.addStringArg(v); }
            template <typename PixType>
            inline void set(ofxOscMessage &m, const ofColor_<PixType> &v) const {  setVec<4>(m, v); }
            inline void set(ofxOscMessage &m, const ofVec2f &v) const { setVec<2>(m, v); }
            inline void set(ofxOscMessage &m, const ofVec3f &v) const { setVec<3>(m, v); }
            inline void set(ofxOscMessage &m, const ofVec4f &v) const { setVec<4>(m, v); }
            inline void set(ofxOscMessage &m, const ofQuaternion &v) const { setVec<4>(m, v); }
            inline void set(ofxOscMessage &m, const ofMatrix3x3 &v) const { setVec<9>(m, v); }
            
            template <size_t n, typename T>
            inline void setVec(ofxOscMessage &m, const T &v) const {
                for(int i = 0; i < n; i++) { set(m, v[i]); }
            }
            
            inline void set(ofxOscMessage &m, const ofMatrix4x4 &v) const {
                for(int j = 0; j < 4; j++) for(int i = 0; i < 4; i++) set(m, v(i, j));
            }
            
            inline void set(ofxOscMessage &m, const ofRectangle &v) const {
                set(m, v.x);
                set(m, v.y);
                set(m, v.width);
                set(m, v.height);
            }
            
            template <typename U, size_t size>
            inline void set(ofxOscMessage &m, const U v[size]) const {
                for(int i = 0; i < size; i++) { set(m, v[i]); }
            }
            
            template <typename U>
            inline void set(ofxOscMessage &m, const vector<U> &v) const {
                for(int i = 0; i < v.size(); i++) { set(m, v[i]); }
            }
        };
        
        template <typename T, bool isCheckValue>
        struct Parameter : AbstractParameter, SetImplementation {
            Parameter(T &t) : t(t) {}
            virtual void send(ofxOscSender &sender, const string &address) {
                ofxOscMessage m;
                m.setAddress(address);
                set(m, get());
                sender.sendMessage(m);
            }

        protected:
            virtual TypeRef(T) get() { return t; }
            T &t;
        };

        template <typename T>
        struct Parameter<T, true> : AbstractParameter, SetImplementation {
            Parameter(T &t) : t(t), old(t) {}
            virtual void send(ofxOscSender &sender, const string &address) {
                if(!isChanged()) return;
                ofxOscMessage m;
                m.setAddress(address);
                set(m, get());
                sender.sendMessage(m);
            }
            
        protected:
            inline bool isChanged() {
                if(old != get()) {
                    old = get();
                    return true;
                } else {
                    return false;
                }
            }
            
            virtual TypeRef(T) get() { return t; }
        protected:
            T &t;
            T old;
        };

        template <typename T, bool isCheckValue>
        struct GetterFunctionParameter : Parameter<T, isCheckValue> {
            typedef T (*GetterFunction)();
            GetterFunctionParameter(GetterFunction getter) : Parameter<T, isCheckValue>(dummy), getter(getter) {}
            
        protected:
            virtual TypeRef(T) get() { return getter(); }
        private:
            GetterFunction getter;
            T dummy;
        };
        
        template <typename T, typename U, bool isCheckValue>
        struct GetterParameter : Parameter<T, isCheckValue> {
            typedef T (U::*Getter)();
            GetterParameter(U *that, Getter getter) : getter(getter), that(*that) {}
            GetterParameter(U &that, Getter getter) : getter(getter), that(that) {}
            
        protected:
            virtual TypeRef(T) get() { return (that.*getter)(); }
        private:
            Getter getter;
            U &that;
        };
        
        typedef shared_ptr<AbstractParameter> ParameterRef;
        typedef shared_ptr<ofxOscSender> OscSenderRef;
        typedef pair<string, int> SenderKey;
        typedef map<string, ParameterRef> Targets;
        typedef map<SenderKey, pair<OscSenderRef, Targets> > TargetsMap;
        
    public:
        static OscPublisher &getSharedInstance() {
            return *(sharedInstance ? sharedInstance : sharedInstance = new OscPublisher);
        }
        
        inline void publish(const string &ip, int port, const string &address, ParameterRef ref) {
            SenderKey key(ip, port);
            if(!isPublished(key)) {
                ofxOscSender *sender = new ofxOscSender;
                sender->setup(ip, port);
                targetsMap.insert(make_pair(key, make_pair(OscSenderRef(sender), Targets())));
            }
            targetsMap[key].second.insert(make_pair(address, ref));
        }
        
        template <typename T>
        void publish(const string &ip, int port, const string &address, const T &value, bool whenValueIsChanged = false) {
            ParameterRef p;
            if(whenValueIsChanged) p = ParameterRef(new Parameter<T, true>(value));
            else                   p = ParameterRef(new Parameter<T, false>(value));
            publish(ip, port, address, p);
        }
        
        template <typename T>
        void publish(const string &ip, int port, const string &address, T (*getter)(), bool whenValueIsChanged = false) {
            ParameterRef p;
            if(whenValueIsChanged) p = ParameterRef(new GetterFunctionParameter<T, true>(getter));
            else                   p = ParameterRef(new GetterFunctionParameter<T, false>(getter));
            publish(ip, port, address, p);
        }

        template <typename T, typename U>
        void publish(const string &ip, int port, const string &address, U *that, T (U::*getter)(), bool whenValueIsChanged = false) {
            ParameterRef p;
            if(whenValueIsChanged) p = ParameterRef(new GetterParameter<T, U, true>(that, getter));
            else                   p = ParameterRef(new GetterParameter<T, U, false>(that, getter));
            publish(ip, port, address, p);
        }

        template <typename T, typename U>
        void publish(const string &ip, int port, const string &address, U &that, T (U::*getter)(), bool whenValueIsChanged = false) {
            ParameterRef p;
            if(whenValueIsChanged) p = ParameterRef(new GetterParameter<T, U, true>(that, getter));
            else                   p = ParameterRef(new GetterParameter<T, U, false>(that, getter));
            publish(ip, port, address, p);
        }
        
        void unsubscribe(const string &ip, int port, const string &address) {
            SenderKey key(ip, port);
            if(targetsMap.find(key) == targetsMap.end()) return;
            targetsMap[key].second.erase(address);
        }
        
        void unsubscribe(const string &ip, int port) {
            SenderKey key(ip, port);
            targetsMap.erase(key);
        }

        inline bool isPublished(const string &ip, int port) const {
            return isPublished(SenderKey(ip, port));
        }

        inline bool isPublished(const SenderKey &key) const {
            return targetsMap.find(key) != targetsMap.end();
        }

        inline bool isPublished(const string &ip, int port, const string &address) const {
            SenderKey key(ip, port);
            return isPublished(key) && (targetsMap.at(key).second.find(address) != targetsMap.at(key).second.end());
        }

    private:
        void update(ofEventArgs &args) {
            for(TargetsMap::iterator _ = targetsMap.begin(); _ != targetsMap.end(); _++) {
                Targets &targets = _->second.second;
                ofxOscSender *sender = _->second.first.get();
                for(Targets::iterator it = targets.begin(); it != targets.end(); it++) {
                    it->second->send(*sender, it->first);
                }
            }
        }
        OscPublisher() {
            ofAddListener(ofEvents().update, this, &OscPublisher::update, OF_EVENT_ORDER_BEFORE_APP);
        }
        virtual ~OscPublisher() {
            ofRemoveListener(ofEvents().update, this, &OscPublisher::update, OF_EVENT_ORDER_BEFORE_APP);
        }
        TargetsMap targetsMap;
        static OscPublisher *sharedInstance;
    };
    OscPublisher *OscPublisher::sharedInstance = NULL;
};

typedef ofx::OscPublisher ofxOscPublisher;

template <typename T>
void ofxPublishOsc(const string &ip, int port, const string &address, const T &value, bool whenValueIsChanged = false) {
    ofxOscPublisher::getSharedInstance().publish(ip, port, address, value, whenValueIsChanged);
}

template <typename T>
void ofxPublishOsc(const string &ip, int port, const string &address, T (*getter)(), bool whenValueIsChanged = false) {
    ofxOscPublisher::getSharedInstance().publish(ip, port, address, getter, whenValueIsChanged);
}

template <typename T, typename U>
void ofxPublishOsc(const string &ip, int port, const string &address, U *that, T (U::*getter)(), bool whenValueIsChanged = false) {
    ofxOscPublisher::getSharedInstance().publish(ip, port, address, that, getter, whenValueIsChanged);
}

template <typename T, typename U>
void ofxPublishOsc(const string &ip, int port, const string &address, U &that, T (U::*getter)(), bool whenValueIsChanged = false) {
    ofxOscPublisher::getSharedInstance().publish(ip, port, address, that, getter, whenValueIsChanged);
}
