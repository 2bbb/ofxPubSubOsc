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

#include <initializer_list>

#if OFX_PUBSUBOSC_DEBUG
#   define TYPE_DEBUG(type) ofLogNotice() << __func__ << ":" << __LINE__ << "[" << address << "], " << typeid(type).name();
#else
#   define TYPE_DEBUG(type) ;
#endif

namespace ofx {
    using namespace ofxpubsubosc;
    
    namespace {
        template <typename T>
        using get_type = typename T::type;
        
        template <typename T>
        struct is_callable {
            template <typename U, decltype(&U::operator()) = &U::operator()>
            struct checker {};
            template <typename U> static std::true_type  test(checker<U> *);
            template <typename>   static std::false_type test(...);
            static constexpr bool value = decltype(test<T>(nullptr))::value;
        };
        
        template <typename T>
        struct function_info;
        
        template <typename T, bool b>
        struct function_info_impl {
            static constexpr bool is_function = false;
        };
        
        template <typename T>
        struct function_info_impl<T, true> : public function_info<decltype(&T::operator())> {};
        
        template <typename T>
        struct function_info : public function_info_impl<T, is_callable<T>::value> {};
        
        template <typename U, typename ret, typename ... arguments>
        struct function_info<ret(U::*)(arguments ...) const> {
            static constexpr bool is_function = true;
            static constexpr std::size_t arity = sizeof...(arguments);
            using result_type = ret;
            using arguments_types_tuple = std::tuple<arguments ...>;
            template <std::size_t index>
            using argument_type = get_type<std::tuple_element<index, arguments_types_tuple>>;
            using function_type = std::function<result_type(arguments ...)>;
        };
        
        template <typename U, typename ret, typename ... arguments>
        struct function_info<ret(U::*)(arguments ...)> {
            static constexpr bool is_function = true;
            static constexpr std::size_t arity = sizeof...(arguments);
            using result_type = ret;
            using arguments_types_tuple = std::tuple<arguments ...>;
            template <std::size_t index>
            using argument_type = get_type<std::tuple_element<index, arguments_types_tuple>>;
            using function_type = std::function<result_type(arguments ...)>;
        };
        
        template <typename ret, typename ... arguments>
        struct function_info<ret(*)(arguments ...)> {
            static constexpr bool is_function = true;
            static constexpr std::size_t arity = sizeof...(arguments);
            using result_type = ret;
            using arguments_types_tuple = std::tuple<arguments ...>;
            template <std::size_t index>
            using argument_type = get_type<std::tuple_element<index, arguments_types_tuple>>;
            using function_type = std::function<result_type(arguments ...)>;
        };
        
        template <typename T>
        function_info<T> get_function_info(T &t); // for decltype
    };
    
    class OscSubscriberManager {
    private:
        class OscSubscriber;
        
        struct SetImplementation {
        protected:
#define define_set_arithmetic(type) \
            inline void set(ofxOscMessage &m, type &v, std::size_t offset = 0) { \
                if(m.getArgType(offset) == OFXOSC_TYPE_INT32) v = m.getArgAsInt32(offset); \
                else if(m.getArgType(offset) == OFXOSC_TYPE_INT64) v = m.getArgAsInt64(offset); \
                else if(m.getArgType(offset) == OFXOSC_TYPE_FLOAT) v = m.getArgAsFloat(offset); \
                else if(m.getArgType(offset) == OFXOSC_TYPE_DOUBLE) v = m.getArgAsDouble(offset); \
                else if(m.getArgType(offset) == OFXOSC_TYPE_STRING) v = ofToDouble(m.getArgAsString(offset)); \
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
                if(m.getArgType(offset) == OFXOSC_TYPE_STRING) v = m.getArgAsString(offset);
                else if(m.getArgType(offset) == OFXOSC_TYPE_FLOAT) v = ofToString(m.getArgAsFloat(offset));
                else if(m.getArgType(offset) == OFXOSC_TYPE_DOUBLE) v = ofToString(m.getArgAsDouble(offset));
                else if(m.getArgType(offset) == OFXOSC_TYPE_INT32) v = ofToString(m.getArgAsInt32(offset));
                else if(m.getArgType(offset) == OFXOSC_TYPE_INT64) v = ofToString(m.getArgAsInt64(offset));
                else v = m.getArgAsString(offset);
            }
            
