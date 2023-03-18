//
// BLACK - Bounded Ltl sAtisfiability ChecKer
//
// (C) 2019 Nicola Gigante
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

#include <catch.hpp>

#include <black/support.hpp>

#include <variant>
#include <string>

using namespace black::support;

class test 
{
public:
  using case1 = int;
  using case2 = std::tuple<std::string, float>;

  test(int v) : _data{case1{v}} { }
  test(std::string s, float f) : _data{case2{s, f}} { }

  template<typename T>
  std::optional<T> to() const {
    if(std::holds_alternative<T>(_data))
      return std::get<T>(_data);
    return {};
  }

  template<typename T>
  bool is() const {
    return to<T>().has_value();
  }

  template<typename ...Handlers>
  auto match(Handlers ...h) {
    return matcher<test, std::tuple<case1, case2>>::match(*this, h...);
  }

private:
  std::variant<case1, case2> _data;
};

TEST_CASE("Match infrastructure") {

  test t = 21;

  auto b = t.match(
    [](int x) {
      return x * 2;
    },
    [](test::case2, std::string, float) {
      return false;
    }
  );

  STATIC_REQUIRE(std::is_same_v<decltype(b), int>);

  REQUIRE(b == 42);

}
