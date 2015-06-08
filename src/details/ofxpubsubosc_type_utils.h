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
                
    };
};

#if (OF_VERSION_MAJOR == 0) && (OF_VERSION_MINOR < 9) /* ofColor will provid operator==(const ofColor &c) const from ver. 0.9.0 */

template <typename T>
bool operator==(const ofColor_<T> &x, const ofColor_<T> &y) {
    return x == y;
}

template <typename T>
bool operator!=(const ofColor_<T> &x, const ofColor_<T> &y) {
    return x != y;
}

#endif

bool operator==(const ofMatrix3x3 &x, const ofMatrix3x3 &y) {
    return (x.a == y.a) && (x.b == y.b) && (x.c == y.c)
        && (x.d == y.d) && (x.e == y.e) && (x.f == y.f)
        && (x.g == y.g) && (x.h == y.h) && (x.i == y.i);
}

bool operator!=(const ofMatrix3x3 &x, const ofMatrix3x3 &y) {
    return !operator==(x, y);
}

bool operator==(const ofMatrix4x4 &x, const ofMatrix4x4 &y) {
    return (x._mat[0][0] == y._mat[0][0]) && (x._mat[0][1] == y._mat[0][1]) && (x._mat[0][2] == y._mat[0][2]) && (x._mat[0][3] == y._mat[0][3])
        && (x._mat[1][0] == y._mat[1][0]) && (x._mat[1][1] == y._mat[1][1]) && (x._mat[1][2] == y._mat[1][2]) && (x._mat[1][3] == y._mat[1][3])
        && (x._mat[2][0] == y._mat[2][0]) && (x._mat[2][1] == y._mat[2][1]) && (x._mat[2][2] == y._mat[2][2]) && (x._mat[2][3] == y._mat[2][3])
        && (x._mat[3][0] == y._mat[3][0]) && (x._mat[3][1] == y._mat[3][1]) && (x._mat[3][2] == y._mat[3][2]) && (x._mat[3][3] == y._mat[3][3]);
}

bool operator!=(const ofMatrix4x4 &x, const ofMatrix4x4 &y) {
    return !operator==(x, y);
}

bool operator==(const ofBuffer &x, const ofBuffer &y) {
    return (x.size() == y.size()) && (memcmp(x.getBinaryBuffer(), y.getBinaryBuffer(), x.size()) == 0);
}

bool operator!=(const ofBuffer &x, const ofBuffer &y) {
    return !operator==(x, y);
}
