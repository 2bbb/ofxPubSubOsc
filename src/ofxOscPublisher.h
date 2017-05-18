//
//  ofxOscPublisher.h
//
//  Created by ISHII 2bit on 2015/05/11.
//
//

#pragma once

#include "ofMain.h"
#include "ofxOsc.h"

#include "details/ofxpubsubosc_settings.h"
#include "details/ofxpubsubosc_type_utils.h"

#include <initializer_list>

namespace ofx {
    using namespace ofxpubsubosc;
    
    namespace {
        template <typename T>
        using remove_ref = typename std::remove_reference<T>::type;
        
        template <typename T>
        using type_ref = typename add_reference_if_non_arithmetic<T>::type;
        
        template <typename T>
        struct is_integral_and_lt_64bit {
            static const bool value = std::is_integral<T>::value && (sizeof(T) < 8);
        };
        
        template <typename T>
        struct is_integral_and_geq_64bit {
            static const bool value = std::is_integral<T>::value && (8 <= sizeof(T));
        };
    };
    
    class OscPublisherManager {
    public:
        class OscPublisher;
        
    private:
        struct SetImplementation {
        protected:
            template <typename T>
            inline typename std::enable_if<is_integral_and_lt_64bit<T>::value>::type set(ofxOscMessage &m, T v) const { m.addIntArg(v); }
            
            template <typename T>
            inline typename std::enable_if<is_integral_and_geq_64bit<T>::value>::type set(ofxOscMessage &m, T v) const { m.addInt64Arg(v); }
            
#define define_set_float(type) inline void set(ofxOscMessage &m, type v) const { m.addFloatArg(v); }
            define_set_float(float);
            define_set_float(double);
#undef define_set_float
            inline void set(ofxOscMessage &m, const std::string &v) const { m.addStringArg(v); }
            inline void set(ofxOscMessage &m, const ofBuffer &v) const { m.addBlobArg(v); };
            template <typename PixType>
            inline void set(ofxOscMessage &m, const ofColor_<PixType> &v) const {  setVec<4>(m, v); }
            inline void set(ofxOscMessage &m, const ofVec2f &v) const { setVec<2>(m, v); }
            inline void set(ofxOscMessage &m, const ofVec3f &v) const { setVec<3>(m, v); }
            inline void set(ofxOscMessage &m, const ofVec4f &v) const { setVec<4>(m, v); }
            inline void set(ofxOscMessage &m, const ofQuaternion &v) const { setVec<4>(m, v); }
            
            template <std::size_t n, typename T>
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
            
            template <typename U, std::size_t size>
            inline void set(ofxOscMessage &m, const U (&v)[size]) const {
                for(int i = 0; i < size; i++) { set(m, v[i]); }
            }
            
