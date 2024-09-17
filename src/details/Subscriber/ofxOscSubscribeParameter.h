//
//  ofxOscSubscribeParameter.h
//  ofxPubSubOscDev
//
//  Created by ISHII 2bit on 2016/05/29.
//
//

#pragma once

#ifndef ofxOscSubscribeParameter_h
#define ofxOscSubscribeParameter_h

#include "ofxOscMessage.h"
#include "ofxOscMessageEx.h"
#include "ofxOscSubscriberLoadImplementation.h"

namespace ofx {
    namespace PubSubOsc {
        namespace Subscribe {
            struct AbstractParameter {
                virtual void read(const ofxOscMessageEx &message,
                                  std::size_t offset = 0) = 0;
                virtual std::size_t size() const = 0;
                void setEnable(bool bEnabled) { this->bEnabled = bEnabled; }
                bool isEnabled() const { return bEnabled; }
            protected:
                bool bEnabled{true};
            };
            
            using ParameterRef = std::shared_ptr<AbstractParameter>;
            
            template <typename T>
            struct Parameter
                : AbstractParameter
                , subscriber_type_traits<T>
            {
                Parameter(T &t) : t(t) {}
                virtual void read(const ofxOscMessageEx &message,
                                  std::size_t offset = 0) override
                { PubSubOsc::load(message, t, offset); }
                virtual std::size_t size() const override { return subscriber_type_traits<T>::size; };
                
            private:
                
                T &t;
            };
            
            template <typename ... Ts>
            struct TupleParameter
                : AbstractParameter
                , subscriber_type_traits<std::tuple<Ts ...>>
            {
                TupleParameter(std::vector<ParameterRef> &&t) : t(std::move(t)) {}
                virtual void read(const ofxOscMessageEx &message,
                                  std::size_t offset = 0) override
                { load(message, t, offset); }
                virtual std::size_t size() const override { return subscriber_type_traits<std::tuple<Ts ...>>::size; };
                
            private:
                
                std::vector<ParameterRef> t;
            };
            
            namespace detail {
                template <typename ... Ts, std::size_t ... Indices>
                void read_to_tuple(index_sequence<Indices ...> &&,
                                   const ofxOscMessageEx &m,
                                   std::tuple<Ts ...> &t,
                                   std::size_t offset)
                {
                    std::size_t o{0};
                    std::vector<std::size_t>{(PubSubOsc::load(m, std::get<Indices>(t), offset + o), o += subscriber_type_traits<Ts>::size) ...};
                }
                
                template <typename ... Ts>
                void read_to_tuple(const ofxOscMessageEx &m,
                                   std::tuple<Ts ...> &t,
                                   std::size_t offset)
                { read_to_tuple(index_sequence_for<Ts ...>(), m, t, offset); }
            };
            
            template <typename R, typename ... Ts>
            struct SetterFunctionParameter
                : AbstractParameter
                , subscriber_type_traits<std::tuple<Ts ...>>
            {
                using Setter = std::function<R(Ts ...)>;
                SetterFunctionParameter(Setter setter) : setter(std::move(setter)) {};
                
                virtual void read(const ofxOscMessageEx &message,
                                  std::size_t offset = 0) override
                {
                    std::tuple<remove_const_reference<Ts> ...> t;
                    Subscribe::detail::read_to_tuple<remove_const_reference<Ts> ...>(message, t, offset);
                    apply<R, Ts ...>(setter, t);
                }
                virtual std::size_t size() const override { return subscriber_type_traits<std::tuple<Ts ...>>::size; };
                
            private:
                Setter setter;
            };
            
            template <typename R>
            struct SetterFunctionParameter<R, const ofxOscMessageEx &>
                : AbstractParameter
                , subscriber_type_traits<ofxOscMessageEx>
            {
                using Setter = std::function<R(const ofxOscMessageEx &)>;
                SetterFunctionParameter(Setter setter) : setter(std::move(setter)) {};
                
                virtual void read(const ofxOscMessageEx &message,
                                  std::size_t offset = 0) override
                { setter(message); }
                virtual std::size_t size() const override { return subscriber_type_traits<ofxOscMessageEx>::size; };
                
            private:
                Setter setter;
            };