            inline void set(ofxOscMessage &m, ofBuffer &v, std::size_t offset = 0) {
                v = m.getArgAsBlob(offset);
            }
            
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
                type_convert(ofBuffer);
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
            SetterFunctionParameter(std::function<R(T)> setter) : setter(setter) {};
            virtual void read(ofxOscMessage &message) {
                typename remove_const_reference<T>::type t;
                set(message, t);
                setter(t);
            }
            
        private:
            std::function<R(T)> setter;
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

        template <typename R>
        struct CallbackParameter : AbstractParameter, SetImplementation {
        public:
            using Callback = std::function<R(ofxOscMessage &)>;
            CallbackParameter(const Callback &callback)
            : callback(callback) {}
            
            virtual void read(ofxOscMessage &message) { callback(message); }
            
        private:
            Callback callback;
        };
        
        template <typename C, typename R>
        struct MethodCallbackParameter : AbstractParameter, SetImplementation {
            using Callback = R (C::*)(ofxOscMessage &);
            MethodCallbackParameter(C &that, Callback callback)
            : that(that), callback(callback) {}
            
            virtual void read(ofxOscMessage &message) { (that.*callback)(message); }
            
        private:
            Callback callback;
            C &that;
        };

        template <typename C, typename R>
        struct ConstMethodCallbackParameter : AbstractParameter, SetImplementation {
            using Callback = R (C::*)(ofxOscMessage &) const;
            ConstMethodCallbackParameter(const C &that, Callback callback)
            : that(that), callback(callback) {}
            
            virtual void read(ofxOscMessage &message) { (that.*callback)(message); }
            
        private:
            Callback callback;
            const C &that;
        };
        
        using ParameterRef = std::shared_ptr<AbstractParameter>;
        using OscReceiverRef = std::shared_ptr<ofxOscReceiver>;
        using Targets = std::multimap<std::string, ParameterRef>;
        
        class Identifier {
            std::string address;
            ParameterRef ref;
            int key;
            
            void invalidate() {
                address = "";
                ref = nullptr;
                key = 0;
            }
        public:
            Identifier() : address(""), ref(nullptr) {}
            Identifier(const std::string &address, const ParameterRef &ref, int key)
            : address(address)
            , ref(ref)
            , key(key) {}
            
            const int getKey() const { return key; };
            bool isValid() const { return static_cast<bool>(ref); }
            
            friend class OscSubscriber;
        };

    public:
        class OscSubscriber {
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
            
            inline Targets::const_iterator findSubscribed(const Identifier &identifier) const {
                return findFromTargets(identifier, targets);
            }

            inline Identifier subscribe(const std::string &address, ParameterRef ref) {
                Targets::iterator it = targets.insert(std::make_pair(address, ref));
                return {address, ref, port};
            }
            
        public:
            
            template <typename T>
            inline typename std::enable_if<!function_info<T>::is_function, Identifier>::type subscribe(const std::string &address, T &value) {
                return subscribe(address, ParameterRef(new Parameter<T>(value)));
            }
            
            template <typename T>
            inline typename std::enable_if<function_info<T>::is_function, Identifier>::type subscribe(const std::string &address, T &value) {
                TYPE_DEBUG(T);
                return subscribe(address, static_cast<typename function_info<T>::function_type>(value));
            }
            
