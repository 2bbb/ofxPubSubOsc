//
//  ofxpubsubosc_subscribe_value_wrappers.h
//
//  Created by ISHII 2bit on 2015/06/07.
//
//

#pragma once

#include "ofxpubsubosc_type_utils.h"

namespace ofxpubsubosc {
    namespace subscribe {
        template <typename T>
        struct abstract_stream {
            virtual void set(const typename remove_const_reference<T>::type &t) {};
        };
        
        template <typename T>
        struct raw_stream : abstract_stream<T> {
            raw_stream(T &v)
            : v(v) {};
            virtual void set(const typename remove_const_reference<T>::type &t) { this->t = t; };
        private:
            T &v;
        };
        
        template <typename T, typename U>
        struct setter_function_stream : abstract_stream<T> {
            setter_function_stream(U (*setter)(T))
            : setter(setter) {};
            virtual void set(const typename remove_const_reference<T>::type &t) { setter(t); };
        private:
            U (*setter)(T);
        };
        
        template <typename T, typename U, typename C>
        struct setter_method_stream : abstract_stream<T> {
            setter_method_stream(C &obj, U (C::*setter)(T))
            : obj(obj)
            , setter(setter) {};
            virtual void set(const typename remove_const_reference<T>::type &t) { (obj.*setter)(t); };
        private:
            C &obj;
            U (C::*setter)(T);
        };
        
        template <typename T>
        std::shared_ptr<abstract_stream<T> > stream_factory(T &t) {
            return std::shared_ptr<abstract_stream<T> >(new raw_stream<T>(t));
        }
        
        template <typename T, typename U>
        std::shared_ptr<abstract_stream<T> > stream_factory(U (*setter)(T)) {
            return std::shared_ptr<abstract_stream<T> >(new setter_function_stream<T, U>(setter));
        }
        
        template <typename T, typename U, typename C>
        std::shared_ptr<abstract_stream<T> > stream_factory(C &o, U (C::*setter)(T)) {
            return std::shared_ptr<abstract_stream<T> >(new setter_method_stream<T, U, C>(o, setter));
        }
        
        template <typename T, typename U, typename C>
        std::shared_ptr<abstract_stream<T> > stream_factory(C *o, U (C::*setter)(T)) {
            return std::shared_ptr<abstract_stream<T> >(new setter_method_stream<T, U, C>(*o, setter));
        }
    }
};