//
//  ofxOscMessageEx.h
//
//  Created by ISHII 2bit on 2018/01/28.
//

#ifndef ofxOscMessageEx_h
#define ofxOscMessageEx_h

#include <type_traits>

#include "ofxOscMessage.h"

namespace ofx {
    struct OscMessageEx : public ofxOscMessage {
        struct OscArgConverter {
            OscArgConverter(const OscMessageEx &m, std::size_t index)
            : m(m)
            , index(index)
            {};
            
            template <typename type>
            inline operator type() const { return as<type>(); }
            
            template <typename type>
            auto as() const
                -> typename std::enable_if<std::is_integral<type>::value, type>::type
            {
                switch(m.getArgType(index)) {
                    case OFXOSC_TYPE_INT32:  return m.getArgAsInt32(index);
                    case OFXOSC_TYPE_INT64:  return m.getArgAsInt64(index);
                    case OFXOSC_TYPE_FLOAT:  return m.getArgAsFloat(index);
                    case OFXOSC_TYPE_DOUBLE: return m.getArgAsDouble(index);
                    case OFXOSC_TYPE_CHAR:   return m.getArgAsChar(index);
                    case OFXOSC_TYPE_STRING: return std::stoll(m.getArgAsString(index));
                    case OFXOSC_TYPE_SYMBOL: return std::stoll(m.getArgAsSymbol(index));
                    case OFXOSC_TYPE_TRUE:   return true;
                    case OFXOSC_TYPE_FALSE:  return false;
                    default: {
                        ofLogWarning("ofxOscMessageEx::OscArgConverer") << "can't convert from " << m.getArgTypeName(index) << " to " << typeid(type).name();
                        return type();
                    }
                }
            };
            
            template <typename type>
            auto as() const
                -> typename std::enable_if<std::is_floating_point<type>::value, type>::type
            {
                switch(m.getArgType(index)) {
                    case OFXOSC_TYPE_INT32:  return m.getArgAsInt32(index);
                    case OFXOSC_TYPE_INT64:  return m.getArgAsInt64(index);
                    case OFXOSC_TYPE_FLOAT:  return m.getArgAsFloat(index);
                    case OFXOSC_TYPE_DOUBLE: return m.getArgAsDouble(index);
                    case OFXOSC_TYPE_CHAR:   return m.getArgAsChar(index);
                    case OFXOSC_TYPE_STRING: return std::stod(m.getArgAsString(index));
                    case OFXOSC_TYPE_SYMBOL: return std::stod(m.getArgAsSymbol(index));
                    case OFXOSC_TYPE_TRUE:   return true;
                    case OFXOSC_TYPE_FALSE:  return false;
                    default: {
                        ofLogWarning("ofxOscMessageEx::OscArgConverer") << "can't convert from " << m.getArgTypeName(index) << " to " << typeid(type).name();
                        return type();
                    }
                }
            };
            
            template <typename type>
            auto as() const
                -> typename std::enable_if<std::is_same<type, std::string>::value, type>::type
            {
                switch(m.getArgType(index)) {
                    case OFXOSC_TYPE_INT32:  return std::to_string(m.getArgAsInt32(index));
                    case OFXOSC_TYPE_INT64:  return std::to_string(m.getArgAsInt64(index));
                    case OFXOSC_TYPE_FLOAT:  return std::to_string(m.getArgAsFloat(index));
                    case OFXOSC_TYPE_DOUBLE: return std::to_string(m.getArgAsDouble(index));
                    case OFXOSC_TYPE_CHAR:   return std::to_string(m.getArgAsChar(index));
                    case OFXOSC_TYPE_STRING: return m.getArgAsString(index);
                    case OFXOSC_TYPE_SYMBOL: return m.getArgAsSymbol(index);
                    case OFXOSC_TYPE_TRUE:   return "true";
                    case OFXOSC_TYPE_FALSE:  return "false";
                    default: {
                        ofLogWarning("ofxOscMessageEx::OscArgConverer") << "can't convert from " << m.getArgTypeName(index) << " to std::string";
                        return {};
                    }
                }
            }
        private:
            const OscMessageEx &m;
            std::size_t index;
        };
        
        using ofxOscMessage::ofxOscMessage;
        using ofxOscMessage::operator=;
        
        OscMessageEx() {};
        OscMessageEx(const ofxOscMessage &m)
        : ofxOscMessage(m) {};
        
        OscArgConverter operator[](std::size_t index) const {
            return {*this, index};
        }
        
        struct detail {
            template <typename type>
            struct is_convertible_to_int32_t
            : std::integral_constant<
                bool,
                std::is_integral<type>::value
                && sizeof(type) <= 4 && !std::is_same<type, std::uint32_t>::value
            > {};
            
            template <typename type>
            struct is_convertible_to_int64_t
            : std::integral_constant<
                bool,
                std::is_integral<type>::value
                && !is_convertible_to_int32_t<type>::value
            > {};
        };
        
        OscMessageEx &add(float v) {
            addFloatArg(v);
            return *this;
        }
        
        OscMessageEx &add(double v) {
            addDoubleArg(v);
            return *this;
        }
        
        OscMessageEx &add(const std::string &v) {
            addStringArg(v);
            return *this;
        }
        
        template <typename type>
        auto add(const type &v)
            -> typename std::enable_if<
                detail::is_convertible_to_int32_t<type>::value,
                OscMessageEx &
            >::type
        {
            addInt32Arg(v);
            return *this;
        }
        
        template <typename type>
        auto add(const type &v)
            -> typename std::enable_if<
                detail::is_convertible_to_int64_t<type>::value,
                OscMessageEx &
            >::type
        {
            addInt64Arg(v);
            return *this;
        }
        
        template <typename type>
        OscMessageEx &operator<<(const type &v)
        { return add(v); }
    };
};

using ofxOscMessageEx = ofx::OscMessageEx;

#endif /* ofxOscMessageEx_h */
