//
//  ofxPubSubOscTypeTraits.h
//
//  Created by ISHII 2bit on 2016/05/29.
//
//

#pragma once

#ifndef ofxPubSubOscTypeTraits_h
#define ofxPubSubOscTypeTraits_h

#include "ofxPubSubOscTypeUtils.h"
#include "ofxOscArrayPublisher.h"
#include "ofxPubSubOscSettings.h"

#include "ofColor.h"

#include "ofVectorMath.h"
#include "ofRectangle.h"
#include "ofFileUtils.h"

#include <array>

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
            struct size_sum;
            
            template <typename Arg>
            struct size_sum<Arg>;
        };

        template <typename Res, typename ... Args>
        struct type_traits<std::function<Res(Args ...)>> {
            using inner_type = std::tuple<Args ...>;
            static constexpr std::size_t size = PubSubOsc::detail::size_sum<Args ...>::value;
            static constexpr bool has_array_operator = false;
        };
        
        template <typename T, typename U>
        struct type_traits<std::pair<T, U>> {
            using inner_type = std::pair<T, U>;
            static constexpr std::size_t size = PubSubOsc::detail::size_sum<T, U>::value;
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
        template <glm::length_t N, typename T, glm::qualifier Q>
        struct type_traits<glm::vec<N, T, Q>> {
            using inner_type = T;
            static constexpr std::size_t size = N;
            static constexpr bool has_array_operator = true;
        };
        
        template <glm::length_t M, glm::length_t N, typename T, glm::qualifier Q>
        struct type_traits<glm::mat<M, N, T, Q>> {
            using inner_type = typename glm::mat<M, N, T, Q>::col_type;
            static constexpr std::size_t size = M * N;
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
        
        template <typename T, std::size_t array_size>
        struct type_traits<std::array<T, array_size>> {
            using inner_type = T;
            static constexpr std::size_t size = type_traits<T>::size * array_size;
            static constexpr bool has_array_operator = true;
        };
        
#pragma mark has std::tuple<...> toOsc() const
        
        template <typename T, typename ... Us>
        struct is_correct_to_osc_types : std::false_type {};
        
        template <typename T, typename ... Us>
        struct is_correct_to_osc_types<std::tuple<Us ...> (T::*)() const> : std::true_type {};

        template <typename T, typename enabler = void>
        struct type_traits_has_to_osc;

        template <typename T, typename = void>
        struct has_to_osc
        : std::false_type {};
        
        template <typename T>
        struct has_to_osc<
            T,
            PubSubOsc::void_t<
                decltype(&T::toOsc),
                PubSubOsc::is_callable<decltype(&T::toOsc)>,
                PubSubOsc::enable_if_t<is_correct_to_osc_types<decltype(&T::toOsc)>::value>
            >
        >
        : std::true_type {
            using result_type = typename PubSubOsc::function_traits<decltype(&T::toOsc)>::result_type;
        };
        
        template <typename T>
        struct type_traits_has_to_osc<
            T,
            PubSubOsc::enable_if_t<PubSubOsc::has_to_osc<T>::value>
        >
        : PubSubOsc::type_traits<typename PubSubOsc::has_to_osc<T>::result_type> {};
        
        template <typename T>
        struct publisher_type_traits
        : std::conditional<
            PubSubOsc::has_to_osc<T>::value,
            type_traits_has_to_osc<T>,
            type_traits<T>
        >::type {};
        
#define ofxPubSubOscMakePublishable(...) \
        auto toOsc() const -> decltype(std::make_tuple( __VA_ARGS__ )) { return { __VA_ARGS__ }; };

#pragma mark has std::tuple<... &> fromOsc()
        
        template <typename T, typename ... Us>
        struct is_correct_from_osc_types : std::false_type {};
        
        template <typename T, typename ... Us>
        struct is_correct_from_osc_types<std::tuple<Us & ...> (T::*)()> : std::true_type {};

        template <typename T, typename = void>
        struct has_from_osc
        : std::false_type {};
        
        template <typename T>
        struct has_from_osc<
            T,
            PubSubOsc::void_t<
                decltype(&T::fromOsc),
                PubSubOsc::is_callable<decltype(&T::fromOsc)>,
                PubSubOsc::enable_if_t<is_correct_from_osc_types<decltype(&T::fromOsc)>::value>
            >
        >
        : std::true_type {
            using result_type = typename PubSubOsc::function_traits<decltype(&T::toOsc)>::result_type;
        };
        
        template <typename T, typename enabler = void>
        struct type_traits_has_from_osc;

        template <typename T>
        struct type_traits_has_from_osc<
            T,
            PubSubOsc::enable_if_t<PubSubOsc::has_from_osc<T>::value>
        >
        : PubSubOsc::type_traits<typename PubSubOsc::has_from_osc<T>::result_type> {};
        
        template <typename T>
        struct subscriber_type_traits
        : std::conditional<
            PubSubOsc::has_from_osc<T>::value,
            type_traits_has_from_osc<T>,
            type_traits<T>
        >::type {};
        
        template <typename T>
        struct type_traits<T, enable_if_t<has_to_osc<T>::value>> {
            using inner_type = T;
            static constexpr std::size_t size = type_traits<typename has_to_osc<T>::result_type>::size;
            static constexpr bool has_array_operator = false;
        };

        template <typename T>
        struct type_traits<T, enable_if_t<!has_to_osc<T>::value && has_from_osc<T>::value>> {
            using inner_type = T;
            static constexpr std::size_t size = type_traits<typename has_from_osc<T>::result_type>::size;
            static constexpr bool has_array_operator = false;
        };

        template <typename ... Ts>
        auto makeRefTuple(Ts & ... vs)
            -> std::tuple<Ts & ...>
        { return { std::ref(vs) ... }; };
        
#define ofxPubSubOscMakeSubscribable(...) \
        auto fromOsc() -> decltype(ofx::PubSubOsc::makeRefTuple(__VA_ARGS__)) { return { __VA_ARGS__ }; }
        
#define ofxPubSubOscify(...) \
        ofxPubSubOscMakePublishable(__VA_ARGS__); \
        ofxPubSubOscMakeSubscribable(__VA_ARGS__);
        
        namespace detail {
            template <typename Arg, typename ... Args>
            struct size_sum {
                static constexpr std::size_t value = size_sum<Arg>::value + size_sum<Args ...>::value;
            };
            
            template <typename Arg>
            struct size_sum<Arg> {
                static constexpr std::size_t value = type_traits<
                    typename std::remove_reference<Arg>::type
                >::size;
            };
        };
    };
};

#endif /* ofxPubSubOscTypeTraits_h */
