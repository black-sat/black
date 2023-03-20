//
// BLACK - Bounded Ltl sAtisfiability ChecKer
//
// (C) 2020 Nicola Gigante
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

#ifndef BLACK_SUPPORT_SERIALIZE_HPP
#define BLACK_SUPPORT_SERIALIZE_HPP

#include <cereal/cereal.hpp>
#include <cereal/archives/json.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/tuple.hpp>
#include <cereal/types/optional.hpp>
#include <cereal/types/variant.hpp>

#include <fstream>

namespace black::support::internal {

  template<typename T>
  concept serializable = requires(T v, cereal::JSONOutputArchive a) {
    requires (
      requires { v.serialize(a); } ||
      requires { serialize(a, v); } ||
      requires { v.load(a); v.save(a); } ||
      requires { load(a, v); save(a, v); }
    );
  };

  template<serializable T>
  [[nodiscard("please check the error status")]]
  result<void, io_error> serialize(T const &obj, std::ostream &stream) {
    cereal::JSONOutputArchive archive{stream};

    auto state = stream.exceptions();
    stream.exceptions(std::ostream::badbit);

    try {
      archive(obj);
      stream.exceptions(state);
      return {};
    } catch(const std::ios_base::failure&) {
      stream.exceptions(state);
      return io_error({}, io_error::writing, errno, "serialization failed");
    }
  }

  template<serializable T>
  [[nodiscard("please check the error status")]]
  result<void, io_error> serialize(T const &obj, std::string const&filename) {

    std::ofstream stream{filename};
    if(stream.bad())
      return 
        io_error(filename, io_error::opening, errno, "serialization failed");

    result<void, io_error> res = serialize(obj, stream);
    if(!res.has_value()) {
      res.error().filename = filename;
    }

    return res;
  }

  template<serializable T>
  [[nodiscard("please check the error status")]]
  result<void, io_error> deserialize(T &obj, std::istream &stream) {
    cereal::JSONInputArchive archive{stream};

    auto state = stream.exceptions();
    stream.exceptions(std::ostream::badbit);

    try {
      archive(obj);
      stream.exceptions(state);
      return {};
    } catch(const std::ios_base::failure&) {
      stream.exceptions(state);
      return io_error({}, io_error::reading, errno, "deserialization failed");
    }
  }

  template<serializable T>
  [[nodiscard("please check the error status")]]
  result<void, io_error> deserialize(T &obj, std::string const&filename) {

    std::ifstream stream{filename};
    if(stream.bad())
      return 
        io_error(filename, io_error::opening, errno, "deserialization failed");

    result<void, io_error> res = deserialize(obj, stream);
    if(!res.has_value()) {
      res.error().filename = filename;
    }

    return res;
  }

}

namespace black::support{
  using internal::serialize;
  using internal::deserialize;
}

#endif // BLACK_SUPPORT_SERIALIZE_HPP
