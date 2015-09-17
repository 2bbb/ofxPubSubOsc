//
//  ofxOscPublisher.h
//
//  Created by ISHII 2bit on 2015/05/11.
//
//

#pragma once

#include "ofMain.h"
#include "ofxOsc.h"

#include "ofxpubsubosc_type_utils.h"

namespace ofx {
    using namespace ofxpubsubosc;
    
    namespace {
        template <typename T>
        struct remove_reference {
            typedef T type;
        };
        template <typename T>
        struct remove_reference<T &> {
            typedef T type;
        };
#define remove_ref(T) typename ofx::remove_reference<T>::type
        
        template <typename T> struct is_integral { static const bool value = false; };
#define define_is_integral(T) template <> struct is_integral<T> { static const bool value = true; };
        define_is_integral(bool);
        define_is_integral(short);
        define_is_integral(unsigned char);
        define_is_integral(char);
        define_is_integral(unsigned short);
        define_is_integral(int);
        define_is_integral(unsigned int);
        define_is_integral(long);
        define_is_integral(unsigned long);
        define_is_integral(long long);
        define_is_integral(unsigned long long);
#undef define_is_integral
        
#define type_ref(T) typename add_reference_if_non_arithmetic<T>::type

        template <bool b>
        struct enable;
        
        template <>
        struct enable<true> { typedef void type; };
        
        template <typename T>
        struct is_integral_and_lt_64bit {
            static const bool value = is_integral<T>::value && (sizeof(T) < 8);
        };
        
        template <typename T>
        struct is_integral_and_geq_64bit {
            static const bool value = is_integral<T>::value && (8 <= sizeof(T));
        };
    };
    
    class OscPublisherManager {
    private:
        struct SetImplementation {
        protected:
            template <typename T>
            inline typename enable<is_integral_and_lt_64bit<T>::value>::type set(ofxOscMessage &m, T v) const { m.addIntArg(v); }
            
            template <typename T>
            inline typename enable<is_integral_and_geq_64bit<T>::value>::type set(ofxOscMessage &m, T v) const { m.addInt64Arg(v); }
            
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
            inline void set(ofxOscMessage &m, const U (&v)[size]) const {
                for(int i = 0; i < size; i++) { set(m, v[i]); }
            }
            
            template <typename U>
            inline void set(ofxOscMessage &m, const vector<U> &v) const {
                for(int i = 0; i < v.size(); i++) { set(m, v[i]); }
            }
            
#pragma mark ofParameter<T> / ofParameterGroup
            
            template <typename U>
            inline void set(ofxOscMessage &m, const ofParameter<U> &p) const {
                set(m, p.get());
            }
        };
        
#pragma mark Condition
        
        struct BasicCondition {
            BasicCondition() : bPublishNow(true) {};
            
            inline bool getCondition() { return isPublishNow() && inner_condition(); };
            
            inline bool isPublishNow() const { return bPublishNow; };
            inline void setEnablePublish(bool bEnablePublish) { this->bPublishNow = bEnablePublish; };
            
            virtual bool inner_condition() { return true; };
            
            typedef shared_ptr<BasicCondition> Ref;
        private:
            bool bPublishNow;
        };

        struct ConditionRef : BasicCondition {
            ConditionRef(bool &ref) : BasicCondition(), ref(ref) {};
            virtual bool inner_condition() { return ref; };
        private:
            bool &ref;
        };

        struct ConditionFunction : BasicCondition {
            ConditionFunction(bool (*getter)()) : BasicCondition(), getter(getter) {};
            virtual bool inner_condition() { return getter(); };
        private:
            bool (*getter)();
        };

        template <typename T>
        struct ConditionMethod : BasicCondition {
            ConditionMethod(T &that, bool (T::*getter)())
            : BasicCondition()
            , that(that)
            , getter(getter) {};
            
            virtual bool inner_condition() { return (that.*getter)(); };
        private:
            T &that; // dirty!!!
            bool (T::*getter)();
        };

        template <typename T>
        struct ConstConditionMethod : BasicCondition {
            ConstConditionMethod(const T &that, bool (T::*getter)() const)
            : BasicCondition()
            , that(that)
            , getter(getter) {};
            
            virtual bool inner_condition() { return (that.*getter)(); };
        private:
            const T &that;
            bool (T::*getter)() const;
        };

        typedef BasicCondition::Ref BasicConditionRef;
        
#pragma mark Parameter
        
        struct AbstractParameter {
            AbstractParameter() : condition(new BasicCondition) {}
            virtual bool setMessage(ofxOscMessage &m, const string &address) = 0;
            void setCondition(BasicConditionRef ref) { condition = ref; };
            
            inline void setEnablePublish(bool bEnablePublish) { condition->setEnablePublish(bEnablePublish); };
            inline bool isPublishNow() const { return condition->isPublishNow(); };
        protected:
            bool canPublish() {
                return condition->getCondition();
            }
        private:
            BasicConditionRef condition;
        };
        
