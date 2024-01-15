//
// BLACK - Bounded Ltl sAtisfiability ChecKer
//
// (C) 2024 Nicola Gigante
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

#include <black/support>
#include <black/io>

#include <sstream>

namespace black::io {

  template<size_t N, typename T, typename F>
  static auto apply_vec(std::vector<T> const& v, F f) {
    if constexpr(N == 0)
      return f();
    else
      return [&]<size_t ...Is>(std::index_sequence<Is...>) {
        if(v.size() < N)
          return apply_vec<N-1>(v, f);
        return f(v[Is]...);
      }(std::make_index_sequence<N>{});
  }

  namespace internal {
    struct _doc {

      _doc() = default;
      virtual ~_doc() = default;

      virtual std::ostream &format(std::ostream &str) const = 0;
    };
    
    struct simple;
    struct formatted;
  }

  doc::doc() = default;
  doc::doc(doc const&) = default;
  doc::doc(doc &&) = default;
  doc::~doc() = default;
  
  doc &doc::operator=(doc const&) = default;
  doc &doc::operator=(doc &&) = default;

  std::ostream &doc::format(std::ostream &str) const {
    if(_doc)
      return _doc->format(str);
    return str;
  }

  std::string doc::format() const {
    if(!_doc)
      return "";
    std::stringstream str;
    _doc->format(str);
    return str.str();
  }

  //
  // Document formatted with `std::format` specs
  //
  struct internal::formatted : _doc {
    formatted(
      std::string_view fmt, std::vector<doc> args, std::source_location loc
    )
      : fmt{fmt}, args{std::move(args)}, loc{loc} { 
        black_assume(
          this->args.size() < doc::max_fmt_args, loc, 
          "Maximum number of arguments for formatted document is {}, given {}", 
          doc::max_fmt_args, this->args.size()
        );
      }

    std::string fmt;
    std::vector<doc> args;
    std::source_location loc;

    std::ostream &format(std::ostream &str) const override {
      apply_vec<doc::max_fmt_args>(args, [&](auto const& ...docs) {
        auto out = std::ostream_iterator<char>(str);
        std::vformat_to(out, fmt, std::make_format_args(docs...));
      });
      return str;
    }
  };
  
  doc::doc(
    std::string_view fmt, std::vector<doc> args, std::source_location loc
  )
    : _doc{std::make_shared<internal::formatted>(fmt, args, loc)} { }
  

}