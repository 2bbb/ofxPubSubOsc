//
//  ofxpubsubosc_type_traits.h
//
//  Created by ISHII 2bit on 2015/05/12.
//
//

#pragma once

#include "ofMain.h"

namespace ofxpubsubosc {
    template <typename T>
    struct type_traits {
        typedef T inner_type;
        static const size_t size = 1;
        static const bool has_array_operator = false;
    };

    template <typename T>
    struct type_traits<ofColor_<T> > {
        typedef T inner_type;
        static const size_t size = 4;
        static const bool has_array_operator = true;
    };

    template <>
    struct type_traits<ofVec2f> {
        typedef float inner_type;
        static const size_t size = 2;
        static const bool has_array_operator = true;
    };

    template <>
    struct type_traits<ofVec3f> {
        typedef float inner_type;
        static const size_t size = 3;
        static const bool has_array_operator = true;
    };

    template <>
    struct type_traits<ofVec4f> {
        typedef float inner_type;
        static const size_t size = 4;
        static const bool has_array_operator = true;
    };

    template <>
    struct type_traits<ofQuaternion> {
        typedef float inner_type;
        static const size_t size = 4;
        static const bool has_array_operator = true;
    };

    template <>
    struct type_traits<ofMatrix3x3> {
        typedef float inner_type;
        static const size_t size = 9;
        static const bool has_array_operator = false; // because don't has "float operator[](int n) const"
    };

    template <>
    struct type_traits<ofMatrix4x4> {
        typedef float inner_type;
        static const size_t size = 16;
        static const bool has_array_operator = false;
    };

    template <>
    struct type_traits<ofRectangle> {
        typedef float inner_type;
        static const size_t size = 4;
        static const bool has_array_operator = false;
    };

    template <typename T, size_t array_size>
    struct type_traits<T[array_size]> {
        typedef T inner_type;
        static const size_t size = ofxpubsubosc::type_traits<T>::size * array_size;
        static const bool has_array_operator = false;
    };
};