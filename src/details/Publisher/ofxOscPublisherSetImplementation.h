//
//  ofxOscPublisherSetImplementation.h
//
//  Created by ISHII 2bit on 2016/05/29.
//
//

#pragma once

#ifndef ofxOscPublisherSetImplementation_h
#define ofxOscPublisherSetImplementation_h

#include "ofxOscMessage.h"

#include "ofxPubSubOscSettings.h"
#include "ofxPubSubOscTypeUtils.h"
#include "ofxPubSubOscTypeTraits.h"

namespace ofx {
    namespace PubSubOsc {
        template <typename T>
        inline auto set(ofxOscMessage &m, T v)
        -> enable_if_t<is_integral_and_lt_64bit<T>::value>
        { m.addIntArg(v); }
        
        template <typename T>
        inline auto set(ofxOscMessage &m, T v)
        -> enable_if_t<is_integral_and_geq_64bit<T>::value>
        { m.addInt64Arg(v); }
        
#define define_set_float(type) inline void set(ofxOscMessage &m, type v) { m.addFloatArg(v); }
        define_set_float(float);
        define_set_float(double);
#undef define_set_float
        inline void set(ofxOscMessage &m, const std::string &v) { m.addStringArg(v); }
        inline void set(ofxOscMessage &m, const ofBuffer &v) { m.addBlobArg(v); };
        
        
        template <std::size_t n, typename T>
        inline void setVec(ofxOscMessage &m, const T &v) {
            for(int i = 0; i < n; i++) { set(m, v[i]); }
        }
        
        template <typename PixType>
        inline void set(ofxOscMessage &m, const ofColor_<PixType> &v) {  setVec<4>(m, v); }
        inline void set(ofxOscMessage &m, const ofVec2f &v) { setVec<2>(m, v); }
        inline void set(ofxOscMessage &m, const ofVec3f &v) { setVec<3>(m, v); }
        inline void set(ofxOscMessage &m, const ofVec4f &v) { setVec<4>(m, v); }
        inline void set(ofxOscMessage &m, const ofQuaternion &v) { setVec<4>(m, v); }
        
        inline void set(ofxOscMessage &m, const ofMatrix3x3 &v) {
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
        
        inline void set(ofxOscMessage &m, const ofMatrix4x4 &v) {
            for(int j = 0; j < 4; j++) for(int i = 0; i < 4; i++) set(m, v(i, j));
        }
        
        inline void set(ofxOscMessage &m, const ofRectangle &v) {
            set(m, v.x);
            set(m, v.y);
            set(m, v.width);
            set(m, v.height);
        }
        
        template <typename U, std::size_t size>
        inline void set(ofxOscMessage &m, const U (&v)[size]) {
            for(int i = 0; i < size; i++) { set(m, v[i]); }
        }
        
        template <typename U>
        inline void set(ofxOscMessage &m, const std::vector<U> &v) {
            for(int i = 0; i < v.size(); i++) { set(m, v[i]); }
        }
        
#pragma mark ofParameter<T> / ofParameterGroup
        
        template <typename U>
        inline void set(ofxOscMessage &m, const ofParameter<U> &p) {
            set(m, p.get());
        }
    };
};

#endif /* ofxOscPublisherSetImplementation_h */
