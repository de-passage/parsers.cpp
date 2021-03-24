# Parsers

This is a parser combinator library for C++17. It contains a lot of bleeding-edge techniques, so as of writing it is only tested and functional with Clang 11, but hopefully works with older versions with complete support for C++17. Compilation fails on VC2019 but I'm working on it. GCC is untested.

It is very much a work in progress, but it can already be used to produce fairly complex parsers with little effort.  
Among the defining features are (WIP, so take this as a mostly true wishlist, there are still some edge cases): 
+ no runtime dependencies: the library can be used mostly at compile time. The only exception is dynamic range parsers that produce some sort of C++ object as a result. This can be avoided with some more trickery if you don't need to keep the intermediate results (for example by folding over the values as you get them).
+ optional exceptions: customizable failure handlers mean you can choose the strategy you like for ill-formed inputs. 
+ completely configurable: on top of combining existing parsers and defining you own, you can change parsing strategies (3 available at the moment, and you can implement your own), optimize manually special cases or configure error handling 

## What's a parser anyway? 

Simply put, a parser is a program which takes a list of values as input, "consumes" (read and discard) some of it and produces some sort of output. The main application is text processing: a parser will read part or all of a character string and produce some sort of runtime value from its content. Different parsers have different requirements for the input, a parser _succeeds_ if the input matches what it expects and _fails_ if it doesn't. A parser that succeded typically returns a value, built from the parsed input, and the subset that wasn't consumed. This makes parsers _composable_, as this last information can be used to form the input to another parser. We can also handle the failure of a parser gracefully, for example by calling another parser with the input. In all those cases, the result of the combination is another parser, taking an input, consuming part of it and returning some sort of output. The object representing the combination of parsers into more complex ones is called a combinator.

This library provides a number of basic parsers and combinators that should cover most needs. It also does some (i.e. a tremendous amount of) template magic to combine, and optionally transform, the results in a way that makes sense. As is the common usage in C++, it works with iterators. Specifically, it means to work with anything that implements _forward iterator_ requirements.

### What's the difference with regular expressions?

Regular expression are a simple way to express simple parsers. They're convenient for simple use cases but quickly meet situations where something more complex is required. XML is a famous example of something that cannot be properly parsed with regular expression, as there is no way to match opening and closing tags (some extensions may be able to do it but it's outside of what formally constitutes a regular expression and typically devolves into a mess of characters).

In comparison, this library provides a way to feed parts of the input into later computations, producing parsers that essentially self-modify depending on their input. Let's take a trivial example that cannot be done with a regular expression: parsing a character followed by the next character in alphabetic order. For example "ab", "EF" or "wy".
With this library, this can be expressed as: 
~~~ cpp
constexpr auto next_char = [](char c) { return character{c + 1}; };

// Matches two characters in ascending orders
constexpr auto succ1 = bind{ascii::alpha, next_char}; 
// Or with operators
constexpr auto succ2 = ascii::alpha >>= next_char;
~~~
`ascii::alpha` is a parser consuming exactly 1 character if it is a letter in the ASCII range. We define a `next_char` function that returns a `character` parser, a parser consuming exactly the character it was created with. Those two elements are combined with the `bind` combinator that will forward the result of `ascii::alpha` (a single character) to `next_char` and use the result to parse the next character of the input. If it succeeds, it'll return the result of the second parser.

As you can infer from the example, parsers are regular C++ objects. You can therefore build arbitrary complex operations over parsers by simply defining functions and intermediary objects. 

## Basic usage and installation
The library is header-only. Simply copy the content of the __include__ folder in your project to get started.  
You can find many basic examples in the __tests__ folder and more complex examples in the __examples__ folder.

Using the library is fairly straightforward. `#include "parsers/parsers.hpp`, define a parser, call a processing function with your parser and your input.

For maximum flexibility, the library separate the consumption of the input, which is expressed by a _description_ and the production of the output, which is defined by the _interpreter_. Interpreters typically return a `parsers::result` result type, a wrapper around a `std::variant` with 2 template parameters that add some semantics to the type. The first template parameter is the _success type_, the second the _failure type_. The success type is typically a product type including the    
If you rely on premade functions however, the selection of the interpreter and post-processing of the result is done for you.

