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

#ifndef BLACK_IO_DOC_HPP
#define BLACK_IO_DOC_HPP

#include <memory>
#include <string_view>
#include <ostream>
#include <source_location>

namespace black::io {

  namespace internal {
    struct _doc;
  }

  class doc 
  {
  public:
    static constexpr size_t max_fmt_args = 10;

    doc();
    doc(doc const&);
    doc(doc &&);
    ~doc();
    
    doc &operator=(doc const&);
    doc &operator=(doc &&);

    //doc(std::string_view str);

    doc(
      std::string_view fmt, std::vector<doc> args, 
      std::source_location loc = std::source_location::current()
    );

    template<typename ...Args>
      requires (
        sizeof...(Args) < max_fmt_args &&
        (std::is_constructible_v<doc, Args> && ...)
      )
    doc(std::string_view fmt, Args ...args)
      : doc(fmt, {doc(std::move(args))...}) { }

    std::ostream &format(std::ostream &str) const;

    std::string format() const;

  private:
    std::shared_ptr<internal::_doc const> _doc;
  };

  inline std::ostream &operator<<(std::ostream &str, doc const& d) {
    return d.format(str);
  }

}

template<> 
struct std::formatter<black::io::doc> : std::formatter<string_view>
{
  template <typename FormatContext>
  auto format(black::io::doc const&d, FormatContext& ctx) const 
  {
    return formatter<string_view>::format(d.format(), ctx);
  }
};

#endif // BLACK_IO_DOC_HPP