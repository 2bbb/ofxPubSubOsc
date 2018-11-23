//
//  ofxOscSubscriber.h
//
//  Created by ISHII 2bit on 2016/05/29.
//
//

#pragma once

#ifndef ofxOscSubscriber_h
#define ofxOscSubscriber_h

#define TYPE_DEBUG(T)

#include "ofxOsc.h"

#include "ofxOscMessageEx.h"

#include "ofxOscSubscriberLoadImplementation.h"
#include "ofxOscSubscribeParameter.h"

namespace ofx {
    namespace PubSubOsc {
        namespace Subscribe {
            using OscReceiverRef = std::shared_ptr<ofxOscReceiver>;
            
            class SubscribeManager;
            
            class SubscribeIdentifier {
                std::string address;
                ParameterRef ref;
                std::uint16_t key;
                
                void invalidate() {
                    address = "";
                    ref = nullptr;
                    key = 0;
                }
            public:
                SubscribeIdentifier() : address(""), ref(nullptr) {}
                SubscribeIdentifier(const std::string &address, const ParameterRef &ref, std::uint16_t key)
                : address(address)
                , ref(ref)
                , key(key) {}
                
                SubscribeIdentifier(const SubscribeIdentifier &) = default;
                SubscribeIdentifier(SubscribeIdentifier &&) = default;
                
                SubscribeIdentifier &operator=(const SubscribeIdentifier &) = default;
                SubscribeIdentifier &operator=(SubscribeIdentifier &&) = default;
                
                const std::uint16_t getKey() const { return key; };
                bool isValid() const { return static_cast<bool>(ref); }
                
                friend class Subscriber;
            };
            
