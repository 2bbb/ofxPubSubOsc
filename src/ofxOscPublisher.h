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
        
        template <typename T>
        struct remove_reference {
            typedef T type;
        };
        template <typename T>
        struct remove_reference<T &> {
            typedef T type;
        };
#define RemoveRef(T) typename remove_reference<T>::type
        
        bool operator==(const ofBuffer &x, const ofBuffer &y) {
            return (x.size() == y.size()) && (memcmp(x.getBinaryBuffer(), y.getBinaryBuffer(), x.size()) == 0);
        }
        
        bool operator!=(const ofBuffer &x, const ofBuffer &y) {
            return (x.size() != y.size()) || (memcmp(x.getBinaryBuffer(), y.getBinaryBuffer(), x.size()) != 0);
        }
    };
    
    class OscPublisherManager {
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
            inline void set(ofxOscMessage &m, const ofBuffer &v) const { m.addBlobArg(v); };
            
            template <typename PixType>
            inline void set(ofxOscMessage &m, const ofColor_<PixType> &v) const {  setVec<4>(m, v); }
            inline void set(ofxOscMessage &m, const ofVec2f &v) const { setVec<2>(m, v); }
            inline void set(ofxOscMessage &m, const ofVec3f &v) const { setVec<3>(m, v); }
            inline void set(ofxOscMessage &m, const ofVec4f &v) const { setVec<4>(m, v); }
            inline void set(ofxOscMessage &m, const ofQuaternion &v) const { setVec<4>(m, v); }
            
            template <size_t n, typename T>
            inline void setVec(ofxOscMessage &m, const T &v) const {
                for(int i = 0; i < n; i++) { set(m, v[i]); }
            }
            
            inline void set(ofxOscMessage &m, const ofMatrix3x3 &v) const {
                set(m, v.a);
                set(m, v.b);
                set(m, v.c);
                set(m, v.d);
                set(m, v.e);
                set(m, v.f);
                set(m, v.g);
                set(m, v.h);
                set(m, v.i);
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
        
#pragma mark Condition
        
        struct AbstractCondition {
            AbstractCondition() : bPublishNow(true) {};
            inline bool getCondition() { return isPublishNow() && inner_condition(); };
            inline bool isPublishNow() const { return bPublishNow; };
            inline void setEnablePublish(bool bEnablePublish) { this->bPublishNow = bEnablePublish; };
            virtual bool inner_condition() { return true; };
            
            typedef shared_ptr<AbstractCondition> Ref;
        private:
            bool bPublishNow;
        };

        struct ConditionRef : AbstractCondition {
            ConditionRef(bool &ref) : AbstractCondition(), ref(ref) {};
            virtual bool inner_condition() { return ref; };
        private:
            bool &ref;
        };

        struct ConditionFunction : AbstractCondition {
            ConditionFunction(bool (*getter)()) : AbstractCondition(), getter(getter) {};
            virtual bool inner_condition() { return getter(); };
        private:
            bool (*getter)();
        };

        template <typename T>
        struct ConditionMethod : AbstractCondition {
            ConditionMethod(T &that, bool (T::*getter)()) : AbstractCondition(), getter(getter) {};
            virtual bool inner_condition() { return (that.*getter)(); };
        private:
            T &that;
            bool (T::*getter)();
        };

        typedef AbstractCondition::Ref AbstractConditionRef;
#pragma mark Parameter
        
        struct AbstractParameter {
            AbstractParameter() : condition(new AbstractCondition) {}
            virtual void send(ofxOscSender &sender, const string &address) {}
            void setCondition(AbstractConditionRef ref) { condition = ref; };
            
            inline void setEnablePublish(bool bEnablePublish) { condition->setEnablePublish(bEnablePublish); };
            inline bool isPublishNow() const { return condition->isPublishNow(); };
        protected:
            bool canPublish() {
                return condition->getCondition();
            }
        private:
            AbstractConditionRef condition;
        };
        
        template <typename T, bool isCheckValue>
        struct Parameter : AbstractParameter, SetImplementation {
            Parameter(T &t)
            : t(t) {}
            
            virtual void send(ofxOscSender &sender, const string &address) {
                if(!canPublish()) return;
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
            Parameter(T &t) : t(t) {}
            virtual void send(ofxOscSender &sender, const string &address) {
                if(!canPublish() || !isChanged()) return;
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
            GetterFunctionParameter(GetterFunction getter)
            : Parameter<T, isCheckValue>(dummy)
            , getter(getter) {}
            
        protected:
            virtual TypeRef(T) get() { return dummy = getter(); }
            GetterFunction getter;
            RemoveRef(T) dummy;
        };
        
        template <typename T, typename U, bool isCheckValue>
        struct GetterParameter : Parameter<T, isCheckValue> {
            typedef T (U::*Getter)();
            GetterParameter(U *that, Getter getter)
            : Parameter<T, isCheckValue>(dummy)
            , getter(getter)
            , that(*that) {}
            
            GetterParameter(U &that, Getter getter)
            : Parameter<T, isCheckValue>(dummy)
            , getter(getter)
            , that(that) {}
            
        protected:
            virtual TypeRef(T) get() { return dummy = (that.*getter)(); }
            Getter getter;
            U &that;
            RemoveRef(T) dummy;
        };
        
        typedef shared_ptr<AbstractParameter> ParameterRef;
        typedef pair<string, int> SenderKey;
        typedef map<string, ParameterRef> Targets;
    public:
        class OscPublisher {
        public:
#pragma mark publish
            
            inline void publish(const string &address, ParameterRef ref) {
                if(targets.find(address) == targets.end()) {
                    targets.insert(make_pair(address, ref));
                } else {
                    targets[address] = ref;
                }
            }
            
            template <typename T>
            void publish(const string &address, T &value, bool whenValueIsChanged = true) {
                ParameterRef p;
                if(whenValueIsChanged) p = ParameterRef(new Parameter<T, true>(value));
                else                   p = ParameterRef(new Parameter<T, false>(value));
                publish(address, p);
            }
            
            template <typename T>
            void publish(const string &address, T (*getter)(), bool whenValueIsChanged = true) {
                ParameterRef p;
                if(whenValueIsChanged) p = ParameterRef(new GetterFunctionParameter<T, true>(getter));
                else                   p = ParameterRef(new GetterFunctionParameter<T, false>(getter));
                publish(address, p);
            }

            template <typename T, typename U>
            void publish(const string &address, U *that, T (U::*getter)(), bool whenValueIsChanged = true) {
                publish(address, *that, getter, whenValueIsChanged);
            }

            template <typename T, typename U>
            void publish(const string &address, U &that, T (U::*getter)(), bool whenValueIsChanged = true) {
                ParameterRef p;
                if(whenValueIsChanged) p = ParameterRef(new GetterParameter<T, U, true>(that, getter));
                else                   p = ParameterRef(new GetterParameter<T, U, false>(that, getter));
                publish(address, p);
            }
            
#pragma mark publish conditional
#pragma mark condition is bool value ref
            
            template <typename T>
            void publishIf(bool &condition, const string &address, T &value) {
                ParameterRef p = ParameterRef(new Parameter<T, false>(value));
                p->setCondition(shared_ptr<AbstractCondition>(new ConditionRef(condition)));
                publish(address, p);
            }

            template <typename T>
            void publishIf(bool &condition, const string &address, T (*getter)()) {
                ParameterRef p = ParameterRef(new GetterFunctionParameter<T, false>(getter));
                p->setCondition(shared_ptr<AbstractCondition>(new ConditionRef(condition)));
                publish(address, p);
            }
            
            template <typename T, typename U>
            void publishIf(bool &condition, const string &address, U *that, T (U::*getter)()) {
                publishIf(condition, address, *that, getter);
            }
            
            template <typename T, typename U>
            void publishIf(bool &condition, const string &address, U &that, T (U::*getter)()) {
                ParameterRef p = ParameterRef(new GetterParameter<T, U, false>(that, getter));
                p->setCondition(shared_ptr<AbstractCondition>(new ConditionRef(condition)));
                publish(address, p);
            }

#pragma mark condition is function
            
            template <typename T>
            void publishIf(bool (*condition)(), const string &address, T &value) {
                ParameterRef p = ParameterRef(new Parameter<T, false>(value));
                p->setCondition(shared_ptr<AbstractCondition>(new ConditionFunction(condition)));
                publish(address, p);
            }
            
            template <typename T>
            void publishIf(bool (*condition)(), const string &address, T (*getter)()) {
                ParameterRef p = ParameterRef(new GetterFunctionParameter<T, false>(getter));
                p->setCondition(shared_ptr<AbstractCondition>(new ConditionFunction(condition)));
                publish(address, p);
            }
            
            template <typename T, typename U>
            void publishIf(bool (*condition)(), const string &address, U *that, T (U::*getter)()) {
                publishIf(condition, address, *that, getter);
            }
            
            template <typename T, typename U>
            void publishIf(bool (*condition)(), const string &address, U &that, T (U::*getter)()) {
                ParameterRef p = ParameterRef(new GetterParameter<T, U, false>(that, getter));
                p->setCondition(shared_ptr<AbstractCondition>(new ConditionFunction(condition)));
                publish(address, p);
            }

#pragma mark condition is method
            
            template <typename T, typename C>
            void publishIf(C *condition, bool (C::*method)(), const string &address, T &value) {
                publishIf(*condition, method, address, value);
            }
            
            template <typename T, typename C>
            void publishIf(C &condition, bool (C::*method)(), const string &address, T &value) {
                ParameterRef p = ParameterRef(new Parameter<T, true>(value));
                p->setCondition(shared_ptr<AbstractCondition>(new ConditionMethod<C>(condition, method)));
                publish(address, p);
            }
            
            template <typename T, typename C>
            void publishIf(C *condition, bool (C::*method)(), const string &address, T (*getter)()) {
                publishIf(*condition, method, address, getter);
            }

            template <typename T, typename C>
            void publishIf(C &condition, bool (C::*method)(), const string &address, T (*getter)()) {
                ParameterRef p = ParameterRef(new GetterFunctionParameter<T, true>(getter));
                p->setCondition(shared_ptr<AbstractCondition>(new ConditionMethod<C>(condition)));
                publish(address, p);
            }
            
            template <typename T, typename U, typename C>
            void publishIf(C *condition, bool (C::*method)(), const string &address, U *that, T (U::*getter)()) {
                publishIf(*condition, method, address, *that, getter);
            }
            
            template <typename T, typename U, typename C>
            void publishIf(C &condition, bool (C::*method)(), const string &address, U *that, T (U::*getter)()) {
                publishIf(condition, method, address, *that, getter);
            }
            
            template <typename T, typename U, typename C>
            void publishIf(C *condition, bool (C::*method)(), const string &address, U &that, T (U::*getter)()) {
                publishIf(*condition, method, address, that, getter);
            }

            template <typename T, typename U, typename C>
            void publishIf(C &condition, bool (C::*method)(), const string &address, U &that, T (U::*getter)()) {
                ParameterRef p = ParameterRef(new GetterParameter<T, U, true>(that, getter));
                p->setCondition(shared_ptr<AbstractCondition>(new ConditionMethod<C>(condition)));
                publish(address, p);
            }
            
#pragma mark unpublish
            
            void unpublish(const string &address) {
                if(targets.find(address) == targets.end()) targets.erase(address);
            }
            
            void unpublish() {
                targets.clear();
            }

#pragma mark stop publish temporary
            
            void stopPublishTemporary(const string &address) {
                if(isPublished(address)) targets[address]->setEnablePublish(false);
            }
            
            void resumePublish(const string &address) {
                if(isPublished(address)) targets[address]->setEnablePublish(true);
            }
            
#pragma mark status
            
            inline bool isPublished() const {
                return !targets.empty();
            }

            inline bool isPublished(const string &address) const {
                return isPublished() && (targets.find(address) != targets.end());
            }

            inline bool isEnabled(const string &address) const {
                return isPublished(address) && targets.at(address)->isPublishNow();
            }

            typedef shared_ptr<OscPublisher> Ref;
        private:
            OscPublisher(const SenderKey &key) : key(key) {
                sender.setup(key.first, key.second);
            }
            
            void update() {
                for(Targets::iterator it = targets.begin(); it != targets.end(); it++) {
                    it->second->send(sender, it->first);
                }
            }
            
            SenderKey key;
            ofxOscSender sender;
            Targets targets;
            friend class OscPublisherManager;
        };
        
        static OscPublisherManager &getSharedInstance() {
            static OscPublisherManager *sharedInstance = new OscPublisherManager;
            return *sharedInstance;
        }
        
        static OscPublisher &getOscPublisher(const string &ip, int port) {
            OscPublishers &publishers = getSharedInstance().publishers;
            SenderKey key(ip, port);
            if(publishers.find(key) == publishers.end()) {
                publishers.insert(make_pair(key, OscPublisher::Ref(new OscPublisher(key))));
            }
            return *(publishers[key].get());
        }
        
    private:
        typedef map<SenderKey, OscPublisher::Ref> OscPublishers;
        void update(ofEventArgs &args) {
            for(OscPublishers::iterator it = publishers.begin(); it != publishers.end(); ++it) {
                it->second->update();
            }
        }
        OscPublisherManager() {
            ofAddListener(ofEvents().update, this, &OscPublisherManager::update, OF_EVENT_ORDER_AFTER_APP);
        }
        virtual ~OscPublisherManager() {
            ofRemoveListener(ofEvents().update, this, &OscPublisherManager::update, OF_EVENT_ORDER_AFTER_APP);
        }
        OscPublishers publishers;
    };
#undef TypeRef
};

#pragma mark - syntax sugars

#pragma mark getter

typedef ofx::OscPublisherManager ofxOscPublisherManager;
typedef ofxOscPublisherManager::OscPublisher ofxOscPublisher;

inline ofxOscPublisher &ofxGetOscPublisher(const string &ip, int port) {
    return ofxOscPublisherManager::getOscPublisher(ip, port);
}

#pragma mark publish

template <typename T>
inline void ofxPublishOsc(const string &ip, int port, const string &address, T &value, bool whenValueIsChanged = true) {
    ofxGetOscPublisher(ip, port).publish(address, value, whenValueIsChanged);
}

template <typename T>
inline void ofxPublishOsc(const string &ip, int port, const string &address, T (*getter)(), bool whenValueIsChanged = true) {
    ofxGetOscPublisher(ip, port).publish(address, getter, whenValueIsChanged);
}

template <typename T, typename U>
inline void ofxPublishOsc(const string &ip, int port, const string &address, U *that, T (U::*getter)(), bool whenValueIsChanged = true) {
    ofxGetOscPublisher(ip, port).publish(address, that, getter, whenValueIsChanged);
}

template <typename T, typename U>
inline void ofxPublishOsc(const string &ip, int port, const string &address, U &that, T (U::*getter)(), bool whenValueIsChanged = true) {
    ofxGetOscPublisher(ip, port).publish(address, that, getter, whenValueIsChanged);
}

#pragma mark publish if condition

template <typename T>
inline void ofxPublishOscIf(bool &condition, const string &ip, int port, const string &address, T &value) {
    ofxGetOscPublisher(ip, port).publishIf(condition, address, value);
}

template <typename T>
inline void ofxPublishOsc(bool &condition, const string &ip, int port, const string &address, T (*getter)()) {
    ofxGetOscPublisher(ip, port).publishIf(condition, address, getter);
}

template <typename T, typename U>
inline void ofxPublishOsc(bool &condition, const string &ip, int port, const string &address, U *that, T (U::*getter)()) {
    ofxGetOscPublisher(ip, port).publishIf(condition, address, that, getter);
}

template <typename T, typename U>
inline void ofxPublishOsc(bool &condition, const string &ip, int port, const string &address, U &that, T (U::*getter)()) {
    ofxGetOscPublisher(ip, port).publishIf(condition, address, that, getter);
}

#pragma mark unpublish

inline void ofxUnpublishOSC(const string &ip, int port, const string &address) {
    ofxGetOscPublisher(ip, port).unpublish(address);
}

inline void ofxUnpublishOSC(const string &ip, int port) {
    ofxGetOscPublisher(ip, port).unpublish();
}
