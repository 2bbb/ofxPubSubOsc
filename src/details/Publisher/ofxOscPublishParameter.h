//
//  ofxOscPublishParameter.h
//
//  Created by ISHII 2bit on 2016/05/31.
//
//

#pragma once

#ifndef ofxOscPublishParameter_h
#define ofxOscPublishParameter_h

#include <type_traits>
#include <functional>
#include <map>

#include "ofxPubSubOscSettings.h"
#include "ofxOscPublishCondition.h"
#include "ofxOscPublisherSetImplementation.h"

namespace ofx {
    namespace PubSubOsc {
        namespace Publish {
            struct AbstractParameter {
                AbstractParameter() : condition(new BasicCondition) {}
                virtual bool write(ofxOscMessage &m, const std::string &address) {
                    if(!canPublish() || !isChanged()) return false;
                    writeForce(m, address);
                    return true;
                }
                virtual void writeForce(ofxOscMessage &m, const std::string &address) = 0;
                void setCondition(ConditionRef ref) { condition = ref; };
                
                inline void setEnablePublish(bool bEnablePublish) { condition->setEnablePublish(bEnablePublish); };
                inline bool isPublishNow() const { return condition->isPublishNow(); };
            protected:
                bool canPublish() {
                    return condition->getCondition();
                }
                virtual bool isChanged() { return true; };
            private:
                ConditionRef condition;
            };
            
            using ParameterRef = std::shared_ptr<AbstractParameter>;

            template <typename T, bool isCheckValue>
            struct Parameter : AbstractParameter {
                Parameter(T &t)
                : t(t) {}
                
                virtual void writeForce(ofxOscMessage &m, const std::string &address) {
                    m.setAddress(address);
                    set(m, get());
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
            struct Parameter<Base(&)[size], true> : AbstractParameter {
                Parameter(Base (&t)[size])
                : t(t) { for(std::size_t i = 0; i < size; i++) old[i] = t[i]; }
                virtual ~Parameter() { };
                
                virtual void writeForce(ofxOscMessage &m, const std::string &address) {
                    m.setAddress(address);
                    set(m, get());
                }
                
            protected:
                virtual bool isChanged() {
                    bool isChange = false;
                    for(std::size_t i = 0; i < size; i++) {
                        isChange = isChange || (old[i] != get()[i]);
                        if(isChange) break;
                    }
                    
                    if(isChange) {
                        for(std::size_t i = 0; i < size; i++) {
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
            struct ConstParameter : AbstractParameter {
                ConstParameter(const T &t)
                : t(t) {}
                
                virtual void writeForce(ofxOscMessage &m, const std::string &address) {
                    m.setAddress(address);
                    set(m, get());
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
            struct ConstParameter<const Base(&)[size], false> : AbstractParameter {
                ConstParameter(const Base (&t)[size])
                : t(t) {}
                
                virtual void writeForce(ofxOscMessage &m, const std::string &address) {
                    m.setAddress(address);
                    set(m, get());
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
            struct FunctionParameter : Parameter<T, isCheckValue> {
                using Function = std::function<T()>;
                FunctionParameter(Function getter)
                : Parameter<T, isCheckValue>(dummy)
                , getter(getter) {}
                
            protected:
                virtual type_ref<T> get() { return dummy = getter(); }
                Function getter;
                remove_ref<T> dummy;
            };

            template <typename Base, std::size_t size, bool isCheckValue>
            struct FunctionParameter<Base(&)[size], isCheckValue> : Parameter<Base(&)[size], isCheckValue>  {
                using T = Base (&)[size];
                using Function = std::function<T()>;
                FunctionParameter(Function getter)
                : Parameter<Base(&)[size], isCheckValue>(dummy)
                , getter(getter) {}
                
            protected:
                virtual Base (&get())[size] {
                    Base (&arr)[size] = getter();
                    for(std::size_t i = 0; i < size; i++) dummy[i] = arr[i];
                        return dummy;
                }
                Function getter;
                Base dummy[size];
            };
            
            using Targets = std::multimap<std::string, ParameterRef>;
        };
    };
};

#endif /* ofxOscPublishParameter_h */
