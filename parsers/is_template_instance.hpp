#ifndef GUARD_DPSG_IS_TEMPLATE_INSTANCE_HPP
#define GUARD_DPSG_IS_TEMPLATE_INSTANCE_HPP

#include <type_traits>

/* template<class T, template<typename...> class C> bool is_template_instance_v;

    A simple meta-predicate that detects whether a type is an instance of a
   given type or not.

    Example:

        #include <string>
        #include <tuple>
        #include <variant>
        using my_tuple = std::tuple<int, const char*, double>;
        using my_variant = std::variant<std::string, char>;

        static_assert(dpsg::is_template_instance_v<my_tuple, std::tuple>);
        static_assert(dpsg::is_template_instance_v<my_variant, std::variant>);
        static_assert(!dpsg::is_template_instance_v<my_tuple, std::variant>);
        static_assert(!dpsg::is_template_instance_v<my_variant, std::tuple>);
        static_assert(dpsg::is_template_instance_v<std::string,
                                                   std::basic_string>);

    If you have access to C++20, a concept named template_instance_of is
   provided. e.g.

        template<class S>
        requires dpsg::template_instance_of<S, std::basic_string>
        void do_something_generic(S&& str) { ... }

    Or
        void do_something_generic(dpsg::template_instance_of<std::basic_string>
                                  auto&& str) {...}

    Note that the concept automatically decays the input type, while the
   predicate does not.
*/

namespace dpsg {

template <class Instance, template <typename...> class Template>
struct is_template_instance : std::false_type {};
template <template <typename...> class C, typename... Args>
struct is_template_instance<C<Args...>, C> : std::true_type {};

template <class Instance, template <typename...> class Template>
constexpr static inline bool is_template_instance_v =
    is_template_instance<Instance, Template>::value;

#if defined(__cpp_concepts)
template <class Instance, template <typename...> class Template>
concept template_instance_of =
    is_template_instance_v<std::decay_t<Instance>, Template>;
#endif

}  // namespace dpsg

#endif  // GUARD_DPSG_IS_TEMPLATE_INSTANCE_HPP