//
// BLACK - Bounded Ltl sAtisfiability ChecKer
//
// (C) 2022 Nicola Gigante
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

//
// Note: no include guards. This file is designed to be included multiple times.
//
// This file declares the fragment defined by the macro `FRAGMENT`. It is meant
// to be included many times with a different fragment name each time.

#ifndef FRAGMENT
  #error "define the `FRAGMENT` symbol before including this file"
#endif

//
// Classic utility to concatenate two tokens after expansion
//
#define concat_(a, b) a##b
#define concat(a, b) concat_(a,b)

//
// Here we define the fragment with `make_fragment_cpp_t` (see `core.hpp`). The
// `enum_elements_FRAGMENT` macros, defined in `fragments.hpp`, are called
// passing the `append_syntax_element` macro which just appends the
// `syntax_element` to the list. Everything is included in the
// `black::new_api::logic` namespace.
//
#define append_syntax_element(Element) \
  , syntax_element::Element

namespace black::new_api::logic {
  struct FRAGMENT : black::internal::new_api::make_fragment_cpp_t<0
    concat(enum_elements_, FRAGMENT) (append_syntax_element)
  > { };
}

//
// Here we declare the namespace associated to the fragment. We include
// something from the `logic` namespace, and we specialize many things passing
// the fragment at hand to their template parameters.
//
namespace black::new_api::FRAGMENT {

  using black::new_api::logic::alphabet;
  using black::new_api::logic::otherwise;
  
  template<typename Only>
  using only = black::new_api::logic::only<
    Only, black::new_api::logic::FRAGMENT
  >;

  #define declare_hierarchy(Base) \
    using Base = black::new_api::logic::Base< \
      black::new_api::logic::FRAGMENT \
    >;

  #define declare_storage_kind(Base, Storage) \
    using Storage = black::new_api::logic::Storage< \
      black::new_api::logic::FRAGMENT \
    >;
  
  #define declare_leaf_storage_kind(Base, Storage) \
    using Storage = black::new_api::logic::Storage;

  #define declare_hierarchy_element(Base, Storage, Element) \
    using Element = black::new_api::logic::Element< \
      black::new_api::logic::FRAGMENT \
    >;
  
  #define declare_leaf_hierarchy_element(Base, Storage, Element) \
    using Element = black::new_api::logic::Element;

  #include <black/new/internal/formula/hierarchy.hpp>

}

#undef append_syntax_element
#undef concat_
#undef concat
