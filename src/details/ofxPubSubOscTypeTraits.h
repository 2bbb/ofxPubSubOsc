//
//  ofxPubSubOscTypeTraits.h
//
//  Created by ISHII 2bit on 2016/05/29.
//
//

#pragma once

#ifndef ofxPubSubOscTypeTraits_h
#define ofxPubSubOscTypeTraits_h

#include <array>

#include "ofVectorMath.h"
#include "ofRectangle.h"
#include "ofFileUtils.h"

#include "ofxPubSubOscSettings.h"
#include "ofxOscArrayPublisher.h"

namespace ofx {
    using ofxOscMessageEx = class OscMessageEx;
    
    namespace PubSubOsc {
        template <typename T, typename enabler = void>
        struct type_traits {
            using inner_type = T;
            static constexpr std::size_t size = 1;
            static constexpr bool has_array_operator = false;
        };
        
        template <>
        struct type_traits<void> {
            using inner_type = void;
            static constexpr std::size_t size = 0;
            static constexpr bool has_array_operator = false;
        };
        
        template <>
        struct type_traits<ofxOscMessage> {
            using inner_type = ofxOscMessage;
            static constexpr std::size_t size = 0;
            static constexpr bool has_array_operator = false;
        };
        
        template <>
        struct type_traits<ofxOscMessageEx> {
            using inner_type = ofxOscMessageEx;
            static constexpr std::size_t size = 0;
            static constexpr bool has_array_operator = false;
        };
        
        namespace detail {
            template <typename Arg, typename ... Args>
            struct size_sum {
                static constexpr std::size_t value = type_traits<Arg>::size + size_sum<Args ...>::value;
            };
            
            template <typename Arg>
            struct size_sum<Arg> {
                static constexpr std::size_t value = type_traits<Arg>::size;
            };
        };
        
        template <typename Res, typename ... Args>
        struct type_traits<std::function<Res(Args ...)>> {
            using inner_type = std::tuple<Args ...>;
            static constexpr std::size_t size = PubSubOsc::detail::size_sum<Args ...>::value;
            static constexpr bool has_array_operator = false;
        };
        
        template <typename ... Ts>
        struct type_traits<std::tuple<Ts ...>> {
            using inner_type = std::tuple<Ts ...>;
            static constexpr std::size_t size = PubSubOsc::detail::size_sum<Ts ...>::value;
            static constexpr bool has_array_operator = false;
        };
        
        template <typename T>
        struct type_traits<ofColor_<T> > {
            using inner_type = T;
            static constexpr std::size_t size = 4;
            static constexpr bool has_array_operator = true;
        };
        
        template <>
        struct type_traits<ofVec2f> {
            using inner_type = float;
            static constexpr std::size_t size = 2;
            static constexpr bool has_array_operator = true;
        };
        
        template <>
        struct type_traits<ofVec3f> {
            using inner_type = float;
            static constexpr std::size_t size = 3;
            static constexpr bool has_array_operator = true;
        };
        
        template <>
        struct type_traits<ofVec4f> {
            using inner_type = float;
            static constexpr std::size_t size = 4;
            static constexpr bool has_array_operator = true;
        };
        
        template <>
        struct type_traits<ofQuaternion> {
            using inner_type = float;
            static constexpr std::size_t size = 4;
            static constexpr bool has_array_operator = true;
        };
        
        template <>
        struct type_traits<ofMatrix3x3> {
            using inner_type = float;
            static constexpr std::size_t size = 9;
            static constexpr bool has_array_operator = false; // because don't has "float operator[](int n) const"
        };
        
        template <>
        struct type_traits<ofMatrix4x4> {
            using inner_type = float;
            static constexpr std::size_t size = 16;
            static constexpr bool has_array_operator = false;
        };
        
#pragma mark glm
#ifdef GLM_VERSION
#   if GLM_VERSION < 990
        template <typename glm_vec_t>
        struct type_traits<glm_vec_t, enable_if_t<is_glm_vec<glm_vec_t>::value>> {
            using inner_type = typename glm_vec_t::value_type;
            static constexpr std::size_t size = get_glm_vec_size<glm_vec_t>::value;
            static constexpr bool has_array_operator = true;
        };
        
        template<typename glm_mat_t>
        struct type_traits<glm_mat_t, enable_if_t<is_glm_mat<glm_mat_t>::value>> {
            using inner_type = typename glm_mat_t::col_type;
            static constexpr std::size_t size = get_glm_vec_size<typename glm_mat_t::row_type>::value;
            static constexpr bool has_array_operator = true;
        };
#   else
        template <std::size_t N, typename T, glm::qualifier Q>
        struct type_traits<glm::vec<N, T, Q>> {
            using inner_type = T;
            static constexpr std::size_t size = N;
            static constexpr bool has_array_operator = true;
        };
        
        template <std::size_t M, std::size_t N, typename T, glm::qualifier Q>
        struct type_traits<glm::mat<M, N, T, Q>> {
            using inner_type = typename glm::mat<M, N, T, Q>::col_type;
            static constexpr std::size_t size = M;
            static constexpr bool has_array_operator = true;
        };
#   endif
        template <typename T, glm::precision P>
        struct type_traits<glm::tquat<T, P>> {
            using inner_type = T;
            static constexpr std::size_t size = 4;
            static constexpr bool has_array_operator = true;
        };
#endif

        template <>
        struct type_traits<ofRectangle> {
            using inner_type = float;
            static constexpr std::size_t size = 4;
            static constexpr bool has_array_operator = false;
        };
        
        template <typename T, std::size_t array_size>
        struct type_traits<T[array_size]> {
            using inner_type = T;
            static constexpr std::size_t size = type_traits<T>::size * array_size;
            static constexpr bool has_array_operator = true;
        };
        
        template <typename T, std::size_t array_size>
        struct type_traits<ArrayPublisher<T, array_size> > {
            using inner_type = typename ArrayPublisher<T, array_size>::inner_type;
            static constexpr std::size_t size = type_traits<T>::size * array_size;
            static constexpr bool has_array_operator = true;
        };
        
        template <typename T, std::size_t array_size>
        struct type_traits<ArrayBuffer<T, array_size> > {
            using inner_type = typename ArrayBuffer<T, array_size>::inner_type;
            static constexpr std::size_t size = type_traits<T>::size * array_size;
            static constexpr bool has_array_operator = true;
        };
    };
};

#endif /* ofxPubSubOscTypeTraits_h */
