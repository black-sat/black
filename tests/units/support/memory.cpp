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

#include <catch.hpp>

#include <black/support>

class Base
{
public:
    Base() = default;
    Base(Base const&) = default;
    Base(Base &&) = default;
    
    virtual ~Base() = default;
    
    Base &operator=(Base const&) = default;
    Base &operator=(Base &&) = default;

    bool operator==(Base const&) const = default;

    virtual int run(int x) const = 0;

    virtual void set_factor(int k) = 0;

};

class Derived : public Base 
{
public:
    Derived(int k) : _k{k} { }

    Derived(Derived const&) = default;
    Derived(Derived &&) = default;

    virtual ~Derived() override = default;
    
    Derived &operator=(Derived const&) = default;
    Derived &operator=(Derived &&) = default;

    bool operator==(Derived const& other) const = default;

    virtual int run(int x) const override { return x * _k; }

    virtual void set_factor(int k) override { _k = k; }

private:
    int _k = 0;
};

class OtherDerived : public Base 
{
public:
    OtherDerived(int k, int c) : _k{k}, _c{c} { }

    OtherDerived(OtherDerived const&) = default;
    OtherDerived(OtherDerived &&) = default;

    virtual ~OtherDerived() override = default;
    
    OtherDerived &operator=(OtherDerived const&) = default;
    OtherDerived &operator=(OtherDerived &&) = default;

    bool operator==(OtherDerived const& other) const = default;

    virtual int run(int x) const override { return x * _k + _c; }

    virtual void set_factor(int k) override { _k = k; }
    
    virtual void set_offset(int c) { _c = c; }

protected:
    int _k;
    int _c;
};

class MostDerived : public OtherDerived 
{
public:
    MostDerived(int k, int c) : OtherDerived(k, c) { }

    MostDerived(MostDerived const&) = default;
    MostDerived(MostDerived &&) = default;

    virtual ~MostDerived() override = default;
    
    MostDerived &operator=(MostDerived const&) = default;
    MostDerived &operator=(MostDerived &&) = default;

    bool operator==(MostDerived const& other) const = default;

    virtual int run(int x) const override { return x * _k + _c + 1; }
};

TEST_CASE("erased<T>") {

    using black::support::erased;

    erased<Base> b1 = Derived{2};

    erased<Base> b2 = b1;

    REQUIRE(b1 == b2);

    REQUIRE(b1->run(21) == 42);
    REQUIRE(b2->run(21) == 42);

    b2->set_factor(3);

    REQUIRE(b1 != b2);

    REQUIRE(b1->run(21) == 42);
    REQUIRE(b2->run(21) == 63);

    b2 = OtherDerived{2, 3};

    REQUIRE(b1 != b2);
    REQUIRE(b2->run(21) == 45);

    erased<OtherDerived> d1 = MostDerived{2, 3};

    REQUIRE(d1->run(21) == 46);
    
    d1->set_offset(4);
    REQUIRE(d1->run(21) == 47);

    erased<Base> b3 = MostDerived{2, 4};
    b1 = d1;

    REQUIRE(b1 == b3);
    REQUIRE(b1->run(21) == 47);
    REQUIRE(b3->run(21) == 47);

    erased<Base const> b4 = Derived{2};
    // correctly does not compile: b4->set_factor(4);

    erased<Base> b5 = b4;

    REQUIRE(b5->run(21) == 42);

}