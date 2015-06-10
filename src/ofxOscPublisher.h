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
    
    template <typename T>
    struct remove_reference {
        typedef T type;
    };
    template <typename T>
    struct remove_reference<T &> {
        typedef T type;
    };
#define RemoveRef(T) typename ofx::remove_reference<T>::type

    class OscPublisherManager {
    private:
        struct SetImplementation {
        protected:
#define define_set_int(type) inline void set(ofxOscMessage &m, type v) const { if(sizeof(type) < 8) m.addIntArg(v); else m.addInt64Arg(v); }
            define_set_int(bool);
            define_set_int(short);
            define_set_int(unsigned short);
            define_set_int(int);
            define_set_int(unsigned int);
            define_set_int(long);
            define_set_int(unsigned long);
            define_set_int(long long);
            define_set_int(unsigned long long);
#undef define_set_int
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
            ConditionMethod(T &that, bool (T::*getter)())
            : AbstractCondition()
            , that(that)
            , getter(getter) {};
            
            virtual bool inner_condition() { return (that.*getter)(); };
        private:
            T &that; // dirty!!!
            bool (T::*getter)();
        };

        template <typename T>
        struct ConstConditionMethod : AbstractCondition {
            ConstConditionMethod(const T &that, bool (T::*getter)() const)
            : AbstractCondition()
            , that(that)
            , getter(getter) {};
            
            virtual bool inner_condition() { return (that.*getter)(); };
        private:
            const T &that;
            bool (T::*getter)() const;
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
                if(!canPublish() || !isChanged()) return;
                ofxOscMessage m;
                m.setAddress(address);
                set(m, get());
                sender.sendMessage(m);
            }

        protected:
            virtual bool isChanged() { return true; }
            virtual TypeRef(T) get() { return t; }
            T &t;
        };

        template <typename T>
        struct Parameter<T, true> : Parameter<T, false> {
            Parameter(T &t)
            : Parameter<T, false>(t) {}
            
        protected:
            virtual inline bool isChanged() {
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
            
            virtual void send(ofxOscSender &sender, const string &address) {
                if(!canPublish() || !isChanged()) return;
                ofxOscMessage m;
                m.setAddress(address);
                set(m, get());
                sender.sendMessage(m);
            }
            
        protected:
            inline bool isChanged() {
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
            virtual TypeRef(T) get() { return dummy = getter(); }
            GetterFunction getter;
            RemoveRef(T) dummy;
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
            virtual TypeRef(T) get() { return dummy = (that.*getter)(); }
            Getter getter;
            C &that;
            RemoveRef(T) dummy;
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
            virtual TypeRef(T) get() { return dummy = (that.*getter)(); }
            Getter getter;
            const C &that;
            RemoveRef(T) dummy;
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
                p->setCondition(shared_ptr<AbstractCondition>(new ConditionRef(condition)));
                publish(address, p);
            }

            template <typename T>
            void publishIf(bool &condition, const string &address, T (*getter)()) {
                ParameterRef p = ParameterRef(new GetterFunctionParameter<T, false>(getter));
                p->setCondition(shared_ptr<AbstractCondition>(new ConditionRef(condition)));
                publish(address, p);
            }
            
            template <typename T, typename C>
            void publishIf(bool &condition, const string &address, C &that, T (C::*getter)()) {
                ParameterRef p = ParameterRef(new GetterParameter<T, C, false>(that, getter));
                p->setCondition(shared_ptr<AbstractCondition>(new ConditionRef(condition)));
                publish(address, p);
            }

            template <typename T, typename C>
            void publishIf(bool &condition, const string &address, const C &that, T (C::*getter)() const) {
                ParameterRef p = ParameterRef(new ConstGetterParameter<T, C, false>(that, getter));
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
            
            template <typename T, typename C>
            void publishIf(bool (*condition)(), const string &address, C &that, T (C::*getter)()) {
                ParameterRef p = ParameterRef(new GetterParameter<T, C, false>(that, getter));
                p->setCondition(shared_ptr<AbstractCondition>(new ConditionFunction(condition)));
                publish(address, p);
            }

            template <typename T, typename C>
            void publishIf(bool (*condition)(), const string &address, const C &that, T (C::*getter)() const) {
                ParameterRef p = ParameterRef(new ConstGetterParameter<T, C, false>(that, getter));
                p->setCondition(shared_ptr<AbstractCondition>(new ConditionFunction(condition)));
                publish(address, p);
            }
            
#pragma mark condition is method
            
            template <typename T, typename Condition>
            void publishIf(Condition &condition, bool (Condition::*method)(), const string &address, T &value) {
                ParameterRef p = ParameterRef(new Parameter<T, true>(value));
                p->setCondition(shared_ptr<AbstractCondition>(new ConditionMethod<Condition>(condition, method)));
                publish(address, p);
            }
            
            template <typename T, typename Condition>
            void publishIf(Condition &condition, bool (Condition::*method)(), const string &address, T (*getter)()) {
                ParameterRef p = ParameterRef(new GetterFunctionParameter<T, true>(getter));
                p->setCondition(shared_ptr<AbstractCondition>(new ConditionMethod<Condition>(condition, method)));
                publish(address, p);
            }
            
            template <typename T, typename C, typename Condition>
            void publishIf(Condition &condition, bool (Condition::*method)(), const string &address, C &that, T (C::*getter)()) {
                ParameterRef p = ParameterRef(new GetterParameter<T, C, true>(that, getter));
                p->setCondition(shared_ptr<AbstractCondition>(new ConditionMethod<Condition>(condition, method)));
                publish(address, p);
            }
            
            template <typename T, typename C, typename Condition>
            void publishIf(Condition &condition, bool (Condition::*method)(), const string &address, const C &that, T (C::*getter)() const) {
                ParameterRef p = ParameterRef(new ConstGetterParameter<T, C, true>(that, getter));
                p->setCondition(shared_ptr<AbstractCondition>(new ConditionMethod<Condition>(condition, method)));
                publish(address, p);
            }
            
#pragma mark condition is const method
            
            template <typename T, typename Condition>
            void publishIf(const Condition &condition, bool (Condition::*method)() const, const string &address, T &value) {
                ParameterRef p = ParameterRef(new Parameter<T, true>(value));
                p->setCondition(shared_ptr<AbstractCondition>(new ConstConditionMethod<Condition>(condition, method)));
                publish(address, p);
            }
            
            template <typename T, typename Condition>
            void publishIf(const Condition &condition, bool (Condition::*method)() const, const string &address, T (*getter)()) {
                ParameterRef p = ParameterRef(new GetterFunctionParameter<T, true>(getter));
                p->setCondition(shared_ptr<AbstractCondition>(new ConstConditionMethod<Condition>(condition, method)));
                publish(address, p);
            }
            
            template <typename T, typename C, typename Condition>
            void publishIf(const Condition &condition, bool (Condition::*method)() const, const string &address, C &that, T (C::*getter)()) {
                ParameterRef p = ParameterRef(new GetterParameter<T, C, true>(that, getter));
                p->setCondition(shared_ptr<AbstractCondition>(new ConstConditionMethod<Condition>(condition, method)));
                publish(address, p);
            }
            
            template <typename T, typename C, typename Condition>
            void publishIf(const Condition &condition, bool (Condition::*method)() const, const string &address, const C &that, T (C::*getter)() const) {
                ParameterRef p = ParameterRef(new ConstGetterParameter<T, C, true>(that, getter));
                p->setCondition(shared_ptr<AbstractCondition>(new ConstConditionMethod<Condition>(condition, method)));
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
