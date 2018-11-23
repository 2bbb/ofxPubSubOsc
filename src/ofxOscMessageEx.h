//
//  ofxOscMessageEx.h
//
//  Created by ISHII 2bit on 2018/01/28.
//

#ifndef ofxOscMessageEx_h
#define ofxOscMessageEx_h

#include <type_traits>

#include "ofxOscMessage.h"

#include "Publisher/ofxOscPublisherSetImplementation.h"
#include "Subscriber/ofxOscSubscriberLoadImplementation.h"

namespace ofx {
    struct OscMessageEx : public ofxOscMessage {
        using ofxOscMessage::ofxOscMessage;
        using ofxOscMessage::operator=;
        
        OscMessageEx() {};
        OscMessageEx(const ofxOscMessage &m)
        : ofxOscMessage(m) {};
        template <typename ... arguments>
        OscMessageEx(const std::string &address, const arguments & ... args) {
            setAddress(address);
            const std::vector<ofxOscMessageEx &> &_ = {add(args) ...};
        }
        
#pragma mark add / operator<<
        
        template <typename type>
        OscMessageEx &add(const type &v) {
            PubSubOsc::set(static_cast<ofxOscMessage &>(*this), v);
            return *this;
        }
        
        template <typename type>
        OscMessageEx &operator<<(const type &v)
        { return add(v); }
        
#pragma mark operator[]
        
        struct OscArgConverter {
            OscArgConverter(const OscMessageEx &m, std::size_t index)
            : m(m)
            , index(index)
            {};
            
            template <typename type>
            inline operator type() const { return as<type>(); }
            
            template <typename type>
            inline type as() const
            {
                type v;
                PubSubOsc::load(static_cast<const ofxOscMessage &>(m), v, index);
                return v;
            };
            
        private:
            OscArgConverter() = delete;
            OscArgConverter(const OscArgConverter &) = delete;
            OscArgConverter(OscArgConverter &&) = delete;
            OscArgConverter &operator=(const OscArgConverter &) = delete;
            OscArgConverter &operator=(OscArgConverter &&) = delete;
            const OscMessageEx &m;
            const std::size_t index;
        };

        inline OscArgConverter operator[](std::size_t index) const {
            return {*this, index};
        }
        
        std::uint16_t getWaitingPort() const
        { return waitingPort; };
        void setWaitingPort(std::uint16_t port)
        { waitingPort = port; };
        
    protected:
        std::uint16_t waitingPort{0};
    };
};

using ofxOscMessageEx = ofx::OscMessageEx;

#endif /* ofxOscMessageEx_h */