            class Subscriber {
                Targets::const_iterator findFromTargets(const SubscribeIdentifier &identifier, const Targets &targets) const {
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
                
                inline Targets::const_iterator findSubscribed(const SubscribeIdentifier &identifier) const {
                    return findFromTargets(identifier, targets);
                }
                
                template <typename T>
                inline auto make_parameter_ref(T &value)
                -> enable_if_t<!is_callable<T>::value, ParameterRef>
                {
                    return ParameterRef(new Parameter<T>(value));
                }
                
                template <typename ... Ts>
                inline ParameterRef make_parameter_ref(std::vector<ParameterRef> &&t) {
                    return ParameterRef(new TupleParameter<Ts ...>(std::move(t)));
                }
                
#pragma mark -
#pragma mark setter function
                
                template <typename R>
                inline ParameterRef make_parameter_ref(std::function<R()> setter) {
                    TYPE_DEBUG(T);
                    return ParameterRef(new SetterFunctionParameter<R, void>(setter));
                }
                
                template <typename R, typename ... Ts>
                inline ParameterRef make_parameter_ref(std::function<R(Ts ...)> setter) {
                    TYPE_DEBUG(T);
                    return ParameterRef(new SetterFunctionParameter<R, Ts ...>(setter));
                }
                
                template <typename R, typename ... Ts>
                inline ParameterRef make_parameter_ref(std::function<R(Ts && ...)> setter) {
                    TYPE_DEBUG(T);
                    return ParameterRef(new SetterFunctionParameter<R, Ts ...>(setter));
                }
                
#pragma mark setter function for lambda and method
                
                template <typename T>
                inline auto make_parameter_ref(T &value)
                -> enable_if_t<is_callable<T>::value, ParameterRef>
                {
                    TYPE_DEBUG(T);
                    return make_parameter_ref(function_traits<T>::cast(value));
                }
                
                template <typename T>
                inline auto make_parameter_ref(T &&value)
                -> enable_if_t<is_callable<T>::value, ParameterRef>
                {
                    TYPE_DEBUG(T);
                    return make_parameter_ref(function_traits<T>::cast(value));
                }
                
                template <typename T>
                inline auto make_parameter_ref(T *value)
                -> enable_if_t<is_callable<T>::value, ParameterRef>
                {
                    TYPE_DEBUG(T);
                    return make_parameter_ref(function_traits<T>::cast(value));
                }
                
                template <typename Obj, typename Meth>
                inline auto make_parameter_ref(Obj &&obj, Meth &&meth)
                -> enable_if_t<is_bindable<Obj, Meth>::value, ParameterRef>
                {
                    TYPE_DEBUG(Obj);
                    TYPE_DEBUG(Meth);
                    return make_parameter_ref(bind(std::forward<Obj>(obj),
                                                   std::forward<Meth>(meth)));
                }
                
                template <typename T, typename U, typename ... Ts>
                inline auto make_parameter_ref(T &&t, U &&u, Ts && ... ts)
                -> enable_if_t<!is_bindable<T, U>::value, ParameterRef>
                {
                    std::vector<ParameterRef> v{
                        make_parameter_ref(std::forward<T>(t)),
                        make_parameter_ref(std::forward<U>(u)),
                        make_parameter_ref(std::forward<Ts>(ts)) ...
                    };
                    return make_parameter_ref<T, U, Ts ...>(std::move(v));
                }
                
            public:
                
#pragma mark subscribe
                
                template <typename ... Ts>
                inline SubscribeIdentifier subscribe(const std::string &address, Ts && ... values) {
                    ParameterRef ref = make_parameter_ref(std::forward<Ts>(values) ...);
                    Targets::iterator it = targets.insert(std::make_pair(address, ref));
                    return {address, ref, port};
                }
                
#pragma mark unscribe
                
                inline void unsubscribe(const std::string &address) {
                    targets.erase(address);
                }
                
                inline void unsubscribe(SubscribeIdentifier &identifier) {
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
                
#pragma mark leakPicker
                
                inline void setLeakPicker(ParameterRef ref) {
                    leakPicker = ref;
                }
                
                template <typename R>
                inline void setLeakPicker(std::function<R(ofxOscMessageEx &)> callback) {
                    setLeakPicker(ParameterRef(new SetterFunctionParameter<R, ofxOscMessageEx &>(callback)));
                }
                
                template <typename T>
                inline void setLeakPicker(T &t) {
                    setLeakPicker(function_traits<T>::cast(t));
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
                    std::queue<ofxOscMessageEx> empty;
                    std::swap(leakedOscMessages, empty);
                }
                
                inline bool hasWaitingLeakedOscMessages() const {
                    return !static_cast<bool>(leakPicker) && !leakedOscMessages.empty();
                }
                
                inline bool getNextLeakedOscMessage(ofxOscMessageEx &m) {
                    if(hasWaitingLeakedOscMessages()) {
                        m.copy(leakedOscMessages.front());
                        leakedOscMessages.pop();
                        return true;
                    } else {
                        return false;
                    }
                }
                
                void notify(ofxOscMessageEx &m) {
                    const std::string &address = m.getAddress();
                    Targets::iterator it = targets.find(address);
                    if(it != targets.end()) {
                        for(std::size_t i = 0; i < targets.count(address); i++, ++it) {
                            it->second->read(m);
                        }
                    }
                }
                
                void notify(const SubscribeIdentifier &identifier, ofxOscMessageEx &m) {
                    if(!identifier.isValid()) return;
                    Targets::const_iterator it{findSubscribed(identifier)};
                    if(it != targets.end() && it->first == m.getAddress()) {
                        it->second->read(m);
                    }
                }
                
                void setEnable(bool bEnabled) { this->bEnabled = bEnabled; }
                bool isEnabled() const { return bEnabled; }
                
                using Ref = std::shared_ptr<Subscriber>;
            private:
                Subscriber(std::uint16_t port)
                : port(port) {
                    receiver.setup(port);
                }
                
                void update() {
                    clearLeakedOscMessages();
                    ofxOscMessageEx m;
                    while(receiver.hasWaitingMessages()) {
                        receiver.getNextMessage(m);
                        m.setWaitingPort(port);
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
                
                std::uint16_t port;
                ofxOscReceiver receiver;
                Targets targets;
                ParameterRef leakPicker;
                std::queue<ofxOscMessageEx> leakedOscMessages;
                bool bEnabled{true};
                
                friend class SubscriberManager;
            };
            
            class SubscriberManager {
            public:
                static SubscriberManager &getSharedInstance() {
                    static SubscriberManager *sharedInstance = new SubscriberManager;
                    return *sharedInstance;
                }
                
                static Subscriber &getOscSubscriber(std::uint16_t port) {
                    Subscribers &managers = getSharedInstance().managers;
                    if(managers.find(port) == managers.end()) {
                        managers.insert(std::make_pair(port, Subscriber::Ref(new Subscriber(port))));
                    }
                    return *(managers[port].get());
                }
                
                void setEnable(std::uint16_t port, bool bEnabled) {
                    getOscSubscriber(port).setEnable(bEnabled);
                }
                
                bool isEnabled(std::uint16_t port) const {
                    return getOscSubscriber(port).isEnabled();
                }
            private:
                using Subscribers = std::map<std::uint16_t, Subscriber::Ref>;
                void update(ofEventArgs &args) {
                    for(Subscribers::iterator it = managers.begin(); it != managers.end(); ++it) {
                        if(it->second->isEnabled()) {
                            it->second->update();
                        }
                    }
                }
                
                SubscriberManager() {
                    ofAddListener(ofEvents().update, this, &SubscriberManager::update, OF_EVENT_ORDER_BEFORE_APP);
                }
                
                virtual ~SubscriberManager() {
                    ofRemoveListener(ofEvents().update, this, &SubscriberManager::update, OF_EVENT_ORDER_BEFORE_APP);
                }
                Subscribers managers;
                
#pragma mark iterator
            public:
                using iterator = Subscribers::iterator;
                using const_iterator = Subscribers::const_iterator;
                using reverse_iterator = Subscribers::reverse_iterator;
                using const_reverse_iterator = Subscribers::const_reverse_iterator;
                
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
    };
};

using ofxOscSubscriber = ofx::PubSubOsc::Subscribe::Subscriber;
using ofxOscSubscriberManager = ofx::PubSubOsc::Subscribe::SubscriberManager;
using ofxOscSubscriberIdentifier = ofx::PubSubOsc::Subscribe::SubscribeIdentifier;

/// \brief get a OscSubscriberManager.
/// \returns ofxOscSubscriberManager

inline ofxOscSubscriberManager &ofxGetOscSubscriberManager() {
    return ofxOscSubscriberManager::getSharedInstance();
}

/// \brief get a OscSubscriber.
/// \param port binded port is typed std::uint16_t
/// \returns ofxOscSubscriber binded to port

inline ofxOscSubscriber &ofxGetOscSubscriber(std::uint16_t port) {
    return ofxOscSubscriberManager::getOscSubscriber(port);
}

#pragma mark interface about subscribe

/// \name ofxSubscribeOsc
/// \{

/// \brief bind a referece of value to the argument(s) of OSC messages with an address pattern _address_ incoming to _port_.
/// \param port binded port is typed std::uint16_t
/// \param address osc address is typed const std::string &
/// \param value reference of value is typed T &
/// \returns ofxOscSubscriberIdentifier

#pragma mark reference

template <typename T>
inline auto ofxSubscribeOsc(std::uint16_t port, const std::string &address, T &value)
-> ofx::PubSubOsc::enable_if_t<!ofx::PubSubOsc::is_callable<T>::value, ofxOscSubscriberIdentifier>
{
    return ofxGetOscSubscriber(port).subscribe(address, value);
}

template <typename T>
inline auto ofxSubscribeOsc(std::uint16_t port, const std::string &address, T callback)
-> ofx::PubSubOsc::enable_if_t<ofx::PubSubOsc::is_callable<T>::value, ofxOscSubscriberIdentifier>
{
    return ofxGetOscSubscriber(port).subscribe(address, ofx::PubSubOsc::function_traits<T>::cast(callback));
}

#pragma mark setter function/method

/// \brief bind a callback to the OSC messages with an address pattern _address_ incoming to _port_.
/// \param port binded port is typed std::uint16_t
/// \param address osc address is typed const std::string &
/// \param callback is kicked when receive a message to address
/// \returns ofxOscSubscriberIdentifier

template <typename Ret, typename ... Args>
inline ofxOscSubscriberIdentifier ofxSubscribeOsc(std::uint16_t port, const std::string &address, std::function<Ret(Args ...)> callback) {
    return ofxGetOscSubscriber(port).subscribe(address, callback);
}

template <typename Obj, typename Meth>
inline auto ofxSubscribeOsc(std::uint16_t port, const std::string &address, Obj &&obj, Meth &&meth)
-> ofx::PubSubOsc::enable_if_t<ofx::PubSubOsc::is_bindable<Obj, Meth>::value, ofxOscSubscriberIdentifier>
{
    return ofxGetOscSubscriber(port).subscribe(address, ofx::PubSubOsc::bind(std::forward<Obj>(obj),
                                                                             std::forward<Meth>(meth)));
}

template <typename T, typename U, typename ... Ts>
inline auto ofxSubscribeOsc(std::uint16_t port, const std::string &address, T &&t, U &&u, Ts && ... values)
-> ofx::PubSubOsc::enable_if_t<!ofx::PubSubOsc::is_bindable<T, U>::value, ofxOscSubscriberIdentifier>
{
    return ofxGetOscSubscriber(port).subscribe(address, std::forward<T>(t), std::forward<U>(u), std::forward<Ts>(values) ...);
}

#pragma mark subscribe multiple port at once

template <typename ... Args>
inline void ofxSubscribeOsc(const std::initializer_list<std::uint16_t> &ports, const std::string &address, Args & ... args) {
    for(auto &port : ports) {
        ofxSubscribeOsc(port, address, args ...);
    }
}

template <typename ... Args>
inline void ofxSubscribeOsc(std::uint16_t port, const std::initializer_list<const std::string> &addresses, Args & ... args) {
    auto &subscriber = ofxGetOscSubscriber(port);
    for(auto &address : addresses) {
        subscriber.subscribe(address, args ...);
    }
}

template <typename ... Args>
inline void ofxSubscribeOsc(const std::initializer_list<std::uint16_t> &ports, const std::initializer_list<const std::string> &addresses, Args & ... args) {
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
/// \param port binded port is typed std::uint16_t
/// \param address osc address is typed const std::string &
/// \returns void

inline void ofxUnsubscribeOsc(std::uint16_t port, const std::string &address) {
    ofxGetOscSubscriber(port).unsubscribe(address);
}

/// \brief unbind from OSC messages with any address patterns incoming to _port_.
/// \param port binded port is typed std::uint16_t
/// \returns void

inline void ofxUnsubscribeOsc(std::uint16_t port) {
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

inline void ofxNotifyToSubscribedOsc(ofxOscSubscriberIdentifier &identifier, ofxOscMessageEx &m) {
    ofxGetOscSubscriber(identifier.getKey()).notify(m);
}

inline void ofxNotifyToSubscribedOsc(std::uint16_t port, ofxOscMessageEx &m) {
    ofxGetOscSubscriber(port).notify(m);
}

inline void ofxNotifyToSubscribedOsc(ofxOscMessageEx &m) {
    ofxOscSubscriberManager &manager = ofxGetOscSubscriberManager();
    ofxOscSubscriberManager::iterator it  = manager.begin(),
    end = manager.end();
    for(; it != end; ++it) {
        it->second->notify(m);
    }
}

#pragma mark activate / deactivate listening

inline void ofxSetOscSubscriberActive(std::uint16_t port, bool bActive) {
    ofxGetOscSubscriber(port).setEnable(bActive);
}

inline bool ofxGetOscSubscriberActive(std::uint16_t port) {
    return ofxGetOscSubscriber(port).isEnabled();
}

#pragma mark interface about leaked osc

/// \name ofxSetLeakedOscPicker
/// \{

/// \brief bind a callback to the OSC messages with are not match other patterns incoming to port.
/// \param port binded port is typed std::uint16_t
/// \callback is kicked when receive a leaked addresses
/// \returns void

template <typename Callback>
inline void ofxSetLeakedOscPicker(std::uint16_t port, Callback callback) {
    ofxGetOscSubscriber(port).setLeakPicker(callback);
}

template <typename Obj, typename Meth>
inline void ofxSetLeakedOscPicker(std::uint16_t port, Obj obj, Meth meth) {
    ofxGetOscSubscriber(port).setLeakPicker(ofx::PubSubOsc::bind(obj, meth));
}

#pragma mark leak picking all port

template <typename Callback>
inline void ofxSetLeakedOscPickerAll(Callback callback) {
    for(auto subscriber : ofxGetOscSubscriberManager()) {
        subscriber.second->setLeakPicker(callback);
    }
}

template <typename Obj, typename Meth>
inline void ofxSetLeakedOscPickerAll(Obj obj, Meth meth) {
    ofxSetLeakedOscPickerAll(ofx::PubSubOsc::bind(obj, meth));
}

#pragma mark remove leaked osc picker(s)

/// \brief remove a callback receives messages has leaked patterns incoming to port.
/// \param port binded port is typed std::uint16_t
/// \returns void

inline void ofxRemoveLeakedOscPicker(std::uint16_t port) {
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

#endif /* ofxOscSubscriber_h */