            template <typename U>
            inline void set(ofxOscMessage &m, const std::vector<U> &v) const {
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
            
            using Ref = std::shared_ptr<BasicCondition>;
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

        using BasicConditionRef = BasicCondition::Ref;
        
#pragma mark Parameter
        
        struct AbstractParameter {
            AbstractParameter() : condition(new BasicCondition) {}
            virtual bool setMessage(ofxOscMessage &m, const std::string &address) = 0;
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
            
            virtual bool setMessage(ofxOscMessage &m, const std::string &address) {
                if(!canPublish() || !isChanged()) return false;
                m.setAddress(address);
                set(m, get());
                return true;
            }
        protected:
            virtual bool isChanged() { return true; }
            virtual type_ref<T> get() { return t; }
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

        template <typename Base, std::size_t size>
        struct Parameter<Base(&)[size], true> : AbstractParameter, SetImplementation {
            Parameter(Base (&t)[size])
            : t(t) { for(std::size_t i = 0; i < size; i++) old[i] = t[i]; }
            virtual ~Parameter() { };
            
            virtual bool setMessage(ofxOscMessage &m, const std::string &address) {
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
            Base (&t)[size];
            Base old[size];
        };
        
        template <typename T, bool isCheckValue>
        struct ConstParameter : AbstractParameter, SetImplementation {
            ConstParameter(const T &t)
            : t(t) {}
            
            virtual bool setMessage(ofxOscMessage &m, const std::string &address) {
                if(!canPublish() || !isChanged()) return false;
                m.setAddress(address);
                set(m, get());
                return true;
            }
        protected:
            virtual bool isChanged() { return true; }
            virtual const T &get() { return t; }
            const T t;
        };
        
        template <typename T>
        struct ConstParameter<T, true> : ConstParameter<T, false> {
            ConstParameter(const T &t)
            : ConstParameter<T, false>(t) {}
            
        protected:
            virtual bool isChanged() {
                static bool initial{true};
                bool tmp = initial;
                initial = false;
                return tmp;
            }
        };
        
        template <typename Base, std::size_t size>
        struct ConstParameter<const Base(&)[size], false> : AbstractParameter, SetImplementation {
            ConstParameter(const Base (&t)[size])
            : t(t) {}
            
            virtual bool setMessage(ofxOscMessage &m, const std::string &address) {
                if(!canPublish() || !isChanged()) return false;
                m.setAddress(address);
                set(m, get());
                return true;
            }
            
        protected:
            virtual bool isChanged() { return true; }
            
            virtual const Base (&get())[size] { return t; }
            const Base (&t)[size];
        };
        
        template <typename Base, std::size_t size>
        struct ConstParameter<const Base(&)[size], true> : ConstParameter<const Base(&)[size], false> {
            ConstParameter(const Base(&t)[size])
            : ConstParameter<const Base(&)[size], false>(t) {}
            
        protected:
            virtual bool isChanged() {
                static bool initial{true};
                bool tmp = initial;
                initial = false;
                return tmp;
            }
        };

        template <typename T, bool isCheckValue>
        struct GetterFunctionParameter : Parameter<T, isCheckValue> {
            using GetterFunction = T (*)();
            GetterFunctionParameter(GetterFunction getter)
            : Parameter<T, isCheckValue>(dummy)
            , getter(getter) {}
            
        protected:
            virtual type_ref<T> get() { return dummy = getter(); }
            GetterFunction getter;
            remove_ref<T> dummy;
        };
        
        template <typename Base, std::size_t size, bool isCheckValue>
        struct GetterFunctionParameter<Base(&)[size], isCheckValue> : Parameter<Base(&)[size], isCheckValue>  {
            using T = Base (&)[size];
            using GetterFunction = T (*)();
            GetterFunctionParameter(GetterFunction getter)
            : Parameter<Base[size], isCheckValue>(dummy)
            , getter(getter) {}
            
        protected:
            virtual Base (&get())[size] {
                Base (&arr)[size] = getter();
                for(std::size_t i = 0; i < size; i++) dummy[i] = arr[i];
                return dummy;
            }
            GetterFunction getter;
            Base dummy[size];
        };
        
        template <typename T, typename C, bool isCheckValue>
        struct GetterParameter : Parameter<T, isCheckValue> {
            using Getter = T (C::*)();
            
            GetterParameter(C &that, Getter getter)
            : Parameter<T, isCheckValue>(dummy)
            , getter(getter)
            , that(that) {}
            
        protected:
            virtual type_ref<T> get() { return dummy = (that.*getter)(); }
            Getter getter;
            C &that;
            remove_ref<T> dummy;
        };

        template <typename Base, std::size_t size, typename C, bool isCheckValue>
        struct GetterParameter<Base(&)[size], C, isCheckValue> : Parameter<Base(&)[size], isCheckValue> {
            using T = Base (&)[size];
            using Getter = T (C::*)();
            
            GetterParameter(C &that, Getter getter)
            : Parameter<T, isCheckValue>(dummy)
            , getter(getter)
            , that(that) {}

        protected:
            virtual T get() {
                T arr = (that.*getter)();
                for(std::size_t i = 0; i < size; i++) dummy[i] = arr[i];
                return dummy;
            }
            Getter getter;
            C &that;
            Base dummy[size];
        };
        
        template <typename T, typename C, bool isCheckValue>
        struct ConstGetterParameter : Parameter<T, isCheckValue> {
            using Getter = T (C::*)() const;
            
            ConstGetterParameter(const C &that, Getter getter)
            : Parameter<T, isCheckValue>(dummy)
            , getter(getter)
            , that(that) {}
            
        protected:
            virtual type_ref<T> get() { return dummy = (that.*getter)(); }
            Getter getter;
            const C &that;
            remove_ref<T> dummy;
        };
        
        template <typename Base, std::size_t size, typename C, bool isCheckValue>
        struct ConstGetterParameter<Base(&)[size], C, isCheckValue> : Parameter<Base(&)[size], isCheckValue> {
            using T = Base (&)[size];
            using Getter = T (C::*)() const;
            
            ConstGetterParameter(const C &that, Getter getter)
            : Parameter<T, isCheckValue>(dummy)
            , getter(getter)
            , that(that) {}
            
        protected:
            virtual T get() {
                T arr = (that.*getter)();
                for(std::size_t i = 0; i < size; i++) dummy[i] = arr[i];
                return dummy;
            }
            Getter getter;
            const C &that;
            Base dummy[size];
        };

        using ParameterRef = std::shared_ptr<AbstractParameter>;
        using Targets = std::multimap<std::string, ParameterRef>;
        
        public:
        struct IP {
            IP(const IP &ip)
            : ip(ip.ip) {}
            
            IP(const std::string &ip)
            : ip(ip) {}
            
            bool operator<(const IP &rhs) const {
                return ip < rhs.ip;
            }
            
            std::string ip;
        private:
            IP();
        };
        
        struct Destination {
            Destination() {}
            Destination(const Destination &destination)
            : ip(destination.ip)
            , port(destination.port) {}
            
            Destination(const std::string &ip, int port)
            : ip(ip)
            , port(port) {}
            
            inline bool operator<(const Destination &rhs) const {
                return (ip != rhs.ip) ? (ip < rhs.ip) : (port < rhs.port);
            }
            
            inline bool operator!=(const Destination &rhs) const {
                return ip != rhs.ip || port != rhs.port;
            }
            
            std::string ip;
            int port;
        private:
        };
        
        struct DestinationWithAddress {
            DestinationWithAddress(const DestinationWithAddress &destination)
            : destination(destination.destination)
            , address(destination.address) {}
            
            DestinationWithAddress(const std::string &ip, int port, const std::string &address)
            : destination(ip, port)
            , address(address) {}
            
            inline bool operator<(const DestinationWithAddress &rhs) const {
                return (destination != rhs.destination) ? (destination < rhs.destination) : (address < rhs.address);
            }
            
            operator Destination() const {
                return destination;
            }

            inline operator const Destination&() const {
                return destination;
            }
            
            Destination destination;
            std::string address;
        private:
            DestinationWithAddress();
        };

        class Identifier {
            std::string address;
            ParameterRef ref;
            Destination key;
            
            void invalidate() {
                address = "";
                ref = nullptr;
                key = Destination();
            }
        public:
            Identifier() {}
            Identifier(const std::string &address, const ParameterRef &ref, const Destination &key)
            : address(address)
            , ref(ref)
            , key(key) {}
            
            const Destination &getKey() const { return key; };
            bool isValid() const { return static_cast<bool>(ref); }

            friend class OscPublisher;
        };

    public:
        class OscPublisher {
            Targets::const_iterator findFromTargets(const Identifier &identifier, const Targets &targets) const {
                if(!identifier.isValid()) return targets.end();
                Targets::const_iterator it = targets.find(identifier.address);
                if(it != targets.end()) {
                    for(std::size_t i = 0, size = targets.count(identifier.address); i < size; ++i, ++it) {
                        if(it->second == identifier.ref) {
                            return it;
                        }
                    }
                }
                return targets.end();
            }
            
            inline Targets::const_iterator findPublished(const Identifier &identifier) const {
                return findFromTargets(identifier, targets);
            }
            
            inline Targets::const_iterator findRegistered(const Identifier &identifier) const {
                return findFromTargets(identifier, registeredTargets);
            }
            

        public:
#pragma mark publish
            
            inline Identifier publish(const std::string &address, ParameterRef ref) {
                targets.insert(std::make_pair(address, ref));
                return {address, ref, destination};
            }
            
            Identifier publish(const std::string &address, const char * const value, bool whenValueIsChanged = true) {
                ParameterRef p;
                return publish(address, std::string(value), whenValueIsChanged);
            }
            
            template <typename T>
            Identifier publish(const std::string &address, T &value, bool whenValueIsChanged = true) {
                ParameterRef p;
                if(whenValueIsChanged) p = ParameterRef(new Parameter<T, true>(value));
                else                   p = ParameterRef(new Parameter<T, false>(value));
                return publish(address, p);
            }
            
            template <typename T>
            Identifier publish(const std::string &address, const T &value, bool whenValueIsChanged = true) {
                ParameterRef p;
                if(whenValueIsChanged) p = ParameterRef(new ConstParameter<T, true>(value));
                else                   p = ParameterRef(new ConstParameter<T, false>(value));
                return publish(address, p);
            }
            
            template <typename T>
            Identifier publish(const std::string &address, T (*getter)(), bool whenValueIsChanged = true) {
                ParameterRef p;
                if(whenValueIsChanged) p = ParameterRef(new GetterFunctionParameter<T, true>(getter));
                else                   p = ParameterRef(new GetterFunctionParameter<T, false>(getter));
                return publish(address, p);
            }

            template <typename T, typename C>
            Identifier publish(const std::string &address, C *that, T (C::*getter)(), bool whenValueIsChanged = true) {
                ParameterRef p;
                if(whenValueIsChanged) p = ParameterRef(new GetterParameter<T, C, true>(*that, getter));
                else                   p = ParameterRef(new GetterParameter<T, C, false>(*that, getter));
                return publish(address, p);
            }
            
            template <typename T, typename C>
            Identifier publish(const std::string &address, C &that, T (C::*getter)(), bool whenValueIsChanged = true) {
                ParameterRef p;
                if(whenValueIsChanged) p = ParameterRef(new GetterParameter<T, C, true>(that, getter));
                else                   p = ParameterRef(new GetterParameter<T, C, false>(that, getter));
                return publish(address, p);
            }
            
            template <typename T, typename C>
            Identifier publish(const std::string &address, const C * const that, T (C::*getter)() const, bool whenValueIsChanged = true) {
                ParameterRef p;
                if(whenValueIsChanged) p = ParameterRef(new ConstGetterParameter<T, C, true>(*that, getter));
                else                   p = ParameterRef(new ConstGetterParameter<T, C, false>(*that, getter));
                return publish(address, p);
            }
            
            template <typename T, typename C>
            Identifier publish(const std::string &address, const C &that, T (C::*getter)() const, bool whenValueIsChanged = true) {
                ParameterRef p;
                if(whenValueIsChanged) p = ParameterRef(new ConstGetterParameter<T, C, true>(that, getter));
                else                   p = ParameterRef(new ConstGetterParameter<T, C, false>(that, getter));
                return publish(address, p);
            }

#pragma mark publish conditional
#pragma mark condition is bool value ref
            
            Identifier publishIf(bool &condition, const std::string &address, const char * const value) {
                ParameterRef p;
                return publishIf(condition, address, std::string(value));
            }
            
            template <typename T>
            Identifier publishIf(bool &condition, const std::string &address, T &value) {
                ParameterRef p = ParameterRef(new Parameter<T, false>(value));
                p->setCondition(std::shared_ptr<BasicCondition>(new ConditionRef(condition)));
                return publish(address, p);
            }

            template <typename T>
            Identifier publishIf(bool &condition, const std::string &address, const T &value) {
                ParameterRef p = ParameterRef(new ConstParameter<T, false>(value));
                p->setCondition(std::shared_ptr<BasicCondition>(new ConditionRef(condition)));
                return publish(address, p);
            }
            
            template <typename T>
            Identifier publishIf(bool &condition, const std::string &address, T (*getter)()) {
                ParameterRef p = ParameterRef(new GetterFunctionParameter<T, false>(getter));
                p->setCondition(std::shared_ptr<BasicCondition>(new ConditionRef(condition)));
                return publish(address, p);
            }
            
            template <typename T, typename C>
            Identifier publishIf(bool &condition, const std::string &address, C *that, T (C::*getter)()) {
                ParameterRef p = ParameterRef(new GetterParameter<T, C, false>(*that, getter));
                p->setCondition(std::shared_ptr<BasicCondition>(new ConditionRef(condition)));
                return publish(address, p);
            }

            template <typename T, typename C>
            Identifier publishIf(bool &condition, const std::string &address, C &that, T (C::*getter)()) {
                ParameterRef p = ParameterRef(new GetterParameter<T, C, false>(that, getter));
                p->setCondition(std::shared_ptr<BasicCondition>(new ConditionRef(condition)));
                return publish(address, p);
            }
            
            template <typename T, typename C>
            Identifier publishIf(bool &condition, const std::string &address, const C * const that, T (C::*getter)() const) {
                ParameterRef p = ParameterRef(new ConstGetterParameter<T, C, false>(*that, getter));
                p->setCondition(std::shared_ptr<BasicCondition>(new ConditionRef(condition)));
                return publish(address, p);
            }

            template <typename T, typename C>
            Identifier publishIf(bool &condition, const std::string &address, const C &that, T (C::*getter)() const) {
                ParameterRef p = ParameterRef(new ConstGetterParameter<T, C, false>(that, getter));
                p->setCondition(std::shared_ptr<BasicCondition>(new ConditionRef(condition)));
                return publish(address, p);
            }
            
#pragma mark condition is function
            
            template <typename T>
            Identifier publishIf(bool (*condition)(), const std::string &address, T &value) {
                ParameterRef p = ParameterRef(new Parameter<T, false>(value));
                p->setCondition(std::shared_ptr<BasicCondition>(new ConditionFunction(condition)));
                return publish(address, p);
            }
            
            template <typename T>
            Identifier publishIf(bool (*condition)(), const std::string &address, T (*getter)()) {
                ParameterRef p = ParameterRef(new GetterFunctionParameter<T, false>(getter));
                p->setCondition(std::shared_ptr<BasicCondition>(new ConditionFunction(condition)));
                return publish(address, p);
            }
            
            template <typename T, typename C>
            Identifier publishIf(bool (*condition)(), const std::string &address, C *that, T (C::*getter)()) {
                ParameterRef p = ParameterRef(new GetterParameter<T, C, false>(*that, getter));
                p->setCondition(std::shared_ptr<BasicCondition>(new ConditionFunction(condition)));
                return publish(address, p);
            }

            template <typename T, typename C>
            Identifier publishIf(bool (*condition)(), const std::string &address, C &that, T (C::*getter)()) {
                ParameterRef p = ParameterRef(new GetterParameter<T, C, false>(that, getter));
                p->setCondition(std::shared_ptr<BasicCondition>(new ConditionFunction(condition)));
                return publish(address, p);
            }
            
            template <typename T, typename C>
            Identifier publishIf(bool (*condition)(), const std::string &address, const C * const that, T (C::*getter)() const) {
                ParameterRef p = ParameterRef(new ConstGetterParameter<T, C, false>(*that, getter));
                p->setCondition(std::shared_ptr<BasicCondition>(new ConditionFunction(condition)));
                return publish(address, p);
            }

            template <typename T, typename C>
            Identifier publishIf(bool (*condition)(), const std::string &address, const C &that, T (C::*getter)() const) {
                ParameterRef p = ParameterRef(new ConstGetterParameter<T, C, false>(that, getter));
                p->setCondition(std::shared_ptr<BasicCondition>(new ConditionFunction(condition)));
                return publish(address, p);
            }
            
#pragma mark condition is method
            
            template <typename T, typename Condition>
            Identifier publishIf(Condition &condition, bool (Condition::*method)(), const std::string &address, T &value) {
                ParameterRef p = ParameterRef(new Parameter<T, true>(value));
                p->setCondition(std::shared_ptr<BasicCondition>(new ConditionMethod<Condition>(condition, method)));
                return publish(address, p);
            }
            
            template <typename T, typename Condition>
            Identifier publishIf(Condition &condition, bool (Condition::*method)(), const std::string &address, T (*getter)()) {
                ParameterRef p = ParameterRef(new GetterFunctionParameter<T, true>(getter));
                p->setCondition(std::shared_ptr<BasicCondition>(new ConditionMethod<Condition>(condition, method)));
                return publish(address, p);
            }
            
            template <typename T, typename C, typename Condition>
            Identifier publishIf(Condition &condition, bool (Condition::*method)(), const std::string &address, C *that, T (C::*getter)()) {
                ParameterRef p = ParameterRef(new GetterParameter<T, C, true>(*that, getter));
                p->setCondition(std::shared_ptr<BasicCondition>(new ConditionMethod<Condition>(condition, method)));
                return publish(address, p);
            }
            
            template <typename T, typename C, typename Condition>
            Identifier publishIf(Condition &condition, bool (Condition::*method)(), const std::string &address, C &that, T (C::*getter)()) {
                ParameterRef p = ParameterRef(new GetterParameter<T, C, true>(that, getter));
                p->setCondition(std::shared_ptr<BasicCondition>(new ConditionMethod<Condition>(condition, method)));
                return publish(address, p);
            }
            
            template <typename T, typename C, typename Condition>
            Identifier publishIf(Condition &condition, bool (Condition::*method)(), const std::string &address, const C * const that, T (C::*getter)() const) {
                ParameterRef p = ParameterRef(new ConstGetterParameter<T, C, true>(*that, getter));
                p->setCondition(std::shared_ptr<BasicCondition>(new ConditionMethod<Condition>(condition, method)));
                return publish(address, p);
            }
            
            template <typename T, typename C, typename Condition>
            Identifier publishIf(Condition &condition, bool (Condition::*method)(), const std::string &address, const C &that, T (C::*getter)() const) {
                ParameterRef p = ParameterRef(new ConstGetterParameter<T, C, true>(that, getter));
                p->setCondition(std::shared_ptr<BasicCondition>(new ConditionMethod<Condition>(condition, method)));
                return publish(address, p);
            }
            
#pragma mark condition is const method
            
            template <typename T, typename Condition>
            Identifier publishIf(const Condition &condition, bool (Condition::*method)() const, const std::string &address, T &value) {
                ParameterRef p = ParameterRef(new Parameter<T, true>(value));
                p->setCondition(std::shared_ptr<BasicCondition>(new ConstConditionMethod<Condition>(condition, method)));
                return publish(address, p);
            }
            
            template <typename T, typename Condition>
            Identifier publishIf(const Condition &condition, bool (Condition::*method)() const, const std::string &address, T (*getter)()) {
                ParameterRef p = ParameterRef(new GetterFunctionParameter<T, true>(getter));
                p->setCondition(std::shared_ptr<BasicCondition>(new ConstConditionMethod<Condition>(condition, method)));
                return publish(address, p);
            }
            
            template <typename T, typename C, typename Condition>
            Identifier publishIf(const Condition &condition, bool (Condition::*method)() const, const std::string &address, C *that, T (C::*getter)()) {
                ParameterRef p = ParameterRef(new GetterParameter<T, C, true>(*that, getter));
                p->setCondition(std::shared_ptr<BasicCondition>(new ConstConditionMethod<Condition>(condition, method)));
                return publish(address, p);
            }
            
            template <typename T, typename C, typename Condition>
            Identifier publishIf(const Condition &condition, bool (Condition::*method)() const, const std::string &address, C &that, T (C::*getter)()) {
                ParameterRef p = ParameterRef(new GetterParameter<T, C, true>(that, getter));
                p->setCondition(std::shared_ptr<BasicCondition>(new ConstConditionMethod<Condition>(condition, method)));
                return publish(address, p);
            }
            
            template <typename T, typename C, typename Condition>
            Identifier publishIf(const Condition &condition, bool (Condition::*method)() const, const std::string &address, const C * const that, T (C::*getter)() const) {
                ParameterRef p = ParameterRef(new ConstGetterParameter<T, C, true>(*that, getter));
                p->setCondition(std::shared_ptr<BasicCondition>(new ConstConditionMethod<Condition>(condition, method)));
                return publish(address, p);
            }
            
            template <typename T, typename C, typename Condition>
            Identifier publishIf(const Condition &condition, bool (Condition::*method)() const, const std::string &address, const C &that, T (C::*getter)() const) {
                ParameterRef p = ParameterRef(new ConstGetterParameter<T, C, true>(that, getter));
                p->setCondition(std::shared_ptr<BasicCondition>(new ConstConditionMethod<Condition>(condition, method)));
                return publish(address, p);
            }
            
#pragma mark unpublish
            
            void unpublish(const std::string &address) {
                if(targets.find(address) == targets.end()) targets.erase(address);
            }
            
            void unpublish(Identifier &identifier) {
                if(!identifier.isValid()) return;
                Targets::const_iterator it{findPublished(identifier)};
                if(it != targets.end()) {
                    targets.erase(it);
                }
                identifier.invalidate();
            }
            
            void unpublish() {
                targets.clear();
            }

#pragma mark stop publish temporary
            
            void stopPublishTemporary(const std::string &address) {
                if(isPublished(address)) {
                    Targets::iterator it = targets.find(address);
                    for(std::size_t i = 0, size = targets.count(address); i < size; i++, ++it) {
                        it->second->setEnablePublish(false);
                    }
                }
            }
            
            void stopPublishTemporary(const Identifier &identifier) {
                if(!identifier.isValid()) return;
                Targets::const_iterator it{findPublished(identifier)};
                if(it != targets.end()) {
                    it->second->setEnablePublish(false);
                }
            }
            
            void resumePublish(const std::string &address) {
                if(isPublished(address)) {
                    Targets::iterator it = targets.find(address);
                    for(std::size_t i = 0, size = targets.count(address); i < size; i++, ++it) {
                        it->second->setEnablePublish(true);
                    }
                }
            }
            
            void resumePublishTemporary(const Identifier &identifier) {
                if(!identifier.isValid()) return;
                Targets::const_iterator it{findPublished(identifier)};
                if(it != targets.end()) {
                    it->second->setEnablePublish(true);
                }
            }
            
#pragma mark doRegister
            
            inline Identifier doRegister(const std::string &address, ParameterRef ref) {
                registeredTargets.insert(std::make_pair(address, ref));
                return {address, ref, destination};
            }
            
            template <typename T>
            Identifier doRegister(const std::string &address, T &value) {
                return doRegister(address, ParameterRef(new Parameter<T, false>(value)));
            }
            
            template <typename T>
            Identifier doRegister(const std::string &address, T (*getter)()) {
                return doRegister(address, ParameterRef(new GetterFunctionParameter<T, false>(getter)));
            }
            
            template <typename T, typename C>
            Identifier doRegister(const std::string &address, C *that, T (C::*getter)()) {
                return doRegister(address, ParameterRef(new GetterParameter<T, C, false>(*that, getter)));
            }
            
            template <typename T, typename C>
            Identifier doRegister(const std::string &address, C &that, T (C::*getter)()) {
                return doRegister(address, ParameterRef(new GetterParameter<T, C, false>(that, getter)));
            }
            
            template <typename T, typename C>
            Identifier doRegister(const std::string &address, const C * const that, T (C::*getter)() const) {
                return doRegister(address, ParameterRef(new ConstGetterParameter<T, C, false>(*that, getter)));
            }
            
            template <typename T, typename C>
            Identifier doRegister(const std::string &address, const C &that, T (C::*getter)() const) {
                return doRegister(address, ParameterRef(new ConstGetterParameter<T, C, false>(that, getter)));
            }
            
#pragma mark publishRegistered
            
            inline void publishRegistered(const std::string &address) {
                Targets::iterator it = registeredTargets.find(address);
                if(it == registeredTargets.end()) {
                    ofLogWarning("ofxPubSubOsc") << address << " is not registered.";
                }
                ofxOscMessage m;
                for(std::size_t i = 0, size = registeredTargets.count(address); i < size; i++, ++it) {
                    if(it->second->setMessage(m, it->first)) sender.sendMessage(m);
                    m.clear();
                }
            }
            
            inline void publishRegistered(const Identifier &identifier) {
                if(!identifier.isValid()) return;
                Targets::const_iterator it{findRegistered(identifier)};
                if(it != registeredTargets.end()) {
                    ofxOscMessage m;
                    if(it->second->setMessage(m, it->first)) sender.sendMessage(m);
                    m.clear();
                }
            }
            
#pragma mark unregister
            
            inline void unregister(const std::string &address) {
                if(registeredTargets.find(address) == registeredTargets.end()) registeredTargets.erase(address);
            }
            
            inline void unregister(Identifier &identifier) {
                if(!identifier.isValid()) return;
                Targets::const_iterator it{findRegistered(identifier)};
                if(it != registeredTargets.end()) {
                    registeredTargets.erase(it);
                }
                identifier.invalidate();
            }
            
            inline void unregister() {
                registeredTargets.clear();
            }
            
#pragma mark status
            
            inline bool isPublished() const {
                return !targets.empty();
            }
            
            inline bool isPublished(const Identifier &identifier) const {
                if(!identifier.isValid()) false;
                return isPublished() && (findPublished(identifier) != targets.end());
            }
            
            inline bool isPublished(const std::string &address) const {
                return isPublished() && (targets.find(address) != targets.end());
            }
            
            inline bool isEnabled(const std::string &address) const {
                // TODO: fix
                return isPublished(address) && targets.find(address)->second->isPublishNow();
            }
            
            inline bool isEnabled(const Identifier &identifier) const {
                if(!identifier.isValid()) return false;
                Targets::const_iterator it{findPublished(identifier)};
                return (it != targets.end()) && it->second->isPublishNow();
            }
            
            inline bool isRegistered() const {
                return !registeredTargets.empty();
            }
            
            inline bool isRegistered(const std::string &address) const {
                return isRegistered() && (registeredTargets.find(address) != registeredTargets.end());
            }
            
            using Ref = std::shared_ptr<OscPublisher>;
            
            static void setUseBundle(bool b) {
                bUseBundle() = b;
            }
            
            static bool isUseBundle() {
                return bUseBundle();
            }
            
        private:
            OscPublisher(const Destination &destination)
            : destination(destination) {
                sender.setup(destination.ip, destination.port);
            }
            
            void update() {
                ofxOscMessage m;
                if(isUseBundle()) {
                    ofxOscBundle bundle;
                    for(Targets::iterator it = targets.begin(); it != targets.end(); ++it) {
                        if(it->second->setMessage(m, it->first)) bundle.addMessage(m);
                        m.clear();
                    }
                    if(bundle.getMessageCount()) sender.sendBundle(bundle);
                    bundle.clear();
                    return;
                }
                for(Targets::iterator it = targets.begin(); it != targets.end(); ++it) {
                    if(it->second->setMessage(m, it->first)) sender.sendMessage(m);
                    m.clear();
                }
                
            }
            
            Destination destination;
            ofxOscSender sender;
            Targets targets;
            Targets registeredTargets;
            static bool &bUseBundle() {
                static bool _;
                return _;
            };
            friend class OscPublisherManager;
        };
        
        using OscPublishers = std::map<Destination, OscPublisher::Ref>;
        
        static OscPublisherManager &getSharedInstance() {
            static OscPublisherManager *sharedInstance = new OscPublisherManager;
            return *sharedInstance;
        }
        
        static OscPublisher &getOscPublisher(const std::string &ip, int port) {
            OscPublishers &publishers = getSharedInstance().publishers;
            Destination destination(ip, port);
            if(publishers.find(destination) == publishers.end()) {
                publishers.insert(std::make_pair(destination, OscPublisher::Ref(new OscPublisher(destination))));
            }
            return *(publishers[destination].get());
        }
        
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
        
#pragma mark iterator
    public:
        using iterator = OscPublishers::iterator;
        using const_iterator = OscPublishers::const_iterator;
        using reverse_iterator = OscPublishers::reverse_iterator;
        using const_reverse_iterator = OscPublishers::const_reverse_iterator;
        
        iterator begin() { return publishers.begin(); }
        iterator end() { return publishers.end(); }
        
        const_iterator begin() const { return publishers.cbegin(); }
        const_iterator end() const { return publishers.cend(); }
        const_iterator cbegin() const { return publishers.cbegin(); }
        const_iterator cend() const { return publishers.cend(); }
        
        reverse_iterator rbegin() { return publishers.rbegin(); }
        reverse_iterator rend() { return publishers.rend(); }
        
        const_reverse_iterator rbegin() const { return publishers.crbegin(); }
        const_reverse_iterator rend() const { return publishers.crend(); }
        const_reverse_iterator crbegin() const { return publishers.crbegin(); }
        const_reverse_iterator crend() const { return publishers.crend(); }
    };
};
#undef type_ref

#pragma mark - syntax sugars

#pragma mark getter

using ofxOscPublisherManager = ofx::OscPublisherManager;
using ofxOscPublisher = ofxOscPublisherManager::OscPublisher;
using ofxOscPublisherIdentifier = ofxOscPublisherManager::Identifier;

/// \brief get a OscPublisherManager.
/// \returns ofxOscPublisherManager

inline ofxOscPublisherManager &ofxGetOscPublisherManager() {
    return ofxOscPublisherManager::getSharedInstance();
}

/// \brief get a OscPublisher.
/// \param ip target ip is typed const std::string &
/// \param port target port is typed int
/// \returns ofxOscPublisher binded to ip & port

inline ofxOscPublisher &ofxGetOscPublisher(const std::string &ip, int port) {
    return ofxOscPublisherManager::getOscPublisher(ip, port);
}

#pragma mark publish

/// \name ofxPublishOsc
/// \{

/// \brief publish value as an OSC message with an address pattern address to ip:port every time the value has changed.
/// If whenValueIsChanged is set to false, then the binded value is sent every frame after App::update.
/// template parameter T is suggested by value
/// \param ip target ip is typed const std::string &
/// \param port target port is typed int
/// \param address osc address is typed const std::string &
/// \param value reference of value is typed T &
/// \param whenValueIsChanged if this value to false, then we send value every update
/// \returns ofxOscPublisherIdentifier

template <typename ValueRefOrGetterFunction>
inline ofxOscPublisherIdentifier ofxPublishOsc(const std::string &ip, int port, const std::string &address, ValueRefOrGetterFunction &&valueRefOrGetterFunction, bool whenValueIsChanged = true) {
    return ofxGetOscPublisher(ip, port).publish(address, valueRefOrGetterFunction, whenValueIsChanged);
}

/// \brief publish the value will be gave by function as an OSC message with an address pattern address to ip:port every time the value has changed.
/// If whenValueIsChanged is set to false, then the binded value is sent every frame after App::update.
/// template parameter ObjectPtrOrRef is suggested by that, and Method is suggested by getter.
/// \param ip target ip is typed const std::string &
/// \param port target port is typed int
/// \param address osc address is typed const std::string &
/// \param that this object is typed ObjectPtrOrRef, will bind with next parameter method. is called as (that->*getter)() or (that.*getter)().
/// \param getter this method gives value, is typed T(C::*)()
/// \param whenValueIsChanged if this value to false, then we send value every update
/// \returns ofxOscPublisherIdentifier

template <typename ObjectPtrOrRef, typename Method>
inline ofxOscPublisherIdentifier ofxPublishOsc(const std::string &ip, int port, const std::string &address, ObjectPtrOrRef &&that, Method getter, bool whenValueIsChanged = true) {
    return ofxGetOscPublisher(ip, port).publish(address, that, getter, whenValueIsChanged);
}

template <typename ... Args>
inline void ofxPublishOsc(const std::string &ip, const std::initializer_list<int> ports, Args & ... args) {
    for(auto port : ports) {
        ofxGetOscPublisher(ip, port).publish(args ...);
    }
}

template <typename ... Args>
inline void ofxPublishOsc(const std::initializer_list<ofxOscPublisherManager::IP> &ips, Args & ... args) {
    for(auto &ip : ips) {
        ofxPublishOsc(ip, args ...);
    }
}

template <typename ... Args>
inline void ofxPublishOsc(const std::initializer_list<ofxOscPublisherManager::Destination> &targets, Args & ... args) {
    for(auto &target : targets) {
        ofxPublishOsc(target.ip, target.port, args ...);
    }
}

template <typename ... Args>
inline void ofxPublishOsc(const std::initializer_list<ofxOscPublisherManager::DestinationWithAddress> &targets, Args & ... args) {
    for(auto &target : targets) {
        ofxPublishOsc(target.destination.ip, target.destination.port, target.address, args ...);
    }
}

/// \}

#pragma mark publish if condition

/// \name ofxPublishOscif
/// \{

/// \brief publish value as an OSC message with an address pattern address to ip:port when condition is true.
/// template parameter T is suggested by value
/// \param condition condition of publish typed bool &
/// \param ip target ip is typed const std::string &
/// \param port target port is typed int
/// \param address osc address is typed const std::string &
/// \param value reference of value is typed T &
/// \param whenValueIsChanged if this value to false, then we send value every update
/// \returns ofxOscPublisherIdentifier

template <typename ConditionValueRef, typename ValueRefOrGetterFunction>
inline ofxOscPublisherIdentifier ofxPublishOscIf(ConditionValueRef &&condition, const std::string &ip, int port, const std::string &address, ValueRefOrGetterFunction &&valueRefOrGetterFunction) {
    return ofxGetOscPublisher(ip, port).publishIf(condition, address, valueRefOrGetterFunction);
}

template <typename ConditionValueRef, typename ObjectPtrOrRef, typename GetterMethod>
inline ofxOscPublisherIdentifier ofxPublishOscIf(ConditionValueRef &&condition, const std::string &ip, int port, const std::string &address, ObjectPtrOrRef &&that, GetterMethod getter) {
    return ofxGetOscPublisher(ip, port).publishIf(condition, address, that, getter);
}

#pragma mark condition is method

template <typename ConditionObjectPtrOrRef, typename ConditionMethodReturnsBool, typename ValueRefOrFunction>
inline ofxOscPublisherIdentifier ofxPublishOscIf(ConditionObjectPtrOrRef &&condition, ConditionMethodReturnsBool method, const std::string &ip, int port, const std::string &address, ValueRefOrFunction &&valueRefOrGetterFunction) {
    return ofxGetOscPublisher(ip, port).publishIf(condition, method, address, valueRefOrGetterFunction);
}

template <typename ConditionObjectPtrOrRef, typename ConditionMethodReturnsBool, typename ObjectPtrOrRef, typename GetterMethod>
inline ofxOscPublisherIdentifier ofxPublishOscIf(ConditionObjectPtrOrRef &&condition, ConditionMethodReturnsBool method, const std::string &ip, int port, const std::string &address, ObjectPtrOrRef &&that, GetterMethod getter) {
    return ofxGetOscPublisher(ip, port).publishIf(condition, method, address, *that, getter);
}

/// \}

#pragma mark unpublish

/// \name ofxUnpublishOsc
/// \{

inline void ofxUnpublishOsc(ofxOscPublisherIdentifier &identifier) {
    if(!identifier.isValid()) return;
    ofxGetOscPublisher(identifier.getKey().ip, identifier.getKey().port).unpublish(identifier);
}

inline void ofxUnpublishOsc(const std::string &ip, int port, const std::string &address) {
    ofxGetOscPublisher(ip, port).unpublish(address);
}

inline void ofxUnpublishOsc(const std::string &ip, int port) {
    ofxGetOscPublisher(ip, port).unpublish();
}

inline void ofxUnpublishOsc() {
    ofxOscPublisherManager &manager = ofxGetOscPublisherManager();
    ofxOscPublisherManager::iterator it  = manager.begin(),
                                     end = manager.end();
    for(; it != end; it++) {
        it->second->unpublish();
    }
}

/// \}

#pragma mark register

/// \name ofxRegisterPublishingOsc
/// \{

template <typename ValueOrGetterFunctionType>
inline ofxOscPublisherIdentifier ofxRegisterPublishingOsc(const std::string &ip, int port, const std::string &address, ValueOrGetterFunctionType &&valueOrGetterFunction) {
    return ofxGetOscPublisher(ip, port).doRegister(address, valueOrGetterFunction);
}

template <typename ObjectPtrOrRef, typename GetterMethod>
inline ofxOscPublisherIdentifier ofxRegisterPublishingOsc(const std::string &ip, int port, const std::string &address, ObjectPtrOrRef &&that, GetterMethod &&getter) {
    return ofxGetOscPublisher(ip, port).doRegister(address, that, getter);
}

/// \}

#pragma mark publish registered

/// \name ofxPublishRegisteredOsc
/// \{

inline void ofxPublishRegisteredOsc(const std::string &ip, int port, const std::string &address) {
    ofxGetOscPublisher(ip, port).publishRegistered(address);
}

inline void ofxPublishRegisteredOsc(const ofxOscPublisherIdentifier &identifier) {
    if(!identifier.isValid()) return;
    ofxGetOscPublisher(identifier.getKey().ip, identifier.getKey().port).publishRegistered(identifier);
}

/// \}

#pragma mark unregister

/// \name ofxUnregisterPublishingOsc
/// \{

inline void ofxUnregisterPublishingOsc(ofxOscPublisherIdentifier &identifier) {
    if(!identifier.isValid()) return;
    ofxGetOscPublisher(identifier.getKey().ip, identifier.getKey().port).unregister(identifier);
}

inline void ofxUnregisterPublishingOsc(const std::string &ip, int port, const std::string &address) {
    ofxGetOscPublisher(ip, port).unregister(address);
}

inline void ofxUnregisterPublishingOsc(const std::string &ip, int port) {
    ofxGetOscPublisher(ip, port).unregister();
}

inline void ofxUnregisterPublishingOsc() {
    ofxOscPublisherManager &manager = ofxGetOscPublisherManager();
    ofxOscPublisherManager::iterator it  = manager.begin(),
                                     end = manager.end();
    for(; it != end; it++) {
        it->second->unregister();
    }
}

/// \}

#pragma mark using bundle option

inline void ofxSetPublisherUsingBundle(bool bUseBundle) {
    ofxOscPublisher::setUseBundle(bUseBundle);
}

#pragma mark helper for publish array

/// \name helper for publish array
/// \{

template <typename T, size_t size>
struct array_type {
    using type = T (&)[size];
    using fun = type (*)();
    template <typename U>
    struct meth {
        using method = type (U::*)();
        using const_method = type (U::*)() const;
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
