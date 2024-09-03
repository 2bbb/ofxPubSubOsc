//
//  ofxOscPublisherSetImplementation.h
//
//  Created by ISHII 2bit on 2016/05/29.
//
//

#pragma once

#ifndef ofxOscPublisherSetImplementation_h
#define ofxOscPublisherSetImplementation_h

#include "ofxPubSubOscTypeTraits.h"
#include "ofxPubSubOscTypeUtils.h"
#include "ofxPubSubOscSettings.h"

#include "ofxOscMessage.h"

#include "ofColor.h"
#include "ofParameter.h"
#include "ofParameterGroup.h"
#include "ofUtils.h"

#include <cmath>

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

        inline void set(ofxOscMessage &m, const ofxOscMessage &v) { m = v; }
        inline void set(ofxOscMessage &m, ofxOscMessage &&v) { std::swap(m, v); }

#if defined(OFXPUBSUBOSC_SEND_DOUBLE_AS_DOUBLE) && OFXPUBSUBOSC_SEND_DOUBLE_AS_DOUBLE
        void set(ofxOscMessage &m, float v) { m.addFloatArg(v); }
        void set(ofxOscMessage &m, double v) { m.addDoubleArg(v); }
#else
#   define define_set_float(type) inline void set(ofxOscMessage &m, type v) { m.addFloatArg(v); }
        define_set_float(float);
        define_set_float(double);
#   undef define_set_float
#endif
        inline void set(ofxOscMessage &m, const std::string &v) { m.addStringArg(v); }
        inline void set(ofxOscMessage &m, const ofBuffer &v) { m.addBlobArg(v); };
        
        
        template <std::size_t n, typename T>
        inline void setVec(ofxOscMessage &m, const T &v) {
            for(std::size_t i = 0; i < n; i++) { set(m, v[i]); }
        }
        
        template <typename PixType>
        inline void set(ofxOscMessage &m, const ofColor_<PixType> &v) { setVec<4>(m, v); }
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
            for(std::size_t j = 0; j < 4; j++) for(std::size_t i = 0; i < 4; i++) set(m, v(i, j));
        }
        
#pragma mark glm
#ifdef GLM_VERSION
#   if GLM_VERSION < 990
        template <typename glm_vec_t>
        inline auto set(ofxOscMessage &m, const glm_vec_t &v)
            -> PubSubOsc::enable_if_t<is_glm_vec<glm_vec_t>::value>
        {
            setVec<get_glm_vec_size<glm_vec_t>::value>(m, v);
        }
        
        template <typename glm_mat_t>
        inline auto set(ofxOscMessage &m, const glm_mat_t &v)
            -> PubSubOsc::enable_if_t<is_glm_mat<glm_mat_t>::value>
        {
            constexpr std::size_t row_length = get_glm_vec_size<typename glm_mat_t::row_type>::value;
            constexpr std::size_t col_length = get_glm_vec_size<typename glm_mat_t::col_type>::value;
            for(std::size_t i = 0; i < row_length; i++) setVec<col_length>(m, v[i]);
        }
        
        template <typename T, glm::precision P>
        inline void set(ofxOscMessage &m, const glm::tquat<T, P> &v) {
            setVec<4>(m, v);
        }
#   else
        template <glm::length_t N, typename T, glm::qualifier Q>
        inline void set(ofxOscMessage &m, const glm::vec<N, T, Q> &v) {
            setVec<N>(m, v);
        }
        
        template <glm::length_t M, glm::length_t N, typename T, glm::qualifier Q>
        inline void set(ofxOscMessage &m, const glm::mat<M, N, T, Q> &v) {
            for(std::size_t i = 0; i < M; i++) setVec<N>(m, v[i]);
        }
        
        template <typename T, glm::precision P>
        inline void set(ofxOscMessage &m, const glm::tquat<T, P> &v) {
            setVec<4>(m, v);
        }
#   endif
#endif

        inline void set(ofxOscMessage &m, const ofRectangle &v) {
            set(m, v.x);
            set(m, v.y);
            set(m, v.width);
            set(m, v.height);
        }
        
#pragma mark containerz
        
        template <typename T, typename U>
        inline void set(ofxOscMessage &m, const std::pair<T, U> &v) {
            set(m, v.first);
            set(m, v.second);
        }
        
        namespace details {
            template <std::size_t index, typename ... Ts>
            inline auto set_recursive(ofxOscMessage &m,
                                      const std::tuple<Ts ...> &v)
                -> typename std::enable_if<index == sizeof...(Ts) - 1>::type
            {
                set(m, std::get<index>(v));
            }
            
            template <std::size_t index, typename ... Ts>
            inline auto set_recursive(ofxOscMessage &m,
                                      const std::tuple<Ts ...> &v)
                -> typename std::enable_if<index < sizeof...(Ts) - 1>::type
            {
                set(m, std::get<index>(v));
                set_recursive<index + 1>(m, v);
            }
        }
        
        template <typename ... Ts>
        inline void set(ofxOscMessage &m, const std::tuple<Ts ...> &v) {
            details::set_recursive<0>(m, v);
        }
        
        template <typename U, std::size_t size>
        inline void set(ofxOscMessage &m, const std::array<U, size> &v) {
            for(std::size_t i = 0; i < size; i++) { set(m, v[i]); }
        }
        
        template <typename U, std::size_t size>
        inline auto set(ofxOscMessage &m, const U (&v)[size])
            -> typename std::enable_if<!std::is_same<U, char>::value>::type
        {
            for(std::size_t i = 0; i < size; i++) { set(m, v[i]); }
        }
        
        template <typename U, std::size_t size>
        inline auto set(ofxOscMessage &m, const U (&v)[size])
            -> typename std::enable_if<std::is_same<U, char>::value>::type
        {
            m.addStringArg(v);
        }

        template <typename U, typename Alloc>
        inline void set(ofxOscMessage &m, const std::vector<U, Alloc> &v) {
            for(std::size_t i = 0; i < v.size(); i++) { set(m, v[i]); }
        }
        
        template <typename U, typename Alloc>
        inline void set(ofxOscMessage &m, const std::deque<U, Alloc> &v) {
            for(std::size_t i = 0; i < v.size(); i++) { set(m, v[i]); }
        }
        
        template <typename Func>
        inline auto set(ofxOscMessage &m, Func f)
        -> enable_if_t<is_callable<Func>::value> {
            set(m, f());
        }
        
#pragma mark ofParameter<T> / ofParameterGroup
        
        template <typename U>
        inline void set(ofxOscMessage &m, const ofParameter<U> &p) {
            set(m, p.get());
        }
    };
};

#endif /* ofxOscPublisherSetImplementation_h */
