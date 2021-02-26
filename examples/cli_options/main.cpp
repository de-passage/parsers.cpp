#include <iomanip>
#include <iostream>
#include <type_traits>
#include <utility>

#include "./description.hpp"

namespace cli_options {
using namespace parsers::description;
using namespace parsers::dsl;

struct word : satisfy_character<word> {
  constexpr auto operator()(
      [[maybe_unused]] const char* element) const noexcept {
    return true;
  }
} constexpr word;

struct match_option : satisfy_character<match_option> {
  template <class C>
  constexpr bool operator()(const C* r) const noexcept {
    return parsers::match(any_option{}, parsers::range{r});
  }
};

struct parse_option : modifier<match_option,
                               parsers::interpreters::make_parser_t<
                                   parsers::interpreters::object_parser>> {
  template <class I>
  using result_t =
      parsers::interpreter_value_type<parsers::interpreters::object_parser,
                                      I,
                                      any_option>;

  template <class C>
  result_t<const C*> operator()(const C* elem) const {
    auto range = parsers::range{elem};
    return parsers::parse(any_option{}, range).value();
  }

} constexpr test_options;

constexpr auto test =
    many{test_options | construct{parsers::type<std::string>, word}};

}  // namespace cli_options

int main(int argc, const char** argv) {
  using namespace parsers::interpreters;
  const auto parse = make_parser<object_parser>(cli_options::test);
  auto p = parse(argv, argv + argc);
  std::cout << "parsing succeded? " << std::boolalpha << p.has_value()
            << std::endl;
  auto r = std::get<1>(std::move(p).value());
  std::cout << "result size: " << r.size() << std::endl;
  for (std::size_t i = 0; i < r.size(); ++i) {
    std::cout << "n" << i << " ";
    if (r[i].index() == 1) {
      std::cout << "value : " << std::get<1>(r[i]) << std::endl;
    }
    else {
      auto& o = std::get<0>(r[i]);
      std::cout << "option name: " << o.name;
      if (o.value) {
        std::cout << "\tvalue: " << *o.value << std::endl;
      }
      else {
        std::cout << "\t no value" << std::endl;
      }
    }
  }
  return 0;
}