        template <typename T, bool isCheckValue>
        struct Parameter : AbstractParameter, SetImplementation {
            Parameter(T &t)
            : t(t) {}
            
            virtual bool setMessage(ofxOscMessage &m, const string &address) {
                if(!canPublish() || !isChanged()) return false;
                m.setAddress(address);
                set(m, get());
                return true;
            }
        protected:
            virtual bool isChanged() { return true; }
            virtual type_ref(T) get() { return t; }
            T &t;
        };

        template <typename T>
        struct Parameter<T, true> : Parameter<T, false> {
            Parameter(T &t)
            : Parameter<T, false>(t) {}
            
        protected:
            virtual bool isChanged() {
                if(old != this->get()) {
                    old = this->get();
                    return true;
                } else {
                    return false;
                }
            }
            
            T old;
        };

        template <typename Base, size_t size>
        struct Parameter<Base(&)[size], true> : AbstractParameter, SetImplementation {
            Parameter(Base (&t)[size])
            : t(t) { for(size_t i = 0; i < size; i++) old[i] = t[i]; }
            virtual ~Parameter() { };
            
            virtual bool setMessage(ofxOscMessage &m, const string &address) {
                if(!canPublish() || !isChanged()) return false;
                m.setAddress(address);
                set(m, get());
                return true;
            }
            
        protected:
            virtual bool isChanged() {
                bool isChange = false;
                for(int i = 0; i < size; i++) {
                    isChange = isChange || (old[i] != get()[i]);
                    if(isChange) break;
                }
                
                if(isChange) {
                    for(int i = 0; i < size; i++) {
                        old[i] = get()[i];
                    }
                    return true;
                } else {
                    return false;
                }
            }
            
            virtual Base (&get())[size] { return t; }
        protected:
            Base (&t)[size];
            Base old[size];
        };
        
        template <typename T, bool isCheckValue>
        struct GetterFunctionParameter : Parameter<T, isCheckValue> {
            typedef T (*GetterFunction)();
            GetterFunctionParameter(GetterFunction getter)
            : Parameter<T, isCheckValue>(dummy)
            , getter(getter) {}
            
        protected:
            virtual type_ref(T) get() { return dummy = getter(); }
            GetterFunction getter;
            remove_ref(T) dummy;
        };
        
        template <typename Base, size_t size, bool isCheckValue>
        struct GetterFunctionParameter<Base(&)[size], isCheckValue> : Parameter<Base(&)[size], isCheckValue>  {
            typedef Base (&T)[size];
            typedef T (*GetterFunction)();
            GetterFunctionParameter(GetterFunction getter)
            : Parameter<Base[size], isCheckValue>(dummy)
            , getter(getter) {}
            
        protected:
            virtual Base (&get())[size] {
                Base (&arr)[size] = getter();
                for(size_t i = 0; i < size; i++) dummy[i] = arr[i];
                return dummy;
            }
            GetterFunction getter;
            Base dummy[size];
        };
        
        template <typename T, typename C, bool isCheckValue>
        struct GetterParameter : Parameter<T, isCheckValue> {
            typedef T (C::*Getter)();
            
            GetterParameter(C &that, Getter getter)
            : Parameter<T, isCheckValue>(dummy)
            , getter(getter)
            , that(that) {}
            
        protected:
            virtual type_ref(T) get() { return dummy = (that.*getter)(); }
            Getter getter;
            C &that;
            remove_ref(T) dummy;
        };

        template <typename Base, size_t size, typename C, bool isCheckValue>
        struct GetterParameter<Base(&)[size], C, isCheckValue> : Parameter<Base(&)[size], isCheckValue> {
            typedef Base (&T)[size];
            typedef T (C::*Getter)();
            
            GetterParameter(C &that, Getter getter)
            : Parameter<T, isCheckValue>(dummy)
            , getter(getter)
            , that(that) {}

        protected:
            virtual T get() {
                T arr = (that.*getter)();
                for(size_t i = 0; i < size; i++) dummy[i] = arr[i];
                return dummy;
            }
            Getter getter;
            C &that;
            Base dummy[size];
        };
        
        template <typename T, typename C, bool isCheckValue>
        struct ConstGetterParameter : Parameter<T, isCheckValue> {
            typedef T (C::*Getter)() const;
            
            ConstGetterParameter(const C &that, Getter getter)
            : Parameter<T, isCheckValue>(dummy)
            , getter(getter)
            , that(that) {}
            
        protected:
            virtual type_ref(T) get() { return dummy = (that.*getter)(); }
            Getter getter;
            const C &that;
            remove_ref(T) dummy;
        };
        
        template <typename Base, size_t size, typename C, bool isCheckValue>
        struct ConstGetterParameter<Base(&)[size], C, isCheckValue> : Parameter<Base(&)[size], isCheckValue> {
            typedef Base (&T)[size];
            typedef T (C::*Getter)() const;
            
            ConstGetterParameter(const C &that, Getter getter)
            : Parameter<T, isCheckValue>(dummy)
            , getter(getter)
            , that(that) {}
            