### Utility functions
These functions include: 
###### match
``` cpp
template<class Description, class T>
constexpr bool match(Description&& description, const T& input) noexcept; 
```
Returns true if the given description matches the beginning of input. Unlike with a classic regular expression library, this function will **not** match a subset of the input that starts farther that the 0th index. You can achieve this result by discarding characters until you find the start of your chain. (for example, recursively ` fix(p | (~any & self))`

###### match_full
``` cpp
template <class Descriptor, class T>
constexpr bool match_full(Descriptor&& descriptor, const T& input) noexcept;
```
Similar to `match`, but fails if the input is not completely consumed (i.e. has trailing characters). 

###### parse
~~~ cpp
template <class Description, class T>
constexpr parsers::result</* success type depending on Description */, /*error type WIP*/>
parse(Description&& desc, const T& input);
~~~
Transform the input into a C++ object. Sequences of characters are turned into `parsers::range` (moral equivalent of C++20 `span`), sequences of non characters (produced by `construct` or `build` for example) are combined into `std::tuple`, and alternatives are represented as `std::variant`.  
The final result is a variant wrapper containing either the result or an error type (currently an iterator representing the point of failure, WIP).

### Basic parsers
These are simple parsers that are useful in most circumstances.
###### character
Parses a single character, returning it as is. The dsl provides a `_c` suffix to simplify the code. Note that literal characters are also understood as `character` where possible.
###### static_string
Parses a sequence of characters and returns a `parsers::range`. The dsl provides a `_s` suffix, literal character arrays are also understood as `static_string` where possible. Note that the constructor taking an array will drop the last element as it is usually the terminating 0 that we don't want to include implicitely. In most cases this doesn't cause any problem, but remember to use the iterator based constructor if you ever construct it from an array that doesn't contain a terminating 0.
###### end
Parses the end of the input. This only succeeds if the start iterator is equal to the end iterator.
###### eos
Parses the end of a string. This succeeds if `end` succeeds or if the current character is 0.
###### succeed
A parser that always succeeds without consuming anything. Seems useless but can be exploited to build other parsers. For example the `optional` parser could be defined as:
``` cpp 
constexpr auto optional = [] (auto p) { return p | succeed; };
```
###### fail
A parser that always fail regardless of input. Mostly useful with custom error handlers, that are not implemented yet. So not useful. Yet.

### Combinators

These are not technically parsers as they cannot be used alone, they need to be created from other parsers, they are basically higher-order parsers.  
The 2 most basic combinators are `sequence` and `alternative`, but many more are provided.
#### Essentials
###### sequence
`sequence` is the combination of several parsers one after the other. There is a `both` parser that is the exact equivalent of a `sequence` with exactly two elements.  
`sequence` is represented by the operator `&`.
~~~ cpp
constexpr auto ab = both{character{'a'}, character{'b'}}; // parses exactly "ab", could be written as a sequence
constexpr auto abc = sequence{character{'a'}, character{'b'}, character{'c'}}; // parses exactly "abc"
constexpr auto abc2 = character{'a'} & character{'b'} & character{'c'}; // parses exactly "abc"
~~~
The parse result is a `std::tuple` containing the elements in order, unless it is a sequence of exactly one element in which case it is returned as is.
###### alternative
`alternative` is a choice between several parsers. Inner parsers are typically tried left to right, and the result is the result of the first parser to succeed. Alternative strategies (not implemented) could progress through the inner parsers in parallel and accumulate all the results in some way.  
`either` is equivalent to an alternative with exactly 2 cases.  
`alternative` is represented by the operator `|`
~~~ cpp
constexpr auto choose = either{static_string{"vanilla"}, static_string{"chocolate"}}; // parses "vanilla" or "chocolate"
constexpr auto ab_or_c = alternative{character{'a'}, character{'b'}, character{'c'}}; // parses "a" or "b" or "c"
constexpr auto ab_or_c2 = character{'a'} | character{'b'} | character{'c'}; // parses "a" or "b" or "c"
~~~
The parse result is a `std::variant` parameterized with the same types as the result types of the inner parsers, and containing the successful one. 
###### map
`map` takes the result of a parser and transforms it. It is created from a parser and a transformation function object (class with an overloaded `operator()`, like a lambda).  
`map` is represented by the operator `/`.
``` cpp
constexpr auto digit_as_int = map{ascii::digit, [](char c) { return c - '0'; }; // parsers a character in the range [0-9] and converts it to int
constexpr auto digit_as_int = ascii::digit / [](char c) { return c - '0'; };
```
###### bind
`bind` is the basic mechanism to feed previous results to further parsers. It takes a parser and a function object accepting the result of the first parser and returning a parser.  
`bind` is represented by the operator `>>=`.
``` cpp
// parses 2 consecutive ascending numbers, wrapping from 9 to 0. e.g. "90", "23", "56" but not "13" nor "21"
constexpr auto consecutive = bind{ascii::digit, [](char d) { return d == '9' ? character{'0'} : character{d + 1}; }};
constexpr auto consecutive2 = ascii::digit >>= [](char d) { return d == '9' ? character{'0'} : character{d + 1}; };
```
###### discard
`discard` has for only effect to remove the result of the inner parser from whatever sequence it is contained in. Additionally, it will prevent all instanciation of objects in inner parsers, silencing things like non constexpr construction of `std::vector` by `many` for example.  
`discard` is represented by the operator `~`.
``` cpp
constexpr auto ignore_spaces = discard{many{ascii::space}};
constexpr auto ignore_spaces2 = ~many{ascii::space};
constexpr auto spaces_then_letter = ignore_spaces & any; // match any sequence followed by a character and return only the latter
```
###### fix
`fix` is used to generate recursive parsers. It must be used together with the special value `self` that represents the parser being created.
Some other parsers could be implemented using `fix` but aren't for performance and ergonomy reason. Parsing recursive structures yield a tree of objects linked by `std::unique_ptr`, which can be detrimental to performances and is generally unwieldy.
``` cpp
constexpr auto many = [](auto p) { return fix((p & self) | succeed); };
```
For... reasons, the type level version of `fix` is called `recursive`. See the __examples__ for usage.

#### More combinators
All the combinators in this section could be implemented using the above, but are provided for convenience and performance.
###### optional
Parses 1 or 0 times the inner parser. Could be implemented as `p | succeed`.
###### many
Parses 0 or more times. Could be implemented as `fix((p & self) | succeed)`.
###### many1
Parses 1 or more times. Could be implemented as `p & many{p}`.
###### choose
Behaves similarily to `alternative`, but only in the case where all return types are the same, and unwrap the result. See the __math__ example for good use cases.
###### construct 
Behaves similarily to `sequence`, but feeds the results of the parser to the constructor of the type tagged as the first argument.
###### build
Takes the range consumed by the inner parser and feeds it to a user-defined function object.  
Represented by the operator `/=`.

## Concrete example
The following program reads a single input representing the addition of two integers and returns the result (i.e. "3+5" would write "8"). 
``` cpp 
#include <parsers/parsers.hpp>

#include <iostream>

using namespace parsers::description;
using namespace parsers::dsl;

// turns an arbitrary range (not necessarily ending with 0, as atoi requires)
// into an integer
constexpr auto to_int = [](auto beg, auto end) {
  int count = 0;
  while (beg != end) {
    count = count * 10 + *beg - '0';
    ++beg;
  }
  return count;
};

// parse a sequence of 1 or more digits into a C++ int
constexpr auto integer = many1{ascii::digit} /= to_int;

// parse an integer followed by '+' (discarded), followed by an integer, into a
// std::tuple<int, int>
constexpr auto addition = integer & ~('+'_c) & integer;

int main(int argc, char** argv) {
  if (argc < 2) {
    std::cerr << "Missing input\n";
    return 1;
  }

  auto result = parsers::parse(addition, parsers::range(argv[1]));
  if (result.has_value()) {
    std::cout << std::get<0>(result.value()) + std::get<1>(result.value())
              << '\n';
  }
  else {
    std::cerr << "Invalid input\n";
    return 1;
  }

  return 0;
}
```
