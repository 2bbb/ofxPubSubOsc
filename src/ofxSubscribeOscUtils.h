//
//  ofxSubscribeOscUtils.h
//
//  Created by 2bit on 2021/04/15.
//

#ifndef ofxSubscribeOscUtils_h
#define ofxSubscribeOscUtils_h

#include "ofxOscSubscriber.h"

#include <type_traits>
#include <functional>

namespace ofx {
    namespace PubSubOsc {
        namespace SubscribeOscUtils {
            template <typename value_t, typename min_t, typename max_t>
            inline auto clamper(value_t &v, min_t min, max_t max)
                -> enable_if_t<
                    !is_callable<value_t>::value,
                    std::function<void(value_t)>
                >
            {
                return [&v, min, max] (value_t w) {
                    v = std::min<value_t>(std::max<value_t>(w, min), max);
                };
            };
            
            template <typename callback_t, typename min_t, typename max_t>
            inline auto clamper(callback_t callback, min_t min, max_t max)
                -> enable_if_t<
                    is_callable<callback_t>::value,
                    typename function_traits<callback_t>::function_type
                >
            {
                using value_t = typename function_traits<callback_t>::template argument_type<0>;
                return [callback, min, max] (value_t w) {
                    callback(std::min<value_t>(std::max<value_t>(w, min), max));
                };
            };

            template <typename value_t, typename min_t>
            inline auto over(value_t &v, min_t lower_value)
                -> enable_if_t<
                    !is_callable<value_t>::value,
                    std::function<void(value_t)>
                >
            {
                return [&v, lower_value] (value_t w) {
                    v = std::max<value_t>(w, lower_value);
                };
            };
            
            template <typename callback_t, typename min_t>
            inline auto over(callback_t callback, min_t min)
                -> enable_if_t<
                    is_callable<callback_t>::value,
                    typename function_traits<callback_t>::function_type
                >
            {
                using value_t = typename function_traits<callback_t>::template argument_type<0>;
                return [callback, min] (value_t w) {
                    callback(std::max<value_t>(w, min));
                };
            };

            template <typename value_t, typename max_t>
            inline auto under(value_t &v, max_t higher_value)
                -> enable_if_t<
                    !is_callable<value_t>::value,
                    std::function<void(value_t)>
                >
            {
                return [&v, higher_value] (value_t w) {
                    v = std::min<value_t>(w, higher_value);
                };
            };
            
            template <typename callback_t, typename max_t>
            inline auto under(callback_t callback, max_t max)
                -> enable_if_t<
                    is_callable<callback_t>::value,
                    typename function_traits<callback_t>::function_type
                >
            {
                using value_t = typename function_traits<callback_t>::template argument_type<0>;
                return [callback, max] (value_t w) {
                    callback(std::min<value_t>(w, max));
                };
            };
            
            template <typename value_t, typename container_t>
            inline auto contains(value_t &v,
                                 const container_t &set)
                -> enable_if_t<
                    !is_callable<value_t>::value && is_kind_of_containers<container_t>::value,
                    std::function<void(const value_t &)>
                >
            {
                return [&v, set] (const value_t &w) {
                    if(std::find(set.cbegin(), set.cend(), w) != set.end()) v = w;
                };
            };
            
            template <typename value_t, typename container_value_t>
            inline auto contains(value_t &v,
                                 const std::initializer_list<container_value_t> &init_list)
                -> enable_if_t<
                    !is_callable<value_t>::value,
                    std::function<void(const value_t &)>
                >
            {
                std::vector<container_value_t> set{init_list};
                return [&v, set] (const value_t &w) {
                    if(std::find(set.cbegin(), set.cend(), w) != set.end()) v = w;
                };
            };

            template <
                typename callback_t,
                typename container_t,
                typename value_t = typename function_traits<callback_t>::template argument_type<0>
            >
            inline auto contains(callback_t callback,
                                 const container_t &set)
                -> enable_if_t<
                    is_callable<callback_t>::value && is_kind_of_containers<container_t>::value,
                    typename function_traits<callback_t>::function_type
                >
            {
                return [callback, set] (value_t w) {
                    if(std::find(set.cbegin(), set.cend(), w) != set.end()) callback(w);
                };
            };
            
            template <
                typename callback_t,
                typename container_value_t,
                typename value_t = typename function_traits<callback_t>::template argument_type<0>
            >
            inline auto contains(callback_t callback,
                                 const std::initializer_list<container_value_t> &init_list)
                -> enable_if_t<
                    is_callable<callback_t>::value,
                    typename function_traits<callback_t>::function_type
                >
            {
                std::vector<container_value_t> set{init_list};
                return [callback, set] (value_t w) {
                    if(std::find(set.cbegin(), set.cend(), w) != set.end()) callback(w);
                };
            };
        }; // namespace FunctionUtils
        
        struct SubscribeOscHelper {
            SubscribeOscHelper(std::uint16_t port,
                               const std::string &path = "")
            { setup(port, path); }
            
