//
//  ofxOscSubscriberLoadImplementation.h
//
//  Created by ISHII 2bit on 2016/05/29.
//
//

#pragma once

#ifndef ofxOscSubscriberLoadImplementation_h
#define ofxOscSubscriberLoadImplementation_h

#include <cmath>
#include <algorithm>

#include "ofxOscMessage.h"

#include "ofxPubSubOscSettings.h"
#include "ofxPubSubOscTypeUtils.h"
#include "ofxPubSubOscTypeTraits.h"

namespace ofx {
    namespace PubSubOsc {
#define define_set_arithmetic(type) \
        inline void load(const ofxOscMessage &m, type &v, std::size_t offset = 0) { \
            if(m.getArgType(offset) == OFXOSC_TYPE_INT32) v = m.getArgAsInt32(offset); \
            else if(m.getArgType(offset) == OFXOSC_TYPE_INT64) v = m.getArgAsInt64(offset); \
            else if(m.getArgType(offset) == OFXOSC_TYPE_FLOAT) v = m.getArgAsFloat(offset); \
            else if(m.getArgType(offset) == OFXOSC_TYPE_DOUBLE) v = m.getArgAsDouble(offset); \
            else if(m.getArgType(offset) == OFXOSC_TYPE_STRING) v = ofToDouble(m.getArgAsString(offset)); \
            else if(m.getArgType(offset) == OFXOSC_TYPE_TRUE) v = true; \
            else if(m.getArgType(offset) == OFXOSC_TYPE_FALSE) v = false; \
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
        
        inline void load(const ofxOscMessage &m, std::string &v, std::size_t offset = 0) {
            if(m.getArgType(offset) == OFXOSC_TYPE_STRING) v = m.getArgAsString(offset);
            else if(m.getArgType(offset) == OFXOSC_TYPE_FLOAT) v = ofToString(m.getArgAsFloat(offset));
            else if(m.getArgType(offset) == OFXOSC_TYPE_DOUBLE) v = ofToString(m.getArgAsDouble(offset));
            else if(m.getArgType(offset) == OFXOSC_TYPE_INT32) v = ofToString(m.getArgAsInt32(offset));
            else if(m.getArgType(offset) == OFXOSC_TYPE_INT64) v = ofToString(m.getArgAsInt64(offset));
            else v = m.getArgAsString(offset);
        }
        
        inline void load(const ofxOscMessage &m, ofBuffer &v, std::size_t offset = 0) {
            v = m.getArgAsBlob(offset);
        }
        
        inline void load(const ofxOscMessage &m, ofxOscMessage &v, std::size_t offset = 0) {
            v = m;
        }

#pragma mark ofColor_
        template <typename U>
        inline void loadColor(const ofxOscMessage &m, ofColor_<U> &v, U defaultValue, std::size_t offset = 0) {
            if(m.getNumArgs() == 1) {
                load(m, v.r, offset);
                load(m, v.g, offset);
                load(m, v.b, offset);
                v.a = defaultValue;
            } else if(m.getNumArgs() == 3) {
                load(m, v.r, offset + 0);
                load(m, v.g, offset + 1);
                load(m, v.b, offset + 2);
                v.a = defaultValue;
            } else {
                load(m, v.r, offset + 0);
                load(m, v.g, offset + 1);
                load(m, v.b, offset + 2);
                load(m, v.a, offset + 3);
            }
        }
        
        inline void load(const ofxOscMessage &m, ofColor &v, std::size_t offset = 0) {
            loadColor<unsigned char>(m, v, 255, offset);
        }
        inline void load(const ofxOscMessage &m, ofShortColor &v, std::size_t offset = 0) {
            loadColor<unsigned short>(m, v, 65535, offset);
        }
        inline void load(const ofxOscMessage &m, ofFloatColor &v, std::size_t offset = 0) {
            loadColor<float>(m, v, 1.0f, offset);
        }
        
#pragma mark oF container type
        template <std::size_t n, typename U>
        inline void loadVec(const ofxOscMessage &m, U &v, std::size_t offset = 0) {
            for(std::size_t i = 0; i < std::min(static_cast<std::size_t>(m.getNumArgs() - offset), n); i++) {
                load(m, v[i], offset + i);
            }
        }
        
        inline void load(const ofxOscMessage &m, ofVec2f &v, std::size_t offset = 0) {
            loadVec<2>(m, v, offset);
        }
        inline void load(const ofxOscMessage &m, ofVec3f &v, std::size_t offset = 0) {
            loadVec<3>(m, v, offset);
        }
        inline void load(const ofxOscMessage &m, ofVec4f &v, std::size_t offset = 0) {
            loadVec<4>(m, v, offset);
        }
        inline void load(const ofxOscMessage &m, ofQuaternion &v, std::size_t offset = 0) {
            loadVec<4>(m, v, offset);
        }
        inline void load(const ofxOscMessage &m, ofMatrix3x3 &v, std::size_t offset = 0) {
            loadVec<9>(m, v, offset);
        }
        
        inline void load(const ofxOscMessage &m, ofMatrix4x4 &v, std::size_t offset = 0) {
            for(std::size_t j = 0; j < 4; j++) for(std::size_t i = 0; i < 4; i++) {
                load(m, v(i, j), offset + 4 * j + i);
            }
        }
        
#pragma mark glm
#ifdef GLM_VERSION
#   if GLM_VERSION < 990
        template <typename glm_vec_t>
        inline auto load(const ofxOscMessage &m, glm_vec_t &v, std::size_t offset = 0)
            -> PubSubOsc::enable_if_t<is_glm_vec<glm_vec_t>::value>
        {
            loadVec<get_glm_vec_size<glm_vec_t>::value>(m, v, offset);
        }
        
        template <typename glm_mat_t>
        inline auto load(const ofxOscMessage &m, glm_mat_t &v, std::size_t offset = 0)
            -> PubSubOsc::enable_if_t<is_glm_mat<glm_mat_t>::value>
        {
            constexpr std::size_t row_length = get_glm_vec_size<typename glm_mat_t::row_type>::value;
            constexpr std::size_t col_length = get_glm_vec_size<typename glm_mat_t::col_type>::value;
            for(std::size_t i = 0; i < row_length; i++) loadVec<col_length>(m, v[i], offset + col_length * i);
        }
        
        template <typename T, glm::precision P>
        inline void load(const ofxOscMessage &m, glm::tquat<T, P> &v, std::size_t offset = 0) {
            loadVec<4>(m, v);
        }
#   else
        template <std::size_t N, typename T, glm::qualifier Q>
        inline void load(const ofxOscMessage &m, glm::vec<N, T, Q> &v, std::size_t offset = 0)
        {
            loadVec<N>(m, v, offset);
        }
        
        template <std::size_t M, std::size_t N, typename T, glm::qualifier Q>
        inline void load(const ofxOscMessage &m, glm::mat<M, N, T, Q> &v, std::size_t offset = 0)
        {
            for(std::size_t i = 0; i < M; i++) loadVec<N>(m, v[i], offset + N * i);
        }
        
        template <typename T, glm::precision P>
        inline void load(const ofxOscMessage &m, glm::tquat<T, P> &v, std::size_t offset = 0) {
            loadVec<4>(m, v);
        }
#   endif
#endif
        
        inline void load(const ofxOscMessage &m, ofRectangle &v, std::size_t offset = 0) {
            load(m, v.x,      offset + 0);
            load(m, v.y,      offset + 1);
            load(m, v.width,  offset + 2);
            load(m, v.height, offset + 3);
        }
        
#pragma mark ofParameter<T> / ofParameterGroup
        
        template <typename U>
        inline void load(const ofxOscMessage &m, ofParameter<U> &p, std::size_t offset = 0) {
            U u;
            load(m, u, offset);
            p.set(u);
        }
        
        inline void load(const ofxOscMessage &m, ofAbstractParameter &p, std::size_t offset = 0) {
#define type_convert(type_) if(p.type() == typeid(ofParameter<type_>).name()) { load(m, p.cast<type_>(), offset); return; }
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
        
        inline void load(const ofxOscMessage &m, ofParameterGroup &pg, std::size_t offset = 0) {
            if(m.getArgType(0) == OFXOSC_TYPE_INT32) {
                if(pg.size() <= m.getArgAsInt32(0)) {
                    ofLogWarning("ofxOscSubscriber") << "ofAbstractParameterGroup: not contain index \"" << m.getArgAsInt32(0) << "\"";
                    return;
                }
                load(m, pg.get(m.getArgAsInt32(0)), offset + 1);
            } else if(m.getArgType(0) == OFXOSC_TYPE_INT64) {
                if(pg.size() <= m.getArgAsInt64(0)) {
                    ofLogWarning("ofxOscSubscriber") << "ofAbstractParameterGroup: not contain index \"" << m.getArgAsInt64(0) << "\"";
                    return;
                }
                load(m, pg.get(m.getArgAsInt64(0)), offset + 1);
            } else if(m.getArgType(0) == OFXOSC_TYPE_STRING) {
                if(!pg.contains(m.getArgAsString(0))) {
                    ofLogWarning("ofxOscSubscriber") << "ofAbstractParameterGroup: not contain key \"" << m.getArgAsString(0) << "\"";
                    return;
                }
                load(m, pg.get(m.getArgAsString(0)), offset + 1);
            }
        }
        
#pragma mark container
        template <typename U, std::size_t size>
        inline void load(const ofxOscMessage &m, std::array<U, size> &v, std::size_t offset = 0) {
            for(std::size_t i = 0; i < std::min(size, (m.getNumArgs() - offset) / type_traits<U>::size); i++) {
                load(m, v[i], offset + i * type_traits<U>::size);
            }
        }
        
        template <typename U, std::size_t size>
        inline void load(const ofxOscMessage &m, U (&v)[size], std::size_t offset = 0) {
            for(std::size_t i = 0; i < std::min(size, (m.getNumArgs() - offset) / type_traits<U>::size); i++) {
                load(m, v[i], offset + i * type_traits<U>::size);
            }
        }
        
        template <typename U, typename Alloc>
        inline void load(const ofxOscMessage &m, std::vector<U, Alloc> &v, std::size_t offset = 0) {
            std::size_t num = (m.getNumArgs() - offset) / type_traits<U>::size;
            if(v.size() != num) v.resize(num);
            for(std::size_t i = 0; i < num; i++) {
                load(m, v[i], offset + i * type_traits<U>::size);
            }
        }
        
        template <typename U, typename Alloc>
        inline void load(const ofxOscMessage &m, std::deque<U, Alloc> &v, std::size_t offset = 0) {
            std::size_t num = (m.getNumArgs() - offset) / type_traits<U>::size;
            if(v.size() != num) v.resize(num);
            for(std::size_t i = 0; i < num; i++) {
                load(m, v[i], offset + i * type_traits<U>::size);
            }
        }
    };
};

#endif /* ofxOscSubscriberLoadImplementation_h */
