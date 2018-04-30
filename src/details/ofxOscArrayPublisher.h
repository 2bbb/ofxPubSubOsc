//
//  ofxOscArrayPublisher.h
//  ofxPubSubOscDev
//
//  Created by ISHII 2bit on 2016/05/29.
//
//

#pragma once

#ifndef ofxOscArrayPublisher_h
#define ofxOscArrayPublisher_h

#include <array>

namespace ofx {
    namespace PubSubOsc {
        namespace detail {
            template <typename T>
            struct abstract_stream {
                virtual T get() { return T(); };
            };
            
            template <typename T>
            struct raw_stream : abstract_stream<T> {
                raw_stream(T &v)
                : v(v) {};
                virtual T get() { return v; };
            private:
                T &v;
            };
            
            template <typename T>
            struct getter_function_stream : abstract_stream<T> {
                getter_function_stream(T (*getter)())
                : getter(getter) {};
                virtual T get() { return getter(); };
            private:
                T (*getter)();
            };
            
            template <typename T, typename U>
            struct getter_method_stream : abstract_stream<T> {
                getter_method_stream(U &obj, T (U::*getter)())
                : obj(obj)
                , getter(getter) {};
                
                virtual T get() { return (obj.*getter)(); };
            private:
                U &obj;
                T (U::*getter)();
            };
            
            template <typename T, typename U>
            struct const_getter_method_stream : abstract_stream<T> {
                const_getter_method_stream(const U &obj, T (U::*getter)() const)
                : obj(obj)
                , getter(getter) {};
                
                virtual T get() { return (obj.*getter)(); };
            private:
                const U &obj;
                T (U::*getter)() const;
            };
            
            
#pragma mark pointer stream
            
            template <typename T>
            struct abstract_pointer_stream {
                virtual T *get() { return NULL; };
                T operator [](std::size_t n) { return get()[n]; };
            };
            
            template <typename T>
            struct raw_pointer_stream : abstract_pointer_stream<T> {
                raw_pointer_stream(T *ptr)
                : ptr(ptr) {};
                
                virtual T *get() { return ptr; };
            private:
                T *ptr;
            };
            
            template <typename T>
            struct getter_function_pointer_stream : abstract_pointer_stream<T> {
                getter_function_pointer_stream(T (*getter)())
                : getter(getter) {};
                
                virtual T *get() { return getter(); };
            private:
                T (*getter)();
            };
            
            template <typename T, typename U>
            struct getter_method_pointer_stream : abstract_pointer_stream<T> {
                getter_method_pointer_stream(U &obj, T (U::*getter)())
                : obj(obj)
                , getter(getter) {};
                
                virtual T *get() { return (obj.*getter)(); };
            private:
                U &obj;
                T (U::*getter)();
            };
            
            template <typename T, typename U>
            struct const_getter_method_pointer_stream : abstract_pointer_stream<T> {
                const_getter_method_pointer_stream(const U &obj, T (U::*getter)() const)
                : obj(obj)
                , getter(getter) {};
                
                virtual T *get() { return (obj.*getter)(); };
            private:
                const U &obj;
                T (U::*getter)() const;
            };
            
#pragma mark factory
            
            template <typename T>
            std::shared_ptr<abstract_pointer_stream<T> > pointer_stream_factory(T *t) {
                return std::shared_ptr<abstract_pointer_stream<T> >(new raw_pointer_stream<T>(t));
            }
            
            template <typename T>
            std::shared_ptr<abstract_pointer_stream<T> > pointer_stream_factory(T (*g)()) {
                return std::shared_ptr<abstract_pointer_stream<T> >(new getter_function_pointer_stream<T>(g));
            }
            
            template <typename T, typename U>
            std::shared_ptr<abstract_pointer_stream<T> > pointer_stream_factory(U &that, T (U::*g)()) {
                return std::shared_ptr<abstract_pointer_stream<T> >(new getter_method_pointer_stream<T, U>(that, g));
            }
            
            template <typename T, typename U>
            std::shared_ptr<abstract_pointer_stream<T> > pointer_stream_factory(const U &that, T (U::*g)() const) {
                return std::shared_ptr<abstract_pointer_stream<T> >(new const_getter_method_pointer_stream<T, U>(that, g));
            }
        };
        
        template <typename T, std::size_t s>
        struct ArrayPublisher {
            using inner_type = const T;
            using const_array_t = T const (&)[s];
            
            ArrayPublisher(T *v)
            : stream(PubSubOsc::detail::pointer_stream_factory(v)) {}
            
            ArrayPublisher(T (*getter)())
            : stream(PubSubOsc::detail::pointer_stream_factory(getter)) {}
            template <typename U>
            
            ArrayPublisher(U &that, T (U::*getter)())
            : stream(PubSubOsc::detail::pointer_stream_factory(that, getter)) {}
            template <typename U>
            
            ArrayPublisher(const U &that, T (U::*getter)() const)
            : stream(PubSubOsc::detail::pointer_stream_factory(that, getter)) {}
            virtual ~ArrayPublisher() { }
            
            T operator[](std::size_t n) const { return stream[n]; };
            
            std::size_t size() const { return s; };
            operator const_array_t() { return get(); };
            const_array_t get() { return reinterpret_cast<const_array_t>(stream.get()); };
        protected:
            std::shared_ptr<PubSubOsc::detail::abstract_pointer_stream<T> > stream;
        };
        
        template <typename T, std::size_t s>
        struct ArrayPublisher<const T, s> : ArrayPublisher<T, s> {};
        
        template <typename T, std::size_t s>
        struct ArrayBuffer {
            using inner_type = T;
            using array_t = T (&)[s];
            
            ArrayBuffer() : v(malloc(sizeof(T) * s)) {}
            virtual ~ArrayBuffer() { free(v); v = NULL; }
            
            array_t operator=(const ArrayPublisher<T, s> &arr) { for(std::size_t i = 0; i < size(); i++) v[i] = arr[i]; return get(); };
            bool operator!=(const ArrayPublisher<T, s> &arr) const { for(std::size_t i = 0; i < size(); i++) if(v[i] != arr[i]) return true; return false; };
            bool operator==(const ArrayPublisher<T, s> &arr) const { for(std::size_t i = 0; i < size(); i++) if(v[i] != arr[i]) return false; return true; };
            T operator[](std::size_t n) const { return v[n]; };
            T &operator[](std::size_t n) { return v[n]; };
            
            std::size_t size() const { return s; };
            operator array_t() { return get(); };
            array_t get() { return reinterpret_cast<array_t>(reinterpret_cast<T &>(v[0])); };
        protected:
            T *v;
        };
        
        template <typename T, std::size_t s>
        struct ArrayBuffer<const T, s> : ArrayBuffer<T, s> {};

    };
};

#endif /* ofxOscArrayPublisher_h */
