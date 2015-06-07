//
//  ofxpubsubosc_type_utils.h
//
//  Created by ISHII 2bit on 2015/06/07.
//
//

#pragma once

namespace ofxpubsubosc {
    namespace {
        template <typename T>
        struct remove_const_reference {
            typedef T type;
        };
        
        template <typename T>
        struct remove_const_reference<T &> : remove_const_reference<T> {};
        
        template <typename T>
        struct remove_const_reference<const T> : remove_const_reference<T> {};
        
        template <typename T>
        struct remove_const_reference<const T&> : remove_const_reference<T> {};
        
        template <typename T>
        struct add_reference_if_non_arithmetic {
            typedef T& type;
        };
        
#define define_add_reference_if_non_arithmetic(T) \
template <> \
struct add_reference_if_non_arithmetic<T> { \
typedef T type; \
};
        define_add_reference_if_non_arithmetic(bool);
        define_add_reference_if_non_arithmetic(short);
        define_add_reference_if_non_arithmetic(unsigned short);
        define_add_reference_if_non_arithmetic(int);
        define_add_reference_if_non_arithmetic(unsigned int);
        define_add_reference_if_non_arithmetic(long);
        define_add_reference_if_non_arithmetic(unsigned long);
        define_add_reference_if_non_arithmetic(float);
        define_add_reference_if_non_arithmetic(double);
#undef define_add_reference_if_non_arithmetic
#define TypeRef(T) typename add_reference_if_non_arithmetic<T>::type
        
        template <typename T>
        struct remove_reference {
            typedef T type;
        };
        template <typename T>
        struct remove_reference<T &> {
            typedef T type;
        };
#define RemoveRef(T) typename remove_reference<T>::type
        
        bool operator==(const ofBuffer &x, const ofBuffer &y) {
            return (x.size() == y.size()) && (memcmp(x.getBinaryBuffer(), y.getBinaryBuffer(), x.size()) == 0);
        }
        
        bool operator!=(const ofBuffer &x, const ofBuffer &y) {
            return (x.size() != y.size()) || (memcmp(x.getBinaryBuffer(), y.getBinaryBuffer(), x.size()) != 0);
        }
    };
};

template <typename T>
bool operator==(const ofColor_<T> &x, const ofColor_<T> &y) {
    return x == y;
}

template <typename T>
bool operator!=(const ofColor_<T> &x, const ofColor_<T> &y) {
    return x != y;
}