            template <typename T, typename R>
            inline typename std::enable_if<!std::is_same<typename remove_const_reference<T>::type, ofxOscMessage>::value, Identifier>::type subscribe(const std::string &address, std::function<R(T)> setter) {
                TYPE_DEBUG(T);
                return subscribe(address, ParameterRef(new SetterFunctionParameter<T, R>(setter)));
            }
            
            template <typename T, typename C, typename R>
            inline typename is_not_ofxoscmessage<T, Identifier>::type subscribe(const std::string &address, C &that, R (C::*setter)(T)) {
                return subscribe(address, ParameterRef(new SetterMethodParameter<T, C, R>(that, setter)));
            }
            
            template <typename T, typename C, typename R>
            inline typename is_not_ofxoscmessage<T, Identifier>::type subscribe(const std::string &address, C *that, R (C::*setter)(T)) {
                return subscribe(address, ParameterRef(new SetterMethodParameter<T, C, R>(*that, setter)));
            }
            
            template <typename T, typename C, typename R>
            inline typename is_not_ofxoscmessage<T, Identifier>::type subscribe(const std::string &address, const C &that, R (C::*setter)(T) const) {
                return subscribe(address, ParameterRef(new ConstSetterMethodParameter<T, C, R>(that, setter)));
            }

            template <typename T, typename C, typename R>
            inline typename is_not_ofxoscmessage<T, Identifier>::type subscribe(const std::string &address, const C * const that, R (C::*setter)(T) const) {
                return subscribe(address, ParameterRef(new ConstSetterMethodParameter<T, C, R>(*that, setter)));
            }

            template <typename R>
            inline Identifier subscribe(const std::string &address, const std::function<R(ofxOscMessage &)> callback) {
                TYPE_DEBUG(R);
                return subscribe(address, ParameterRef(new CallbackParameter<R>(callback)));
            }
            
            inline Identifier subscribe(const std::string &address, const std::function<void(ofxOscMessage &)> callback) {
                TYPE_DEBUG(void);
                return subscribe(address, ParameterRef(new CallbackParameter<void>(callback)));
            }
            
            template <typename C, typename R>
            inline Identifier subscribe(const std::string &address, C &that, R (C::*callback)(ofxOscMessage &)) {
                return subscribe(address, ParameterRef(new MethodCallbackParameter<C, R>(that, callback)));
            }
            
            inline void unsubscribe(const std::string &address) {
                targets.erase(address);
            }
            
            inline void unsubscribe(Identifier &identifier) {
                if(!identifier.isValid()) return;
                Targets::const_iterator it{findSubscribed(identifier)};
                if(it != targets.end()) {
                    targets.erase(it);
                }
                identifier.invalidate();
            }
            
            inline void unsubscribe() {
                targets.clear();
            }
            
            inline void setLeakPicker(ParameterRef ref) {
                leakPicker = ref;
            }
            
            inline void setLeakPicker(const std::function<void(ofxOscMessage &)> &callback) {
                setLeakPicker(ParameterRef(new CallbackParameter<void>(callback)));
            }
       
            template <typename C, typename R>
            inline void setLeakPicker(C &that, R (C::*callback)(ofxOscMessage &)) {
                setLeakPicker(ParameterRef(new MethodCallbackParameter<C, R>(that, callback)));
            }
            
            template <typename C, typename R>
            inline void setLeakPicker(const C &that, R (C::*callback)(ofxOscMessage &) const) {
                setLeakPicker(ParameterRef(new ConstMethodCallbackParameter<C, R>(that, callback)));
            }
            
            
            inline void removeLeakPicker() {
                leakPicker.reset();
            }
            
            inline bool isSubscribed() const {
                return !targets.empty();
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
                Targets::iterator it = targets.find(address);
                if(it != targets.end()) {
                    for(std::size_t i = 0; i < targets.count(address); i++, ++it) {
                        it->second->read(m);
                    }
                }
            }
            