            void setup(std::uint16_t port,
                       const std::string &path = "")
            {
                this->port = port;
                this->path = path;
            }
            
            template <typename ... arguments>
            SubscribeOscHelper &subscribe(const std::string &address,
                                          arguments && ... args)
            {
                ofxSubscribeOsc(port,
                                path + address,
                                std::forward<arguments>(args) ...);
                return *this;
            }
            
            /**
             adress: string
             v: reference of number type
             min: minimum value of clamp
             max: maximum value of clamp
             */
            template <typename number_type, typename min_t, typename max_t>
            auto between(const std::string &address,
                         number_type &v,
                         min_t min,
                         max_t max)
                -> enable_if_t<
                    !is_callable<number_type>::value,
                    SubscribeOscHelper &
                >
            { return subscribe(address, SubscribeOscUtils::clamper(v, min, max)); }

            template <typename callback_t, typename min_t, typename max_t>
            auto between(const std::string &address,
                         callback_t callback,
                         min_t min,
                         max_t max)
                -> enable_if_t<
                    is_callable<callback_t>::value,
                    SubscribeOscHelper &
                >
            { return subscribe(address, SubscribeOscUtils::clamper(callback, min, max)); }

            template <typename number_type, typename min_t>
            auto over(const std::string &address,
                      number_type &v,
                      min_t min)
                -> enable_if_t<
                    !is_callable<number_type>::value,
                    SubscribeOscHelper &
                >
            { return subscribe(address, SubscribeOscUtils::over(v, min)); }

            template <typename callback_t, typename min_t>
            auto over(const std::string &address,
                      callback_t callback,
                      min_t min)
                -> enable_if_t<
                    is_callable<callback_t>::value,
                    SubscribeOscHelper &
                >
            { return subscribe(address, SubscribeOscUtils::over(callback, min)); }

            template <typename number_type, typename max_t>
            auto under(const std::string &address,
                       number_type &v,
                       max_t max)
                -> enable_if_t<
                    !is_callable<number_type>::value,
                    SubscribeOscHelper &
                >
            { return subscribe(address, SubscribeOscUtils::under(v, max)); }

            template <typename callback_t, typename max_t>
            auto under(const std::string &address,
                       callback_t callback,
                       max_t max)
                -> enable_if_t<
                    is_callable<callback_t>::value,
                    SubscribeOscHelper &
                >
            { return subscribe(address, SubscribeOscUtils::under(callback, max)); }
            
            template <typename value_type, typename container_t>
            auto contains(const std::string &address,
                          value_type &v,
                          const container_t &set)
                -> enable_if_t<
                    !is_callable<value_type>::value && is_kind_of_containers<container_t>::value,
                    SubscribeOscHelper &
                >
            { return subscribe(address, SubscribeOscUtils::contains(v, set)); }
            
            template <typename value_type, typename container_value_t>
            auto contains(const std::string &address,
                          value_type &v,
                          const std::initializer_list<container_value_t> &set)
                -> enable_if_t<
                    !is_callable<value_type>::value,
                    SubscribeOscHelper &
                >
            { return subscribe(address, SubscribeOscUtils::contains(v, set)); }

            template <
                typename callback_t,
                typename container_t,
                typename value_t = typename function_traits<callback_t>::template argument_type<0>
            >
            auto contains(const std::string &address,
                          callback_t callback,
                          const container_t &set)
                -> enable_if_t<
                    is_callable<callback_t>::value && is_kind_of_containers<container_t>::value,
                    SubscribeOscHelper &
                >
            { return subscribe(address, SubscribeOscUtils::contains(callback, set)); }

            template <
                typename callback_t,
                typename container_value_t,
                typename value_t = typename function_traits<callback_t>::template argument_type<0>
            >
            auto contains(const std::string &address,
                          callback_t callback,
                          const std::initializer_list<container_value_t> &set)
                -> enable_if_t<
                    is_callable<callback_t>::value,
                    SubscribeOscHelper &
                >
            { return subscribe(address, SubscribeOscUtils::contains(callback, set)); }

            SubscribeOscHelper createChild(const std::string &childPath)
            { return { this, port, path + childPath }; }
            
            SubscribeOscHelper &pop() {
                if(parent) return *parent;
                ofLogWarning() << "unbalanced call of pop from '" << path << "'. returning '*this'";
                return *this;
            };
            
        protected:
            SubscribeOscHelper(SubscribeOscHelper *parent,
                            std::uint16_t port,
                            const std::string &path)
            : parent{parent}
            { setup(port, path); }
            std::uint16_t port;
            std::string path;
            SubscribeOscHelper *parent{nullptr};
        }; // struct SubscribeOscHelper
    }; // namespace PubSubOsc
}; // namespace ofx

namespace ofxSubscribeOscUtils = ofx::PubSubOsc::SubscribeOscUtils;
using ofxSubscribeOscHelper = ofx::PubSubOsc::SubscribeOscHelper;

#endif /* ofxPubSubOscFunctionUtils_h */
