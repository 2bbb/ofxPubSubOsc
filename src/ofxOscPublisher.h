//
//  ofxPublisher.h
//
//  Created by ISHII 2bit on 2016/05/29.
//
//

#pragma once

#ifndef ofxPublisher_h
#define ofxPublisher_h

#include "ofxOsc.h"

#include "ofxOscPublisherSetImplementation.h"
#include "ofxOscPublishCondition.h"
#include "ofxOscPublishParameter.h"
#include "ofxOscPublisherStructs.h"

namespace ofx {
    namespace PubSubOsc {
        namespace Publish {
            class PublishIdentifier {
                std::string address;
                ParameterRef ref;
                Destination key;
                
                void invalidate() {
                    address = "";
                    ref = nullptr;
                    key = Destination();
                }
            public:
                PublishIdentifier() {}
                PublishIdentifier(const std::string &address, const ParameterRef &ref, const Destination &key)
                : address(address)
                , ref(ref)
                , key(key) {}
                
                const Destination &getKey() const { return key; };
                bool isValid() const { return static_cast<bool>(ref); }
                
                friend class Publisher;
            };
            
            class Publisher {
                Targets::const_iterator findFromTargets(const PublishIdentifier &identifier, const Targets &targets) const {
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
                
                inline Targets::const_iterator findPublished(const PublishIdentifier &identifier) const {
                    return findFromTargets(identifier, targets);
                }
                
                inline Targets::const_iterator findRegistered(const PublishIdentifier &identifier) const {
                    return findFromTargets(identifier, registeredTargets);
                }
                
                template <typename T>
                inline auto make_parameter_ref(T &value, bool whenValueIsChanged = false)
                -> enable_if_t<!is_callable<T>::value, ParameterRef>
                {
                    if(whenValueIsChanged) return ParameterRef(new Parameter<T, true>(value));
                    else                   return ParameterRef(new Parameter<T, false>(value));
                }
                
                template <typename T>
                inline auto make_parameter_ref(const T &value, bool whenValueIsChanged = false)
                -> enable_if_t<!is_callable<T>::value, ParameterRef>
                {
                    if(whenValueIsChanged) return ParameterRef(new ConstParameter<T, true>(value));
                    else                   return ParameterRef(new ConstParameter<T, false>(value));
                }
                
                template <typename T>
                inline auto make_parameter_ref(std::function<T()> func, bool whenValueIsChanged = false)
                -> enable_if_t<!is_callable<T>::value, ParameterRef>
                {
                    if(whenValueIsChanged) return ParameterRef(new FunctionParameter<T, true>(func));
                    else                   return ParameterRef(new FunctionParameter<T, false>(func));
                }
                
                template <typename Func>
                inline auto make_parameter_ref(Func &func, bool whenValueIsChanged = false)
                -> enable_if_t<is_callable<Func>::value, ParameterRef>
                {
                    return make_parameter_ref(function_traits<Func>::cast(func), whenValueIsChanged);
                }
                
                template <typename Obj, typename Meth>
                inline auto make_parameter_ref(Obj &&obj, Meth &&meth, bool whenValueIsChanged = false)
                -> enable_if_t<is_bindable<Obj, Meth>::value, ParameterRef>
                {
                    return make_parameter_ref(bind(std::forward<Obj>(obj), std::forward<Meth>(meth)), whenValueIsChanged);
                }
                
                inline PublishIdentifier publish_impl(const std::string &address, ParameterRef ref) {
                    targets.insert(std::make_pair(address, ref));
                    return {address, ref, destination};
                }
                
            public:
    #pragma mark publish
                
                PublishIdentifier publish(const std::string &address, const char * const value, bool whenValueIsChanged = true) {
                    ParameterRef p = make_parameter_ref(value, whenValueIsChanged);
                    return publish_impl(address, p);
                }
                
                template <typename T>
                auto publish(const std::string &address, T &value, bool whenValueIsChanged = true)
                -> enable_if_t<!is_callable<T>::value, PublishIdentifier>
                {
                    ParameterRef p = make_parameter_ref(value, whenValueIsChanged);
                    return publish_impl(address, p);
                }
                
                template <typename T>
                auto publish(const std::string &address, const T &value, bool whenValueIsChanged = true)
                -> enable_if_t<!is_callable<T>::value, PublishIdentifier>
                {
                    ParameterRef p = make_parameter_ref(value, whenValueIsChanged);
                    return publish_impl(address, p);
                }
                
                template <typename T>
                PublishIdentifier publish(const std::string &address, std::function<T()> getter, bool whenValueIsChanged = true) {
                    ParameterRef p = make_parameter_ref(getter, whenValueIsChanged);
                    return publish_impl(address, p);
                }
                
                template <typename Func>
                auto publish(const std::string &address, Func &&func, bool whenValueIsChanged = true)
                -> enable_if_t<is_callable<Func>::value, PublishIdentifier>
                {
                    return publish(address, function_traits<Func>::cast(func));
                }
                
                template <typename Obj, typename Meth>
                auto publish(const std::string &address, Obj &&obj, Meth &&meth, bool whenValueIsChanged = true)
                -> enable_if_t<is_bindable<Obj, Meth>::value, PublishIdentifier>
                {
                    ParameterRef p = make_parameter_ref(obj, meth);
                    return publish(address, p, whenValueIsChanged);
                }
                
#pragma mark publish conditional
#pragma mark condition is bool value ref
                
                PublishIdentifier publishIf(bool &condition, const std::string &address, const char * const value) {
                    ParameterRef p = make_parameter_ref(value);
                    p->setCondition(ConditionRef(new Condition(condition)));
                    return publish_impl(address, p);
                }
                
                template <typename T>
                PublishIdentifier publishIf(bool &condition, const std::string &address, T &value) {
                    ParameterRef p = make_parameter_ref(value);
                    p->setCondition(ConditionRef(new Condition(condition)));
                    return publish_impl(address, p);
                }
                
                template <typename T>
                PublishIdentifier publishIf(bool &condition, const std::string &address, const T &value) {
                    ParameterRef p = make_parameter_ref(value);
                    p->setCondition(ConditionRef(new Condition(condition)));
                    return publish_impl(address, p);
                }
                
                template <typename T>
                PublishIdentifier publishIf(bool &condition, const std::string &address, std::function<T()> getter) {
                    ParameterRef p = make_parameter_ref(getter);
                    p->setCondition(ConditionRef(new Condition(condition)));
                    return publish_impl(address, p);
                }
                
                template <typename Obj, typename Meth>
                auto publishIf(bool &condition, const std::string &address, Obj &&obj, Meth &&meth)
                -> enable_if_t<is_bindable<Obj, Meth>::value, PublishIdentifier>
                {
                    return publishIf(condition, address, bind(obj, meth));
                }
                
    #pragma mark condition is function
                
                template <typename T>
                PublishIdentifier publishIf(std::function<bool()> condition, const std::string &address, T &value) {
                    ParameterRef p = make_parameter_ref(value);
                    p->setCondition(std::shared_ptr<BasicCondition>(new ConditionFunction(condition)));
                    return publish_impl(address, p);
                }
                
                template <typename T>
                PublishIdentifier publishIf(std::function<bool()> condition, const std::string &address, std::function<T()> getter) {
                    ParameterRef p = make_parameter_ref(getter);
                    p->setCondition(std::shared_ptr<BasicCondition>(new ConditionFunction(condition)));
                    return publish_impl(address, p);
                }
                
                template <typename Obj, typename Meth>
                auto publishIf(std::function<bool()> condition, const std::string &address, Obj &&obj, Meth &&meth)
                -> enable_if_t<is_bindable<Obj, Meth>::value, PublishIdentifier>
                {
                    return publishIf(condition, address, bind(obj, meth));
                }
                
    #pragma mark condition is method
                
                template <typename CondObj, typename CondMeth, typename ... Args>
                auto publishIf(CondObj &&obj, CondMeth &&meth, Args && ... args)
                -> enable_if_t<is_bindable<CondObj, CondMeth>::value, PublishIdentifier>
                {
                    return publishIf(bind(obj, meth), std::forward<Args>(args) ...);
                }
                
    #pragma mark unpublish
                
                void unpublish(const std::string &address) {
                    if(targets.find(address) != targets.end()) targets.erase(address);
                }
                
                void unpublish(PublishIdentifier &identifier) {
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
                
                void stopPublishTemporary(const PublishIdentifier &identifier) {
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
                
                void resumePublishTemporary(const PublishIdentifier &identifier) {
                    if(!identifier.isValid()) return;
                    Targets::const_iterator it{findPublished(identifier)};
                    if(it != targets.end()) {
                        it->second->setEnablePublish(true);
                    }
                }
                
    #pragma mark doRegister
                
                inline PublishIdentifier doRegister(const std::string &address, ParameterRef ref) {
                    registeredTargets.insert(std::make_pair(address, ref));
                    return {address, ref, destination};
                }
                
                template <typename T>
                PublishIdentifier doRegister(const std::string &address, T &value) {
                    return doRegister(address, make_parameter_ref(value));
                }
                
                template <typename T>
                PublishIdentifier doRegister(const std::string &address, std::function<T()> getter) {
                    return doRegister(address, make_parameter_ref(getter));
                }
                
                template <typename Obj, typename Meth>
                auto doRegister(const std::string &address, Obj &&obj, Meth &&meth)
                -> enable_if_t<is_bindable<Obj, Meth>::value, PublishIdentifier>
                {
                    return doRegister(address, bind(obj, meth));
                }
                
    #pragma mark publishRegistered
                
                inline void publishRegistered(const std::string &address) {
                    Targets::iterator it = registeredTargets.find(address);
                    if(it == registeredTargets.end()) {
                        ofLogWarning("ofxPubSubOsc") << address << " is not registered.";
                    }
                    ofxOscMessage m;
                    for(std::size_t i = 0, size = registeredTargets.count(address); i < size; i++, ++it) {
                        if(it->second->write(m, it->first)) sender.sendMessage(m);
                        m.clear();
                    }
                }
                
                inline void publishRegistered(const PublishIdentifier &identifier) {
                    if(!identifier.isValid()) return;
                    Targets::const_iterator it{findRegistered(identifier)};
                    if(it != registeredTargets.end()) {
                        ofxOscMessage m;
                        if(it->second->write(m, it->first)) sender.sendMessage(m);
                        m.clear();
                    }
                }
                
    #pragma mark unregister
                
                inline void unregister(const std::string &address) {
                    if(registeredTargets.find(address) == registeredTargets.end()) registeredTargets.erase(address);
                }
                
                inline void unregister(PublishIdentifier &identifier) {
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
                
    #pragma mark publishManually
                
                inline void publishManullay(const std::string &address) {
                    {
                        Targets::iterator it = registeredTargets.find(address);
                        if(it != registeredTargets.end()) {
                            ofxOscMessage m;
                            for(std::size_t i = 0, size = registeredTargets.count(address); i < size; i++, ++it) {
                                it->second->writeForce(m, it->first);
                                sender.sendMessage(m);
                                m.clear();
                            }
                        }
                    }
                    
                    {
                        Targets::iterator it = targets.find(address);
                        if(it != targets.end()) {
                            ofxOscMessage m;
                            for(std::size_t i = 0, size = targets.count(address); i < size; i++, ++it) {
                                it->second->writeForce(m, it->first);
                                sender.sendMessage(m);
                                m.clear();
                            }
                        }
                    }
                }
                
                inline void publishManullay(const PublishIdentifier &identifier) {
                    if(!identifier.isValid()) return;
                    Targets::const_iterator it{findPublished(identifier)};
                    if(it != targets.end()) {
                        ofxOscMessage m;
                        it->second->writeForce(m, it->first);
                        sender.sendMessage(m);
                        m.clear();
                    }
                }
                
    #pragma mark status
                
                inline bool isPublished() const {
                    return !targets.empty();
                }
                
                inline bool isPublished(const PublishIdentifier &identifier) const {
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
                
                inline bool isEnabled(const PublishIdentifier &identifier) const {
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
                
                using Ref = std::shared_ptr<Publisher>;
                
                static bool &bUseBundle() {
                    static bool b;
                    return b;
                }
                
                static void setUseBundle(bool b) {
                    bUseBundle() = b;
                }
                
                static bool isUseBundle() {
                    return bUseBundle();
                }
                
                ofxOscSender &getSender() { return sender; }
                
#pragma mark send
                
                template <typename ... Args>
                void send(const std::string &address, Args && ... args) {
                    ofxOscMessage m = createMessage(address, std::forward<Args>(args) ...);
                    sender.sendMessage(m);
                }
                
            private:
                template <typename Arg>
                void createMessageImpl(ofxOscMessage &m, Arg &&arg) {
                    set(m, std::forward<Arg>(arg));
                }
                
                template <typename Arg, typename ... Args>
                auto createMessageImpl(ofxOscMessage &m, Arg &&arg, Args && ... args)
                -> enable_if_t<0 < sizeof...(Args), void>
                {
                    set(m, std::forward<Arg>(arg));
                    createMessageImpl(m, std::forward<Args>(args) ...);
                }
                
                template <typename ... Args>
                auto createMessage(const std::string &address, Args && ... args)
                -> enable_if_t<0 < sizeof...(Args), ofxOscMessage>
                {
                    ofxOscMessage m;
                    m.setAddress(address);
                    createMessageImpl(m, std::forward<Args>(args) ...);
                    return m;
                }

                ofxOscMessage createMessage(const std::string &address) {
                    ofxOscMessage m;
                    m.setAddress(address);
                    return m;
                }

                Publisher(const Destination &destination)
                : destination(destination) {
                    sender.setup(destination.ip, destination.port);
                }
                
                void update() {
                    ofxOscMessage m;
                    if(isUseBundle()) {
                        ofxOscBundle bundle;
                        for(Targets::iterator it = targets.begin(); it != targets.end(); ++it) {
                            if(it->second->write(m, it->first)) bundle.addMessage(m);
                            m.clear();
                        }
                        if(bundle.getMessageCount()) sender.sendBundle(bundle);
                        bundle.clear();
                        return;
                    }
                    for(Targets::iterator it = targets.begin(); it != targets.end(); ++it) {
                        if(it->second->write(m, it->first)) sender.sendMessage(m);
                        m.clear();
                    }
                    
                }
                
                Destination destination;
                ofxOscSender sender;
                Targets targets;
                Targets registeredTargets;
                friend class PublisherManager;
            };
            
            class PublisherManager {
            public:
                using Publishers = std::map<Destination, Publisher::Ref>;
                
                static PublisherManager &getSharedInstance() {
                    static PublisherManager *sharedInstance = new PublisherManager;
                    return *sharedInstance;
                }
                
                static Publisher &getOscPublisher(const std::string &ip, std::uint16_t port) {
                    Publishers &publishers = getSharedInstance().publishers;
                    Destination destination(ip, port);
                    if(publishers.find(destination) == publishers.end()) {
                        publishers.insert(std::make_pair(destination, Publisher::Ref(new Publisher(destination))));
                    }
                    return *(publishers[destination].get());
                }
                
                void update(ofEventArgs &args) {
                    for(Publishers::iterator it = publishers.begin(); it != publishers.end(); ++it) {
                        it->second->update();
                    }
                }
                PublisherManager() {
                    ofAddListener(ofEvents().update, this, &PublisherManager::update, OF_EVENT_ORDER_AFTER_APP);
                }
                virtual ~PublisherManager() {
                    ofRemoveListener(ofEvents().update, this, &PublisherManager::update, OF_EVENT_ORDER_AFTER_APP);
                }
                Publishers publishers;
                
    #pragma mark iterator
            public:
                using iterator = Publishers::iterator;
                using const_iterator = Publishers::const_iterator;
                using reverse_iterator = Publishers::reverse_iterator;
                using const_reverse_iterator = Publishers::const_reverse_iterator;
                
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
    };
};

#pragma mark - syntax sugars

#pragma mark getter

using ofxOscPublisher = ofx::PubSubOsc::Publish::Publisher;
using ofxOscPublisherManager = ofx::PubSubOsc::Publish::PublisherManager;
using ofxOscPublisherIdentifier = ofx::PubSubOsc::Publish::PublishIdentifier;
using ofxOscPublisherDestination = ofx::PubSubOsc::Publish::Destination;
                
/// \brief get a OscPublisherManager.
/// \returns ofxOscPublisherManager

inline ofxOscPublisherManager &ofxGetOscPublisherManager() {
    return ofxOscPublisherManager::getSharedInstance();
}

/// \brief get a OscPublisher.
/// \param ip target ip is typed const std::string &
/// \param port target port is typed std::uint16_t
/// \returns ofxOscPublisher binded to ip & port

inline ofxOscPublisher &ofxGetOscPublisher(const std::string &ip, std::uint16_t port) {
    return ofxOscPublisherManager::getOscPublisher(ip, port);
}

#pragma mark publish

/// \name ofxPublishOsc
/// \{

/// \brief publish value as an OSC message with an address pattern address to ip:port every time the value has changed.
/// If whenValueIsChanged is set to false, then the binded value is sent every frame after App::update.
/// template parameter T is suggested by value
/// \param ip target ip is typed const std::string &
/// \param port target port is typed std::uint16_t
/// \param address osc address is typed const std::string &
/// \param value reference of value is typed T &
/// \param whenValueIsChanged if this value to false, then we send value every update
/// \returns ofxOscPublisherIdentifier

template <typename ValueOrFunction>
inline ofxOscPublisherIdentifier ofxPublishOsc(const std::string &ip, std::uint16_t port, const std::string &address, ValueOrFunction &&valueOrFunction, bbb::explicit_bool whenValueIsChanged = true)
{
    return ofxGetOscPublisher(ip, port).publish(address, std::forward<ValueOrFunction>(valueOrFunction), whenValueIsChanged.get());
}

/// \brief publish the value will be gave by function as an OSC message with an address pattern address to ip:port every time the value has changed.
/// If whenValueIsChanged is set to false, then the binded value is sent every frame after App::update.
/// template parameter ObjectPtrOrRef is suggested by that, and Method is suggested by getter.
/// \param ip target ip is typed const std::string &
/// \param port target port is typed std::uint16_t
/// \param address osc address is typed const std::string &
/// \param that this object is typed ObjectPtrOrRef, will bind with next parameter method. is called as (that->*getter)() or (that.*getter)().
/// \param getter this method gives value, is typed T(C::*)()
/// \param whenValueIsChanged if this value to false, then we send value every update
/// \returns ofxOscPublisherIdentifier

template <typename Object, typename Method>
inline auto ofxPublishOsc(const std::string &ip, std::uint16_t port, const std::string &address, Object &&obj, Method &&meth, bbb::explicit_bool whenValueIsChanged = true)
-> ofx::PubSubOsc::enable_if_t<ofx::PubSubOsc::is_bindable<Object, Method>::value, ofxOscPublisherIdentifier>
{
    return ofxGetOscPublisher(ip, port).publish(address, ofx::PubSubOsc::bind(std::forward<Object>(obj), std::forward<Method>(meth)), whenValueIsChanged);
}
                
template <typename ... Args>
inline void ofxPublishOsc(const std::string &ip, const std::initializer_list<std::uint16_t> ports, Args & ... args) {
    for(auto port : ports) {
        ofxGetOscPublisher(ip, port).publish(args ...);
    }
}

template <typename ... Args>
inline void ofxPublishOsc(const std::initializer_list<ofx::PubSubOsc::Publish::IP> &ips, Args & ... args) {
    for(auto &ip : ips) {
        ofxPublishOsc(ip, args ...);
    }
}

template <typename ... Args>
inline void ofxPublishOsc(const std::initializer_list<ofx::PubSubOsc::Publish::Destination> &targets, Args & ... args) {
    for(auto &target : targets) {
        ofxPublishOsc(target.ip, target.port, args ...);
    }
}

template <typename ... Args>
inline void ofxPublishOsc(const std::initializer_list<ofx::PubSubOsc::Publish::DestinationWithAddress> &targets, Args & ... args) {
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
/// \param port target port is typed std::uint16_t
/// \param address osc address is typed const std::string &
/// \param value reference of value is typed T &
/// \param whenValueIsChanged if this value to false, then we send value every update
/// \returns ofxOscPublisherIdentifier

template <typename ConditionValueRef, typename ValueRefOrGetterFunction>
inline ofxOscPublisherIdentifier ofxPublishOscIf(ConditionValueRef &&condition, const std::string &ip, std::uint16_t port, const std::string &address, ValueRefOrGetterFunction &&valueRefOrGetterFunction) {
    return ofxGetOscPublisher(ip, port).publishIf(condition, address, valueRefOrGetterFunction);
}

template <typename ConditionValueRef, typename ObjectPtrOrRef, typename GetterMethod>
inline ofxOscPublisherIdentifier ofxPublishOscIf(ConditionValueRef &&condition, const std::string &ip, std::uint16_t port, const std::string &address, ObjectPtrOrRef &&that, GetterMethod getter) {
    return ofxGetOscPublisher(ip, port).publishIf(condition, address, that, getter);
}

#pragma mark condition is method

template <typename ConditionObjectPtrOrRef, typename ConditionMethodReturnsBool, typename ValueRefOrFunction>
inline ofxOscPublisherIdentifier ofxPublishOscIf(ConditionObjectPtrOrRef &&condition, ConditionMethodReturnsBool method, const std::string &ip, std::uint16_t port, const std::string &address, ValueRefOrFunction &&valueRefOrGetterFunction) {
    return ofxGetOscPublisher(ip, port).publishIf(condition, method, address, valueRefOrGetterFunction);
}

template <typename ConditionObjectPtrOrRef, typename ConditionMethodReturnsBool, typename ObjectPtrOrRef, typename GetterMethod>
inline ofxOscPublisherIdentifier ofxPublishOscIf(ConditionObjectPtrOrRef &&condition, ConditionMethodReturnsBool method, const std::string &ip, std::uint16_t port, const std::string &address, ObjectPtrOrRef &&that, GetterMethod getter) {
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

inline void ofxUnpublishOsc(const std::string &ip, std::uint16_t port, const std::string &address) {
    ofxGetOscPublisher(ip, port).unpublish(address);
}

inline void ofxUnpublishOsc(const std::string &ip, std::uint16_t port) {
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
inline ofxOscPublisherIdentifier ofxRegisterPublishingOsc(const std::string &ip, std::uint16_t port, const std::string &address, ValueOrGetterFunctionType &&valueOrGetterFunction) {
    return ofxGetOscPublisher(ip, port).doRegister(address, valueOrGetterFunction);
}

template <typename ObjectPtrOrRef, typename GetterMethod>
inline ofxOscPublisherIdentifier ofxRegisterPublishingOsc(const std::string &ip, std::uint16_t port, const std::string &address, ObjectPtrOrRef &&that, GetterMethod &&getter) {
    return ofxGetOscPublisher(ip, port).doRegister(address, that, getter);
}

/// \}

#pragma mark publish registered

/// \name ofxPublishRegisteredOsc
/// \{

inline void ofxPublishRegisteredOsc(const std::string &ip, std::uint16_t port, const std::string &address) {
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

inline void ofxUnregisterPublishingOsc(const std::string &ip, std::uint16_t port, const std::string &address) {
    ofxGetOscPublisher(ip, port).unregister(address);
}

inline void ofxUnregisterPublishingOsc(const std::string &ip, std::uint16_t port) {
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

#pragma mark publish registered
                
/// \name ofxPublishOscManually
/// \{

inline void ofxPublishOscManually(const std::string &ip, std::uint16_t port, const std::string &address) {
    ofxGetOscPublisher(ip, port).publishManullay(address);
}

inline void ofxPublishOscManually(const ofxOscPublisherIdentifier &identifier) {
    if(!identifier.isValid()) return;
    ofxGetOscPublisher(identifier.getKey().ip, identifier.getKey().port).publishManullay(identifier);
}

/// \}

#pragma mark send

template <typename ... Args>
void ofxSendOsc(const std::string &ip, std::uint16_t port, const std::string &address, Args && ... args) {
    ofxGetOscPublisher(ip, port).send(address, std::forward<Args>(args) ...);
}

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
    return reinterpret_cast<T(&)[size]>(reinterpret_cast<T&>(ptr[0]));
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
               reinterpret_cast<T&(U::*)()const>(getter)
    );
}

/// \}
#endif /* ofxPublisher_h */