            void notify(const Identifier &identifier, ofxOscMessage &m) {
                if(!identifier.isValid()) return;
                Targets::const_iterator it{findSubscribed(identifier)};
                if(it != targets.end() && it->first == m.getAddress()) {
                    it->second->read(m);
                }
            }
            
            using Ref = std::shared_ptr<OscSubscriber>;
        private:
            OscSubscriber(int port)
            : port(port) {
                receiver.setup(port);
            }
            
            void update() {
                clearLeakedOscMessages();
                ofxOscMessage m;
                while(receiver.hasWaitingMessages()) {
                    receiver.getNextMessage(m);
                    const std::string &address = m.getAddress();
                    Targets::iterator it = targets.find(address);
                    if(it != targets.end()) {
                        for(std::size_t i = 0; i < targets.count(address); i++, ++it) {
                            it->second->read(m);
                        }
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
        using OscSubscribers = std::map<int, OscSubscriber::Ref>;
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
        using iterator = OscSubscribers::iterator;
        using const_iterator = OscSubscribers::const_iterator;
        using reverse_iterator = OscSubscribers::reverse_iterator;
        using const_reverse_iterator = OscSubscribers::const_reverse_iterator;
        
        iterator begin() { return managers.begin(); }
        iterator end() { return managers.end(); }
        
        const_iterator begin() const { return managers.cbegin(); }
        const_iterator end() const { return managers.cend(); }
        const_iterator cbegin() const { return managers.cbegin(); }
        const_iterator cend() const { return managers.cend(); }
        
        reverse_iterator rbegin() { return managers.rbegin(); }
        reverse_iterator rend() { return managers.rend(); }
        
        const_reverse_iterator rbegin() const { return managers.crbegin(); }
        const_reverse_iterator rend() const { return managers.crend(); }
        const_reverse_iterator crbegin() const { return managers.crbegin(); }
        const_reverse_iterator crend() const { return managers.crend(); }
    };
};

using ofxOscSubscriberManager = ofx::OscSubscriberManager;
using ofxOscSubscriber = ofxOscSubscriberManager::OscSubscriber;
using ofxOscSubscriberIdentifier = ofxOscSubscriberManager::Identifier;

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
inline ofxOscSubscriberIdentifier ofxSubscribeOsc(int port, const std::string &address, T &value) {
    return ofxGetOscSubscriber(port).subscribe(address, value);
}

#pragma mark setter function/method

template <typename T, typename R>
inline typename ofxpubsubosc::is_not_ofxoscmessage<T, ofxOscSubscriberIdentifier>::type ofxSubscribeOsc(int port, const std::string &address, R (*callback)(T)) {
    return ofxGetOscSubscriber(port).subscribe(address, callback);
}

template <typename T, typename C, typename R>
inline typename ofxpubsubosc::is_not_ofxoscmessage<T, ofxOscSubscriberIdentifier>::type ofxSubscribeOsc(int port, const std::string &address, C &that, R (C::*callback)(T)) {
    return ofxGetOscSubscriber(port).subscribe(address, that, callback);
}

template <typename T, typename C, typename R>
inline typename ofxpubsubosc::is_not_ofxoscmessage<T, ofxOscSubscriberIdentifier>::type ofxSubscribeOsc(int port, const std::string &address, C *that, R (C::*callback)(T)) {
    return ofxGetOscSubscriber(port).subscribe(address, *that, callback);
}

template <typename T, typename C, typename R>
inline typename ofxpubsubosc::is_not_ofxoscmessage<T, ofxOscSubscriberIdentifier>::type ofxSubscribeOsc(int port, const std::string &address, const C &that, R (C::*callback)(T) const) {
    return ofxGetOscSubscriber(port).subscribe(address, that, callback);
}

template <typename T, typename C, typename R>
inline typename ofxpubsubosc::is_not_ofxoscmessage<T, ofxOscSubscriberIdentifier>::type ofxSubscribeOsc(int port, const std::string &address, const C * const that, R (C::*callback)(T) const) {
    return ofxGetOscSubscriber(port).subscribe(address, *that, callback);
}

#pragma mark callback function/method

/// \brief bind a callback to the OSC messages with an address pattern _address_ incoming to _port_.
/// \param port binded port is typed int
/// \param address osc address is typed const std::string &
/// \param callback is kicked when receive a message to address
/// \returns void

inline void ofxSubscribeOsc(int port, const std::string &address, void (*callback)(ofxOscMessage &)) {
    return ofxGetOscSubscriber(port).subscribe(address, callback);
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
    return ofxGetOscSubscriber(port).subscribe(address, that, callback);
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
    return ofxGetOscSubscriber(port).subscribe(address, *that, callback);
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
    return ofxGetOscSubscriber(port).subscribe(address, that, callback);
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
    return ofxGetOscSubscriber(port).subscribe(address, *that, callback);
}

template <typename C, typename R>
inline void ofxSubscribeOsc(int port, const std::string &address, const std::function<void(ofxOscMessage &)> &callback) {
    return ofxGetOscSubscriber(port).subscribe(address, callback);
}

template <typename ... Args>
inline void ofxSubscribeOsc(const std::initializer_list<int> &ports, const std::string &address, Args & ... args) {
    for(auto &port : ports) {
        ofxSubscribeOsc(port, address, args ...);
    }
}

template <typename ... Args>
inline void ofxSubscribeOsc(int port, const std::initializer_list<const std::string> &addresses, Args & ... args) {
    auto &subscriber = ofxGetOscSubscriber(port);
    for(auto &address : addresses) {
        subscriber.subscribe(address, args ...);
    }
}

template <typename ... Args>
inline void ofxSubscribeOsc(const std::initializer_list<int> &ports, const std::initializer_list<const std::string> &addresses, Args & ... args) {
    for(auto &port : ports) {
        ofxSubscribeOsc(port, addresses, args ...);
    }
}

/// \}

#pragma mark unsubscribe

/// \name ofxUnsubscribeOsc
/// \{

// TODO: add document

inline void ofxUnsubscribeOsc(ofxOscSubscriberIdentifier &identifier) {
    if(!identifier.isValid()) return;
    ofxGetOscSubscriber(identifier.getKey()).unsubscribe(identifier);
}


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
    for(; it != end; ++it) {
        it->second->unsubscribe();
    }
}

/// \}

#pragma mark notify messages manually

inline void ofxNotifyToSubscribedOsc(ofxOscSubscriberIdentifier &identifier, ofxOscMessage &m) {
    ofxGetOscSubscriber(identifier.getKey()).notify(m);
}

inline void ofxNotifyToSubscribedOsc(int port, ofxOscMessage &m) {
    ofxGetOscSubscriber(port).notify(m);
}

inline void ofxNotifyToSubscribedOsc(ofxOscMessage &m) {
    ofxOscSubscriberManager &manager = ofxGetOscSubscriberManager();
    ofxOscSubscriberManager::iterator it  = manager.begin(),
                                      end = manager.end();
    for(; it != end; ++it) {
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

inline void ofxSetLeakedOscPicker(int port, const std::function<void(ofxOscMessage &)> &callback) {
    ofxGetOscSubscriber(port).setLeakPicker(callback);
}
#pragma mark leak picking all port

template <typename T, typename ... Args>
inline typename std::enable_if<!std::is_integral<T>::value>::type ofxSetLeakedOscPickerAll(T &arg, Args & ... args) {
    for(auto subscriber : ofxGetOscSubscriberManager()) {
        subscriber.second->setLeakPicker(arg, args ...);
    }
}

#pragma mark remove leaked osc picker(s)

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
    for(; it != end; ++it) {
        it->second->removeLeakPicker();
    }
}

/// \}

#define SubscribeOsc(port, name) ofxSubscribeOsc(port, "/" #name, name)
