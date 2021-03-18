# Parsers

This is a parser combinator library for C++17. As of writing it is only tested and functional with Clang 11, but should work fine with older versions with complete support for C++17. Support for VC2019 and GCC is in the work.

It is very much a work in progress, but it can already be used to produce fairly complex parsers with little effort.  
Among the defining features are (WIP, so take this as a mostly true wishlist, there are still some edge cases): 
+ no runtime dependencies: the library can be used mostly at compile time. The only exception is dynamic range parsers that produce some sort of C++ object as a result. This can be avoided with some more trickery if you don't need to keep the intermediate results (for example by folding over the values as you get them).
+ optional exceptions: customizable failure handlers mean you can choose the strategy you like for ill-formed inputs. 
+ completely configurable: on top of combining existing parsers and defining you own, you can change parsing strategies (only 1 available at the moment, but you can implement your own), optimize manually special cases or configure error handling 

## What's a parser anyway? 

Simply put, a parser is a program which takes a list of values as input, "consumes" (read and discard) some of it and produces some sort of output. The main application is text processing: a parser will read part or all of a character string and produce some sort of runtime value from its content. Parser combinators are building blocks for parsers. Each parser combinator is a parser in its own right, returning a result (or failing) as well as the position until which it consumed it's input. This last information can be used to form the input to another parser. The library then does some template magic to combine, and optionally transform, the results in a way that makes sense. 

This library provides a number of basic parsers and combination methods that should cover most needs. 

### What's the difference with regular expressions?

Regular expression are a simple way to express simple parsers. They're convenient for simple use cases but quickly meet situations where something more complex is required. XML is a famous example of something that cannot be properly parsed with regular expression, as there is no way to match opening and closing tags (some extensions may be able to do it but it's outside of what formally constitutes a regular expression and typically devolves into a mess of characters).

In comparison, this library provides a way to feed parts of the input into later computations, producing parsers that essentially self-modify depending on their input. Let's take a trivial example that cannot be done with a regular expression: parsing a character followed by the next character in alphabetic order. For example "ab", "EF" or "wy".
With this library, this can be expressed as: 
~~~ cpp
constexpr auto next_char = [](char c) { return character{c + 1}; }

// Matches two characters in ascending orders
constexpr auto succ1 = bind{ascii::alpha, next_char}; 
// Or with operators
constexpr auto succ2 = ascii::alpha >>= next_char;
~~~
`ascii::alpha` is a parser consuming exactly 1 character if it is a letter in the ASCII range. We define a `next_char` function that returns a `character` parser, a parser consuming exactly the character it was created with. Those two elements are combined with the `bind` combinator that will forward the result of `ascii::alpha` (a single character) to `next_char` and use the result to parse the next character of the input. If it succeeds, it'll return the result of the second parser.

## Basic usage and installation
The library is header-only. Simply copy the content of the __include__ folder in your project to get started.  
You can find many basic examples in the __tests__ folder and more complex examples in the __examples__ folder.

Using the library is fairly straightforward. `#include "parsers/parsers.hpp`, define a parser, call a processing function with your parser and your input.