        protected:
            virtual T get() {
                T arr = (that.*getter)();
                for(size_t i = 0; i < size; i++) dummy[i] = arr[i];
                return dummy;
            }
            Getter getter;
            const C &that;
            Base dummy[size];
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

            template <typename T, typename C>
            void publish(const string &address, C &that, T (C::*getter)(), bool whenValueIsChanged = true) {
                ParameterRef p;
                if(whenValueIsChanged) p = ParameterRef(new GetterParameter<T, C, true>(that, getter));
                else                   p = ParameterRef(new GetterParameter<T, C, false>(that, getter));
                publish(address, p);
            }
            
            template <typename T, typename C>
            void publish(const string &address, const C &that, T (C::*getter)() const, bool whenValueIsChanged = true) {
                ParameterRef p;
                if(whenValueIsChanged) p = ParameterRef(new ConstGetterParameter<T, C, true>(that, getter));
                else                   p = ParameterRef(new ConstGetterParameter<T, C, false>(that, getter));
                publish(address, p);
            }

#pragma mark publish conditional
#pragma mark condition is bool value ref
            
            template <typename T>
            void publishIf(bool &condition, const string &address, T &value) {
                ParameterRef p = ParameterRef(new Parameter<T, false>(value));
                p->setCondition(shared_ptr<BasicCondition>(new ConditionRef(condition)));
                publish(address, p);
            }

            template <typename T>
            void publishIf(bool &condition, const string &address, T (*getter)()) {
                ParameterRef p = ParameterRef(new GetterFunctionParameter<T, false>(getter));
                p->setCondition(shared_ptr<BasicCondition>(new ConditionRef(condition)));
                publish(address, p);
            }
            
            template <typename T, typename C>
            void publishIf(bool &condition, const string &address, C &that, T (C::*getter)()) {
                ParameterRef p = ParameterRef(new GetterParameter<T, C, false>(that, getter));
                p->setCondition(shared_ptr<BasicCondition>(new ConditionRef(condition)));
                publish(address, p);
            }

            template <typename T, typename C>
            void publishIf(bool &condition, const string &address, const C &that, T (C::*getter)() const) {
                ParameterRef p = ParameterRef(new ConstGetterParameter<T, C, false>(that, getter));
                p->setCondition(shared_ptr<BasicCondition>(new ConditionRef(condition)));
                publish(address, p);
            }
            
#pragma mark condition is function
            
            template <typename T>
            void publishIf(bool (*condition)(), const string &address, T &value) {
                ParameterRef p = ParameterRef(new Parameter<T, false>(value));
                p->setCondition(shared_ptr<BasicCondition>(new ConditionFunction(condition)));
                publish(address, p);
            }
            
            template <typename T>
            void publishIf(bool (*condition)(), const string &address, T (*getter)()) {
                ParameterRef p = ParameterRef(new GetterFunctionParameter<T, false>(getter));
                p->setCondition(shared_ptr<BasicCondition>(new ConditionFunction(condition)));
                publish(address, p);
            }
            
            template <typename T, typename C>
            void publishIf(bool (*condition)(), const string &address, C &that, T (C::*getter)()) {
                ParameterRef p = ParameterRef(new GetterParameter<T, C, false>(that, getter));
                p->setCondition(shared_ptr<BasicCondition>(new ConditionFunction(condition)));
                publish(address, p);
            }

            template <typename T, typename C>
            void publishIf(bool (*condition)(), const string &address, const C &that, T (C::*getter)() const) {
                ParameterRef p = ParameterRef(new ConstGetterParameter<T, C, false>(that, getter));
                p->setCondition(shared_ptr<BasicCondition>(new ConditionFunction(condition)));
                publish(address, p);
            }
            
#pragma mark condition is method
            
            template <typename T, typename Condition>
            void publishIf(Condition &condition, bool (Condition::*method)(), const string &address, T &value) {
                ParameterRef p = ParameterRef(new Parameter<T, true>(value));
                p->setCondition(shared_ptr<BasicCondition>(new ConditionMethod<Condition>(condition, method)));
                publish(address, p);
            }
            
            template <typename T, typename Condition>
            void publishIf(Condition &condition, bool (Condition::*method)(), const string &address, T (*getter)()) {
                ParameterRef p = ParameterRef(new GetterFunctionParameter<T, true>(getter));
                p->setCondition(shared_ptr<BasicCondition>(new ConditionMethod<Condition>(condition, method)));
                publish(address, p);
            }
            
            template <typename T, typename C, typename Condition>
            void publishIf(Condition &condition, bool (Condition::*method)(), const string &address, C &that, T (C::*getter)()) {
                ParameterRef p = ParameterRef(new GetterParameter<T, C, true>(that, getter));
                p->setCondition(shared_ptr<BasicCondition>(new ConditionMethod<Condition>(condition, method)));
                publish(address, p);
            }
            
            template <typename T, typename C, typename Condition>
            void publishIf(Condition &condition, bool (Condition::*method)(), const string &address, const C &that, T (C::*getter)() const) {
                ParameterRef p = ParameterRef(new ConstGetterParameter<T, C, true>(that, getter));
                p->setCondition(shared_ptr<BasicCondition>(new ConditionMethod<Condition>(condition, method)));
                publish(address, p);
            }
            
#pragma mark condition is const method
            
