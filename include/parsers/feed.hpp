#ifndef GUARD_DPSG_FEED_HPP
#define GUARD_DPSG_FEED_HPP

#include <type_traits>

/* template<class T, template<class...> class C> using feed_t;

    A super simple but super useful meta-function, that extracts the template
   parameters of the first type it receives and instanciate the template given
   in second parameter with them. Example:

        #include <tuple>
        #include <variant>

        using my_tuple = std::tuple<char, int, double>;

        using my_variant = dpsg::feed_t<my_tuple, std::variant>;

        static_assert(std::is_same_v<my_variant,
                                     std::variant<char, int, double>>);

    Using https://gist.github.com/de-passage/cabea442a4cc21fd3f0ce93e3a6ffbf3

        #include <iostream>
        #include <is_template_instance.hpp>

        template <dpsg::template_instance_of<std::tuple> T, std::size_t... Is>
        void print_tuple(T&& t, [[maybe_unused]] std::index_sequence<Is...>) {
            ((std::cout << std::get<Is>(std::forward<T>(t)) << '\n'), ...);
        }

        template <dpsg::template_instance_of<std::tuple> T>
        void print_tuple(T&& tpl) {
            print_tuple(std::forward<T>(tpl),
                        dpsg::feed_t<T, std::index_sequence_for>{});
        }

*/
namespace dpsg {

template <class T, template <class...> class C>
struct feed;
template <template <class...> class T,
          template <class...>
          class C,
          class... Args>
struct feed<T<Args...>, C> {
  using type = C<Args...>;
};
template <class T, template <class...> class C>
using feed_t = typename feed<std::decay_t<T>, C>::type;

}  // namespace dpsg

#endif  // GUARD_DPSG_FEED_HPP