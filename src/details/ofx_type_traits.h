//
//  ofx_type_traits.h
//
//  Created by ISHII 2bit on 2015/05/12.
//
//

#pragma once

#include "ofMain.h"

template <typename T>
struct ofx_type_traits {
    typedef T inner_type;
    static const size_t size = 1;
};

template <typename T>
struct ofx_type_traits<ofColor_<T> > {
    typedef T inner_type;
    static const size_t size = 4;
};

template <>
struct ofx_type_traits<ofVec2f> {
    typedef float inner_type;
    static const size_t size = 2;
};

template <>
struct ofx_type_traits<ofVec3f> {
    typedef float inner_type;
    static const size_t size = 3;
};

template <>
struct ofx_type_traits<ofVec4f> {
    typedef float inner_type;
    static const size_t size = 4;
};

template <>
struct ofx_type_traits<ofQuaternion> {
    typedef float inner_type;
    static const size_t size = 4;
};

template <>
struct ofx_type_traits<ofMatrix3x3> {
    typedef float inner_type;
    static const size_t size = 9;
};

template <>
struct ofx_type_traits<ofRectangle> {
    typedef float inner_type;
    static const size_t size = 4;
};

template <typename T, size_t array_size>
struct ofx_type_traits<T[array_size]> {
    typedef T inner_type;
    static const size_t size = ofx_type_traits<T>::size * array_size;
};
