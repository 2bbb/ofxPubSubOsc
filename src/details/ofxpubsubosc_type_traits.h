//
//  ofxpubsubosc_type_traits.h
//
//  Created by ISHII 2bit on 2015/05/12.
//
//

#pragma once

#include "ofMain.h"
#include "ofxpubsubosc_subscribe_value_wrappers.h"
#include "ofxpubsubosc_publish_value_wrappers.h"

namespace ofxpubsubosc {
    template <typename T>
    struct type_traits {
        using inner_type = T;
        static const size_t size = 1;
        static const bool has_array_operator = false;
    };

    template <typename T>
    struct type_traits<ofColor_<T> > {
        using inner_type = T;
        static const size_t size = 4;
        static const bool has_array_operator = true;
    };

    template <>
    struct type_traits<ofVec2f> {
        using inner_type = float;
        static const size_t size = 2;
        static const bool has_array_operator = true;
    };

    template <>
    struct type_traits<ofVec3f> {
        using inner_type = float;
        static const size_t size = 3;
        static const bool has_array_operator = true;
    };

    template <>
    struct type_traits<ofVec4f> {
        using inner_type = float;
        static const size_t size = 4;
        static const bool has_array_operator = true;
    };

    template <>
    struct type_traits<ofQuaternion> {
        using inner_type = float;
        static const size_t size = 4;
        static const bool has_array_operator = true;
    };

    template <>
    struct type_traits<ofMatrix3x3> {
        using inner_type = float;
        static const size_t size = 9;
        static const bool has_array_operator = false; // because don't has "float operator[](int n) const"
    };

    template <>
    struct type_traits<ofMatrix4x4> {
        using inner_type = float;
        static const size_t size = 16;
        static const bool has_array_operator = false;
    };

    template <>
    struct type_traits<ofRectangle> {
        using inner_type = float;
        static const size_t size = 4;
        static const bool has_array_operator = false;
    };

    template <typename T, size_t array_size>
    struct type_traits<T[array_size]> {
        using inner_type = T;
        static const size_t size = ofxpubsubosc::type_traits<T>::size * array_size;
        static const bool has_array_operator = true;
    };
    
    template <typename T, size_t array_size>
    struct type_traits<publish::array_publisher<T, array_size> > {
        using inner_type = typename publish::array_publisher<T, array_size>::inner_type;
        static const size_t size = ofxpubsubosc::type_traits<T>::size * array_size;
        static const bool has_array_operator = true;
    };
    
    template <typename T, size_t array_size>
    struct type_traits<publish::array_buffer<T, array_size> > {
        using inner_type = typename publish::array_buffer<T, array_size>::inner_type;
        static const size_t size = ofxpubsubosc::type_traits<T>::size * array_size;
        static const bool has_array_operator = true;
    };
};