            template <typename R>
            struct SetterFunctionParameter<R, ofxOscMessageEx &>
                : AbstractParameter
                , subscriber_type_traits<ofxOscMessageEx>
            {
                using Setter = std::function<R(ofxOscMessageEx &)>;
                SetterFunctionParameter(Setter setter) : setter(std::move(setter)) {};
                
                virtual void read(const ofxOscMessageEx &message,
                                  std::size_t offset = 0) override
                {
                    auto m = message;
                    setter(m);
                }
                virtual std::size_t size() const override { return subscriber_type_traits<ofxOscMessageEx>::size; };
                
            private:
                Setter setter;
            };
            
            template <typename R>
            struct SetterFunctionParameter<R, const ofxOscMessage &>
                : AbstractParameter
                , subscriber_type_traits<ofxOscMessage>
            {
                using Setter = std::function<R(const ofxOscMessage &)>;
                SetterFunctionParameter(Setter setter) : setter(std::move(setter)) {};
                
                virtual void read(const ofxOscMessageEx &message,
                                  std::size_t offset = 0) override
                { setter(message); }
                virtual std::size_t size() const override { return subscriber_type_traits<ofxOscMessage>::size; };
                
            private:
                Setter setter;
            };

            template <typename R>
            struct SetterFunctionParameter<R, ofxOscMessage &>
                : AbstractParameter
                , subscriber_type_traits<ofxOscMessage>
            {
                using Setter = std::function<R(ofxOscMessage &)>;
                SetterFunctionParameter(Setter setter) : setter(std::move(setter)) {};
                
                virtual void read(const ofxOscMessageEx &message,
                                  std::size_t offset = 0) override
                {
                    auto m = message;
                    setter(m);
                }
                virtual std::size_t size() const override { return subscriber_type_traits<ofxOscMessage>::size; };
                
            private:
                Setter setter;
            };
            
            template <typename T, typename ... Ts>
            struct SetterFunctionParameter<
                enable_if_t<std::is_same<ofxOscMessage, T>::value>, T, Ts ...
            >
                : AbstractParameter
                , subscriber_type_traits<std::tuple<T, Ts ...>>
            {
                using Setter = std::function<void(T, Ts ...)>;
                SetterFunctionParameter(Setter setter) : setter(std::move(setter)) {};
                
                virtual void read(const ofxOscMessageEx &message,
                                  std::size_t offset = 0) override
                {
                    std::tuple<T, Ts ...> t;
                    Subscribe::detail::read_to_tuple(message, t, offset);
                    apply(setter, t);
                }
                virtual std::size_t size() const override { return subscriber_type_traits<std::tuple<T, Ts ...>>::size; };
                
            private:
                Setter setter;
            };

            template <typename R>
            struct SetterFunctionParameter<R, void>
                : AbstractParameter
                , subscriber_type_traits<void>
            {
                using Setter = std::function<R()>;
                SetterFunctionParameter(Setter setter) : setter(std::move(setter)) {};
                
                virtual void read(const ofxOscMessageEx &message,
                                  std::size_t offset = 0) override
                { setter(); }
                virtual std::size_t size() const override { return subscriber_type_traits<void>::size; };
                
            private:
                Setter setter;
            };
            
            
#pragma mark load for ParameterRef
            inline void load(const ofxOscMessageEx &m,
                             ParameterRef &ref,
                             std::size_t offset = 0)
            { ref->read(m, offset); }
            
            inline void load(const ofxOscMessageEx &m,
                             std::vector<ParameterRef> &v,
                             std::size_t offset = 0)
            {
                std::size_t o = 0;
                for(std::size_t i = 0; i < v.size(); i++) {
                    if(m.getNumArgs() < offset + o + v[i]->size()) {
                        ofLogWarning("ofxPubSubOsc::Subscriber") << "less args";
                        break;
                    }
                    load(m, v[i], offset + o);
                    o += v[i]->size();
                }
            }
            
            using Targets = std::multimap<std::string, Subscribe::ParameterRef>;
        };
    };
};

#endif /* ofxOscSubscribeParameter_h */