            template <typename T, typename Condition>
            void publishIf(const Condition &condition, bool (Condition::*method)() const, const string &address, T &value) {
                ParameterRef p = ParameterRef(new Parameter<T, true>(value));
                p->setCondition(shared_ptr<BasicCondition>(new ConstConditionMethod<Condition>(condition, method)));
                publish(address, p);
            }
            
            template <typename T, typename Condition>
            void publishIf(const Condition &condition, bool (Condition::*method)() const, const string &address, T (*getter)()) {
                ParameterRef p = ParameterRef(new GetterFunctionParameter<T, true>(getter));
                p->setCondition(shared_ptr<BasicCondition>(new ConstConditionMethod<Condition>(condition, method)));
                publish(address, p);
            }
            
            template <typename T, typename C, typename Condition>
            void publishIf(const Condition &condition, bool (Condition::*method)() const, const string &address, C &that, T (C::*getter)()) {
                ParameterRef p = ParameterRef(new GetterParameter<T, C, true>(that, getter));
                p->setCondition(shared_ptr<BasicCondition>(new ConstConditionMethod<Condition>(condition, method)));
                publish(address, p);
            }
            
            template <typename T, typename C, typename Condition>
            void publishIf(const Condition &condition, bool (Condition::*method)() const, const string &address, const C &that, T (C::*getter)() const) {
                ParameterRef p = ParameterRef(new ConstGetterParameter<T, C, true>(that, getter));
                p->setCondition(shared_ptr<BasicCondition>(new ConstConditionMethod<Condition>(condition, method)));
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
            
#pragma mark doRegister
            
            inline void doRegister(const string &address, ParameterRef ref) {
                if(registeredTargets.find(address) == registeredTargets.end()) {
                    registeredTargets.insert(make_pair(address, ref));
                } else {
                    registeredTargets[address] = ref;
                }
            }
            
            template <typename T>
            void doRegister(const string &address, T &value) {
                doRegister(address, ParameterRef(new Parameter<T, false>(value)));
            }
            
            template <typename T>
            void doRegister(const string &address, T (*getter)()) {
                doRegister(address, ParameterRef(new GetterFunctionParameter<T, false>(getter)));
            }
            
            template <typename T, typename C>
            void doRegister(const string &address, C &that, T (C::*getter)()) {
                doRegister(address, ParameterRef(new GetterParameter<T, C, false>(that, getter)));
            }
            
            template <typename T, typename C>
            void doRegister(const string &address, const C &that, T (C::*getter)() const) {
                doRegister(address, ParameterRef(new ConstGetterParameter<T, C, false>(that, getter)));
            }
            
#pragma mark publishRegistered
            
            inline void publishRegistered(const string &address) {
                Targets::iterator it = registeredTargets.find(address);
                if(it == registeredTargets.end()) {
                    ofLogWarning("ofxPubSubOsc") << address << " is not registered.";
                }
                ofxOscMessage m;
                if(it->second->setMessage(m, it->first)) sender.sendMessage(m);
                m.clear();
            }
            
#pragma mark unregister
            
            inline void unregister(const string &address) {
                if(registeredTargets.find(address) == registeredTargets.end()) registeredTargets.erase(address);
            }
            
            inline void unregister() {
                registeredTargets.clear();
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
            
            inline bool isRegistered() const {
                return !registeredTargets.empty();
            }
            
            inline bool isRegistered(const string &address) const {
                return isRegistered() && (registeredTargets.find(address) != registeredTargets.end());
            }
            
            typedef shared_ptr<OscPublisher> Ref;
            
            static void setUseBundle(bool b) {
                bUseBundle = b;
            }
            
            static bool isUseBundle() {
                return bUseBundle;
            }
            
        private:
            OscPublisher(const SenderKey &key) : key(key) {
                sender.setup(key.first, key.second);
            }
            
            void update() {
                ofxOscMessage m;
                if(isUseBundle()) {
                    ofxOscBundle bundle;
                    for(Targets::iterator it = targets.begin(); it != targets.end(); it++) {
                        if(it->second->setMessage(m, it->first)) bundle.addMessage(m);
                        m.clear();
                    }
                    if(bundle.getMessageCount()) sender.sendBundle(bundle);
                    bundle.clear();
                    return;
                }
                for(Targets::iterator it = targets.begin(); it != targets.end(); it++) {
                    if(it->second->setMessage(m, it->first)) sender.sendMessage(m);
                    m.clear();
                }
                
            }
            
            SenderKey key;
            ofxOscSender sender;
            Targets targets;
            Targets registeredTargets;
            static bool bUseBundle;
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
    
    bool OscPublisherManager::OscPublisher::bUseBundle = false;
};
#undef type_ref

#pragma mark - syntax sugars

#pragma mark getter

typedef ofx::OscPublisherManager ofxOscPublisherManager;
typedef ofxOscPublisherManager::OscPublisher ofxOscPublisher;

/// \brief get a OscPublisher.
/// \param ip target ip is typed const string &
/// \param port target port is typed int
/// \returns ofxOscPublisher binded to ip & port

inline ofxOscPublisher &ofxGetOscPublisher(const string &ip, int port) {
    return ofxOscPublisherManager::getOscPublisher(ip, port);
}

#pragma mark publish

/// \name ofxPublishOsc
/// \{

/// \brief publish value as an OSC message with an address pattern address to ip:port every time the value has changed. 
/// If whenValueIsChanged is set to false, then the binded value is sent every frame after App::update.
/// template parameter T is suggested by value
/// \param ip target ip is typed const string &
/// \param port target port is typed int
/// \param address osc address is typed const string &
/// \param value reference of value is typed T &
/// \param whenValueIsChanged if this value to false, then we send value every update
/// \returns void

template <typename T>
inline void ofxPublishOsc(const string &ip, int port, const string &address, T &value, bool whenValueIsChanged = true) {
    ofxGetOscPublisher(ip, port).publish(address, value, whenValueIsChanged);
}

/// \brief publish the value will be gave by function as an OSC message with an address pattern address to ip:port every time the value has changed.
///  If whenValueIsChanged is set to false, then the binded value is sent every frame after App::update.
/// template parameter T is suggested by value
/// \param ip target ip is typed const string &
/// \param port target port is typed int
/// \param address osc address is typed const string &
/// \param getter this function gives value, is typed T(*)()
/// \param whenValueIsChanged if this value to false, then we send value every update
/// \returns void

template <typename T>
inline void ofxPublishOsc(const string &ip, int port, const string &address, T (*getter)(), bool whenValueIsChanged = true) {
    ofxGetOscPublisher(ip, port).publish(address, getter, whenValueIsChanged);
}

/// \brief publish the value will be gave by function as an OSC message with an address pattern address to ip:port every time the value has changed.
/// If whenValueIsChanged is set to false, then the binded value is sent every frame after App::update.
/// template parameter T is suggested by value and U is suggested by that and getter.
/// \param ip target ip is typed const string &
/// \param port target port is typed int
/// \param address osc address is typed const string &
/// \param that this object is typed U*, will bind with next parameter method. is called as (that->*getter)().
/// \param getter this method gives value, is typed T(C::*)()
/// \param whenValueIsChanged if this value to false, then we send value every update
/// \returns void

template <typename T, typename C>
inline void ofxPublishOsc(const string &ip, int port, const string &address, C *that, T (C::*getter)(), bool whenValueIsChanged = true) {
    ofxGetOscPublisher(ip, port).publish(address, *that, getter, whenValueIsChanged);
}

/// \brief publish the value will be gave by function as an OSC message with an address pattern address to ip:port every time the value has changed.
/// If whenValueIsChanged is set to false, then the binded value is sent every frame after App::update.
/// template parameter T is suggested by value and U is suggested by that and getter.
/// \param ip target ip is typed const string &
/// \param port target port is typed int
/// \param address osc address is typed const string &
/// \param that this object is typed U*, will bind with next parameter method. is called as (that->*getter)().
/// \param getter this method gives value, is typed T(C::*)() const
/// \param whenValueIsChanged if this value to false, then we send value every update
/// \returns void

template <typename T, typename C>
inline void ofxPublishOsc(const string &ip, int port, const string &address, const C * const that, T (C::*getter)() const, bool whenValueIsChanged = true) {
    ofxGetOscPublisher(ip, port).publish(address, *that, getter, whenValueIsChanged);
}

/// \brief publish the value will be gave by function as an OSC message with an address pattern address to ip:port every time the value has changed.
/// If whenValueIsChanged is set to false, then the binded value is sent every frame after App::update.
/// template parameter T is suggested by value and U is suggested by that and getter.
/// \param ip target ip is typed const string &
/// \param port target port is typed int
/// \param address osc address is typed const string &
/// \param that this object is typed U&, will bind with next parameter method. is called as (that.*getter)()
/// \param getter this method gives value, is typed T(C::*)()
/// \param whenValueIsChanged if this value to false, then we send value every update
/// \returns void

template <typename T, typename C>
inline void ofxPublishOsc(const string &ip, int port, const string &address, C &that, T (C::*getter)(), bool whenValueIsChanged = true) {
    ofxGetOscPublisher(ip, port).publish(address, that, getter, whenValueIsChanged);
}

/// \brief publish the value will be gave by function as an OSC message with an address pattern address to ip:port every time the value has changed.
/// If whenValueIsChanged is set to false, then the binded value is sent every frame after App::update.
/// template parameter T is suggested by value and U is suggested by that and getter.
/// \param ip target ip is typed const string &
/// \param port target port is typed int
/// \param address osc address is typed const string &
/// \param that this object is typed U&, will bind with next parameter method. is called as (that.*getter)()
/// \param getter this method gives value, is typed T(C::*)() const
/// \param whenValueIsChanged if this value to false, then we send value every update
/// \returns void

template <typename T, typename C>
inline void ofxPublishOsc(const string &ip, int port, const string &address, const C &that, T (C::*getter)() const, bool whenValueIsChanged = true) {
    ofxGetOscPublisher(ip, port).publish(address, that, getter, whenValueIsChanged);
}
/// \}

#pragma mark publish if condition

/// \name ofxPublishOscif
/// \{

/// \brief publish value as an OSC message with an address pattern address to ip:port when condition is true.
/// template parameter T is suggested by value
/// \param condition condition of publish typed bool &
/// \param ip target ip is typed const string &
/// \param port target port is typed int
/// \param address osc address is typed const string &
/// \param value reference of value is typed T &
/// \param whenValueIsChanged if this value to false, then we send value every update
/// \returns void

template <typename T>
inline void ofxPublishOscIf(bool &condition, const string &ip, int port, const string &address, T &value) {
    ofxGetOscPublisher(ip, port).publishIf(condition, address, value);
}

/// \brief publish value will be gave by function as an OSC message with an address pattern address to ip:port when condition is true.
/// template parameter T is suggested by value
/// \param condition condition of publish typed bool &
/// \param ip target ip is typed const string &
/// \param port target port is typed int
/// \param address osc address is typed const string &
/// \param getter this function gives value, is typed T(*)()
/// \param whenValueIsChanged if this value to false, then we send value every update
/// \returns void

template <typename T>
inline void ofxPublishOscIf(bool &condition, const string &ip, int port, const string &address, T (*getter)()) {
    ofxGetOscPublisher(ip, port).publishIf(condition, address, getter);
}

template <typename T, typename C>
inline void ofxPublishOscIf(bool &condition, const string &ip, int port, const string &address, C *that, T (C::*getter)()) {
    ofxGetOscPublisher(ip, port).publishIf(condition, address, *that, getter);
}

template <typename T, typename C>
inline void ofxPublishOscIf(bool &condition, const string &ip, int port, const string &address, const C * const that, T (C::*getter)() const) {
    ofxGetOscPublisher(ip, port).publishIf(condition, address, *that, getter);
}

template <typename T, typename C>
inline void ofxPublishOscIf(bool &condition, const string &ip, int port, const string &address, C &that, T (C::*getter)()) {
    ofxGetOscPublisher(ip, port).publishIf(condition, address, that, getter);
}

template <typename T, typename C>
inline void ofxPublishOscIf(bool &condition, const string &ip, int port, const string &address, const C &that, T (C::*getter)() const) {
    ofxGetOscPublisher(ip, port).publishIf(condition, address, that, getter);
}

#pragma mark condition is function

template <typename T>
inline void ofxPublishOscIf(bool (*condition)(), const string &ip, int port, const string &address, T &value) {
    ofxGetOscPublisher(ip, port).publishIf(condition, address, value);
}

template <typename T>
inline void ofxPublishOscIf(bool (*condition)(), const string &ip, int port, const string &address, T (*getter)()) {
    ofxGetOscPublisher(ip, port).publishIf(condition, address, getter);
}

template <typename T, typename C>
inline void ofxPublishOscIf(bool (*condition)(), const string &ip, int port, const string &address, C *that, T (C::*getter)()) {
    ofxGetOscPublisher(ip, port).publishIf(condition, address, *that, getter);
}

template <typename T, typename C>
inline void ofxPublishOscIf(bool (*condition)(), const string &ip, int port, const string &address, const C * const that, T (C::*getter)() const) {
    ofxGetOscPublisher(ip, port).publishIf(condition, address, *that, getter);
}

template <typename T, typename C>
inline void ofxPublishOscIf(bool (*condition)(), const string &ip, int port, const string &address, C &that, T (C::*getter)()) {
    ofxGetOscPublisher(ip, port).publishIf(condition, address, that, getter);
}

template <typename T, typename C>
inline void ofxPublishOscIf(bool (*condition)(), const string &ip, int port, const string &address, const C &that, T (C::*getter)() const) {
    ofxGetOscPublisher(ip, port).publishIf(condition, address, that, getter);
}

#pragma mark condition is method

template <typename T, typename Condition>
inline void ofxPublishOscIf(Condition *condition, bool (Condition::*method)(), const string &ip, int port, const string &address, T &value) {
    ofxGetOscPublisher(ip, port).publishIf(*condition, method, address, value);
}

template <typename T, typename Condition>
inline void ofxPublishOscIf(Condition &condition, bool (Condition::*method)(), const string &ip, int port, const string &address, T &value) {
    ofxGetOscPublisher(ip, port).publishIf(condition, method, address, value);
}

template <typename T, typename Condition>
inline void ofxPublishOscIf(Condition *condition, bool (Condition::*method)(), const string &ip, int port, const string &address, T (*getter)()) {
    ofxGetOscPublisher(ip, port).publishIf(*condition, method, address, getter);
}

template <typename T, typename Condition>
inline void ofxPublishOscIf(Condition &condition, bool (Condition::*method)(), const string &ip, int port, const string &address, T (*getter)()) {
    ofxGetOscPublisher(ip, port).publishIf(condition, method, address, getter);
}

template <typename T, typename C, typename Condition>
inline void ofxPublishOscIf(Condition *condition, bool (Condition::*method)(), const string &ip, int port, const string &address, C *that, T (C::*getter)()) {
    ofxGetOscPublisher(ip, port).publishIf(*condition, method, address, *that, getter);
}

template <typename T, typename C, typename Condition>
inline void ofxPublishOscIf(Condition *condition, bool (Condition::*method)(), const string &ip, int port, const string &address, const C * const that, T (C::*getter)() const) {
    ofxGetOscPublisher(ip, port).publishIf(*condition, method, address, *that, getter);
}

template <typename T, typename C, typename Condition>
inline void ofxPublishOscIf(Condition &condition, bool (Condition::*method)(), const string &ip, int port, const string &address, C *that, T (C::*getter)()) {
    ofxGetOscPublisher(ip, port).publishIf(condition, method, address, *that, getter);
}

template <typename T, typename C, typename Condition>
inline void ofxPublishOscIf(Condition &condition, bool (Condition::*method)(), const string &ip, int port, const string &address, const C * const that, T (C::*getter)() const) {
    ofxGetOscPublisher(ip, port).publishIf(condition, method, address, *that, getter);
}

template <typename T, typename C, typename Condition>
inline void ofxPublishOscIf(Condition *condition, bool (Condition::*method)(), const string &ip, int port, const string &address, C &that, T (C::*getter)()) {
    ofxGetOscPublisher(ip, port).publishIf(*condition, method, address, that, getter);
}

template <typename T, typename C, typename Condition>
inline void ofxPublishOscIf(Condition *condition, bool (Condition::*method)(), const string &ip, int port, const string &address, const C &that, T (C::*getter)() const) {
    ofxGetOscPublisher(ip, port).publishIf(*condition, method, address, that, getter);
}

template <typename T, typename C, typename Condition>
inline void ofxPublishOscIf(Condition &condition, bool (Condition::*method)(), const string &ip, int port, const string &address, C &that, T (C::*getter)()) {
    ofxGetOscPublisher(ip, port).publishIf(condition, method, address, that, getter);
}

template <typename T, typename C, typename Condition>
inline void ofxPublishOscIf(Condition &condition, bool (Condition::*method)(), const string &ip, int port, const string &address, const C &that, T (C::*getter)() const) {
    ofxGetOscPublisher(ip, port).publishIf(condition, method, address, that, getter);
}

#pragma mark condition is const method

template <typename T, typename Condition>
inline void ofxPublishOscIf(const Condition * const condition, bool (Condition::*method)() const, const string &ip, int port, const string &address, T &value) {
    ofxGetOscPublisher(ip, port).publishIf(*condition, method, address, value);
}

template <typename T, typename Condition>
inline void ofxPublishOscIf(const Condition &condition, bool (Condition::*method)() const, const string &ip, int port, const string &address, T &value) {
    ofxGetOscPublisher(ip, port).publishIf(condition, method, address, value);
}

template <typename T, typename Condition>
inline void ofxPublishOscIf(const Condition * const condition, bool (Condition::*method)() const, const string &ip, int port, const string &address, T (*getter)()) {
    ofxGetOscPublisher(ip, port).publishIf(*condition, method, address, getter);
}

template <typename T, typename Condition>
inline void ofxPublishOscIf(const Condition &condition, bool (Condition::*method)() const, const string &ip, int port, const string &address, T (*getter)()) {
    ofxGetOscPublisher(ip, port).publishIf(condition, method, address, getter);
}

template <typename T, typename C, typename Condition>
inline void ofxPublishOscIf(const Condition * const condition, bool (Condition::*method)() const, const string &ip, int port, const string &address, C *that, T (C::*getter)()) {
    ofxGetOscPublisher(ip, port).publishIf(*condition, method, address, *that, getter);
}

template <typename T, typename C, typename Condition>
inline void ofxPublishOscIf(const Condition * const condition, bool (Condition::*method)() const, const string &ip, int port, const string &address, const C * const that, T (C::*getter)() const) {
    ofxGetOscPublisher(ip, port).publishIf(*condition, method, address, *that, getter);
}

template <typename T, typename C, typename Condition>
inline void ofxPublishOscIf(const Condition &condition, bool (Condition::*method)() const, const string &ip, int port, const string &address, C *that, T (C::*getter)()) {
    ofxGetOscPublisher(ip, port).publishIf(condition, method, address, *that, getter);
}

template <typename T, typename C, typename Condition>
inline void ofxPublishOscIf(const Condition &condition, bool (Condition::*method)() const, const string &ip, int port, const string &address, const C * const that, T (C::*getter)() const) {
    ofxGetOscPublisher(ip, port).publishIf(condition, method, address, *that, getter);
}

template <typename T, typename C, typename Condition>
inline void ofxPublishOscIf(const Condition * const condition, bool (Condition::*method)() const, const string &ip, int port, const string &address, C &that, T (C::*getter)()) {
    ofxGetOscPublisher(ip, port).publishIf(*condition, method, address, that, getter);
}

template <typename T, typename C, typename Condition>
inline void ofxPublishOscIf(const Condition * const condition, bool (Condition::*method)() const, const string &ip, int port, const string &address, const C &that, T (C::*getter)() const) {
    ofxGetOscPublisher(ip, port).publishIf(*condition, method, address, that, getter);
}

template <typename T, typename C, typename Condition>
inline void ofxPublishOscIf(const Condition &condition, bool (Condition::*method)() const, const string &ip, int port, const string &address, C &that, T (C::*getter)()) {
    ofxGetOscPublisher(ip, port).publishIf(condition, method, address, that, getter);
}

template <typename T, typename C, typename Condition>
inline void ofxPublishOscIf(const Condition &condition, bool (Condition::*method)() const, const string &ip, int port, const string &address, const C &that, T (C::*getter)() const) {
    ofxGetOscPublisher(ip, port).publishIf(condition, method, address, that, getter);
}

/// \}

#pragma mark unpublish

/// \name ofxUnpublishOsc
/// \{

inline void ofxUnpublishOsc(const string &ip, int port, const string &address) {
    ofxGetOscPublisher(ip, port).unpublish(address);
}

inline void ofxUnpublishOsc(const string &ip, int port) {
    ofxGetOscPublisher(ip, port).unpublish();
}

/// \}

#pragma mark register

template <typename T>
inline void ofxRegisterPublishingOsc(const string &ip, int port, const string &address, T &value) {
    ofxGetOscPublisher(ip, port).doRegister(address, value);
}

template <typename T>
inline void ofxRegisterPublishingOsc(const string &ip, int port, const string &address, T (*getter)()) {
    ofxGetOscPublisher(ip, port).doRegister(address, getter);
}

template <typename T, typename C>
inline void ofxRegisterPublishingOsc(const string &ip, int port, const string &address, C *that, T (C::*getter)()) {
    ofxGetOscPublisher(ip, port).doRegister(address, *that, getter);
}

template <typename T, typename C>
inline void ofxRegisterPublishingOsc(const string &ip, int port, const string &address, const C * const that, T (C::*getter)() const) {
    ofxGetOscPublisher(ip, port).doRegister(address, *that, getter);
}

template <typename T, typename C>
inline void ofxRegisterPublishingOsc(const string &ip, int port, const string &address, C &that, T (C::*getter)()) {
    ofxGetOscPublisher(ip, port).doRegister(address, that, getter);
}

template <typename T, typename C>
inline void ofxRegisterPublishingOsc(const string &ip, int port, const string &address, const C &that, T (C::*getter)() const) {
    ofxGetOscPublisher(ip, port).doRegister(address, that, getter);
}

#pragma mark publish registered

inline void ofxPublishRegisteredOsc(const string &ip, int port, const string &address) {
    ofxGetOscPublisher(ip, port).publishRegistered(address);
}

#pragma mark unregister

inline void ofxUnregisterPublishingOsc(const string &ip, int port, const string &address) {
    ofxGetOscPublisher(ip, port).unregister(address);
}

inline void ofxUnregisterPublishingOsc(const string &ip, int port) {
    ofxGetOscPublisher(ip, port).unregister();
}

#pragma mark using bundle option

inline void ofxPublisherSetUsingBundle(bool bUseBundle) {
    ofxOscPublisher::setUseBundle(bUseBundle);
}

#pragma mark helper for publish array

/// \name helper for publish array
/// \{

template <typename T, size_t size>
struct array_type {
    typedef T (&type)[size];
    typedef type (*fun)();
    template <typename U>
    struct meth {
        typedef type (U::*method)();
        typedef type (U::*const_method)() const;
    };
};

/// \}

/// \name ofxPublishAsArray
/// \{

template <typename T, size_t size>
typename array_type<T, size>::type ofxPublishAsArray(T *ptr) {
    return reinterpret_cast<T (&)[size]>(reinterpret_cast<T&>(ptr[0]));
}

template <typename T, size_t size>
typename array_type<T, size>::fun ofxPublishAsArray(T *(*getter)()) {
    return reinterpret_cast<typename array_type<T, size>::type (*)()>(
        reinterpret_cast<T& (*)()>(getter)
    );
}

template <typename T, size_t size, typename U>
typename array_type<T, size>::template meth<U>::method ofxPublishAsArray(T *(U::*getter)()) {
    return reinterpret_cast<typename array_type<T, size>::template meth<U>::method>(
        reinterpret_cast<T& (U::*)()>(getter)
    );
}

template <typename T, size_t size, typename U>
typename array_type<T, size>::template meth<U>::const_method ofxPublishAsArray(T *(U::*getter)() const) {
    return reinterpret_cast<typename array_type<T, size>::template meth<U>::const_method>(
        reinterpret_cast<T& (U::*)() const>(getter)
    );
}

/// \}
