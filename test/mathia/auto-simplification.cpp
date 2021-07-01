#include "mathia.h"
#include "gtest/gtest.h"
#include <iostream>
using namespace mathia;

TEST(Simplification, power)
{
    auto const x = symbol("x");
    auto const y = symbol("y");
    auto const n2 = constant(2);
    auto const n3 = constant(3);
    auto const e = (x*y)*((x*y)^n2);
    EXPECT_EQ(toString(e), "(* (^ x 3) (^ y 3))");
}

TEST(Simplification, quotient)
{
    auto const a = symbol("a");
    auto const x = symbol("x");
    auto const n2 = constant(2);
    auto const n3 = constant(3);
    auto const e = (a * (x ^ n3)) / x;
    EXPECT_EQ(toString(e), "(* a (^ x 2))");
}

TEST(Simplification, distributive)
{
    auto const x = symbol("x");
    auto const n2 = constant(2);
    auto const n3 = constant(3);
    auto const e1 = x + n2 * x;
    auto const e2 = n3 * x;
    EXPECT_EQ(toString(e1), "(* 3 x)");
    EXPECT_EQ(toString(e2), "(* 3 x)");
    EXPECT_EQ(e1, e2);
}

TEST(asCoeffAndRest, 1)
{
    using namespace matchit;
    Id<std::shared_ptr<mathia::impl::Expr>> coeff;
    auto result = match(*(symbol("x")))
    (
        pattern | mathia::impl::asCoeffAndRest(coeff, _) = [&] { return (*coeff) == constant(1); },
        pattern | _ = expr(false)
    );
    EXPECT_TRUE(result);
}

TEST(asCoeffAndRest, 2)
{
    auto const e = symbol("x")^constant(2);

    using namespace matchit;
    Id<std::shared_ptr<mathia::impl::Expr>> coeff;
    auto result = match(*e)
    (
        pattern | mathia::impl::asCoeffAndRest(coeff, _) = [&] { return (*coeff) == constant(1); },
        pattern | _ = expr(false)
    );
    EXPECT_TRUE(result);
}

TEST(asCoeffAndRest, 3)
{
    using namespace matchit;
    Id<std::shared_ptr<mathia::impl::Expr>> coeff;
    auto result = match(*(constant(5) * symbol("x")))
    (
        pattern | mathia::impl::asCoeffAndRest(coeff, _) = [&] { return (*coeff) == constant(5); },
        pattern | _ = expr(false)
    );
    EXPECT_TRUE(result);
}

TEST(asCoeffAndRest, 4)
{
    using namespace matchit;
    Id<std::shared_ptr<mathia::impl::Expr>> rest;
    auto result = match(*(constant(5) * symbol("x")))
    (
        pattern | mathia::impl::asCoeffAndRest(_, rest) = [&] { return (*rest) == symbol("x"); },
        pattern | _ = expr(false)
    );
    EXPECT_TRUE(result);
}

TEST(asCoeffAndRest, multiple)
{
    using namespace matchit;
    Id<std::shared_ptr<mathia::impl::Expr>> coeff1, coeff2, rest;
    auto result = match(*(constant(5) * symbol("x")), *symbol("x"))
    (
        pattern | ds(mathia::impl::asCoeffAndRest(coeff1, rest), mathia::impl::asCoeffAndRest(coeff2, rest)) = [&] { return (*rest) == symbol("x"); },
        pattern | _ = expr(false)
    );
    EXPECT_TRUE(result);
}

TEST(asBaseAndExp, 1)
{
    using namespace matchit;
    Id<std::shared_ptr<mathia::impl::Expr>> base;
    Id<std::shared_ptr<mathia::impl::Expr>> exp;

    auto result = match(*symbol("x"))
    (
        pattern | mathia::impl::asBaseAndExp(base, exp) = [&] { return (*base) == symbol("x") && (*exp) == constant(1); },
        pattern | _ = expr(false)
    );
    EXPECT_TRUE(result);
}

TEST(asBaseAndExp, 2)
{
    using namespace matchit;
    Id<std::shared_ptr<mathia::impl::Expr>> base;
    Id<std::shared_ptr<mathia::impl::Expr>> exp;

    auto result = match(*(symbol("x")^constant(2)))
    (
        pattern | mathia::impl::asBaseAndExp(base, exp) = [&] { return (*base) == symbol("x") && (*exp) == constant(2); },
        pattern | _ = expr(false)
    );
    EXPECT_TRUE(result);
}

TEST(Simplification, mul2Pow)
{
    auto const x = symbol("x");
    EXPECT_EQ(toString(x * x), "(^ x 2)");
    EXPECT_EQ(toString(x * x * x), "(^ x 3)");
}

TEST(Simplification, distributive2_1)
{
    auto const n2 = constant(2);
    auto const x = symbol("x");
    auto const n3 = constant(3);
    auto const e = n2 * x + n3 * x;
    EXPECT_EQ(toString(e), "(* 5 x)");
}

TEST(Simplification, distributive2_2)
{
    auto const n2 = constant(2);
    auto const x = symbol("x");
    auto const f3o2 = fraction(3, 2);
    auto const e = n2 * x + f3o2 * x;
    EXPECT_EQ(toString(e), "(* 7/2 x)");
}

TEST(simplifyRational, int)
{
    auto const n2 = constant(2);
    auto const n3 = constant(3);

    EXPECT_EQ(toString(n2 + n3), "5");
}

TEST(simplifyRational, rational_)
{
    auto const f7o2 = fraction(14, 4);

    EXPECT_EQ(toString(mathia::impl::simplifyRational(f7o2)), "7/2");
}

TEST(simplifyRational, rational)
{
    auto const n2 = constant(2);
    auto const f3o2 = fraction(3, 2);

    EXPECT_EQ(toString(n2 + f3o2), "7/2");
}

TEST(simplifyRational, rational2)
{
    auto const f3o2 = fraction(3, 2);

    EXPECT_EQ(toString(f3o2 + f3o2), "(* 3/2 2)");
}


TEST(asCoeffAndRest, rational)
{
    auto const n2 = constant(2);
    auto const x = symbol("x");
    auto const f3o2 = fraction(3, 2);

    using namespace matchit;
    Id<std::shared_ptr<mathia::impl::Expr>> coeff1, coeff2, rest;
    auto result = match(*(n2 * x), *(f3o2 * x))
    (
        pattern | ds(mathia::impl::asCoeffAndRest(coeff1, rest), mathia::impl::asCoeffAndRest(coeff2, rest))= [&]
        {
            ((*coeff1) + (*coeff2)); // * (*rest);
            return (*coeff1) == n2 && (*coeff2) == f3o2 && (*rest) == symbol("x"); },
        pattern | _ = expr(false)
    );
    EXPECT_TRUE(result);
}


TEST(Simplification, distributive2)
{
    auto const n2 = constant(2);
    auto const x = symbol("x");
    auto const y = symbol("y");
    auto const f3o2 = fraction(3, 2);
    auto const e1 = n2 * x;
    auto const e = n2 * x + y + f3o2 * x;
    EXPECT_EQ(toString(e), "(+ (* 7/2 x) y)");
}

TEST(asCoeffAndRest, product)
{
    auto const x = symbol("x");
    auto const f3o2 = fraction(3, 2);

    using namespace matchit;
    Id<std::shared_ptr<mathia::impl::Expr>> coeff, rest;
    auto result = match(*(f3o2 * x))
    (
        pattern | mathia::impl::asCoeffAndRest(coeff, rest) = [&] { return (*coeff) == f3o2 && (*rest) == symbol("x"); },
        pattern | _ = expr(false)
    );
    EXPECT_TRUE(result);
}

TEST(asCoeffAndRest, product2)
{
    auto const x = symbol("x");
    auto const f3o2 = fraction(3, 2);

    using namespace matchit;
    auto result = match(*(f3o2 * x))
    (
        pattern | as<mathia::impl::Product>(ds(ds(f3o2, f3o2), ds(x, x))) = [&] { return true; },
        pattern | _ = expr(false)
    );
    EXPECT_TRUE(result);
}

TEST(asCoeffAndRest, sum)
{
    auto const x = symbol("x");
    auto const f3o2 = fraction(3, 2);
    auto const y = symbol("y");

    using namespace matchit;
    Id<std::shared_ptr<mathia::impl::Expr>> e;
    auto result = match(*(y + f3o2 * x))(
        pattern | as<mathia::impl::Sum>(
                      ds(
                          ds(
                              _,
                              some(
                                  as<mathia::impl::Product>(ds(ds(f3o2, f3o2), ds(x, x))))),
                          ds(y, y))) = [&]
        { return true; },
        pattern | _ = expr(false));
    EXPECT_TRUE(result);
}

TEST(asCoeffAndRest, sumCompare)
{
    auto const n2 = constant(2);
    auto const x = symbol("x");
    auto const y = symbol("y");

    auto const e = n2 * x + y;
    auto const sum = std::get<mathia::impl::Sum>(*e);

    EXPECT_NE(sum.find(x), sum.end());
}


TEST(asCoeffAndRest, sum2)
{
    auto const n2 = constant(2);
    auto const x = symbol("x");
    auto const y = symbol("y");

    auto const e = n2 * x + y;

    EXPECT_EQ(toString(e), "(+ (* 2 x) y)");

    using namespace matchit;
    Id<std::shared_ptr<mathia::impl::Expr>> ie;
    auto result = match(*(e))(
        pattern | as<mathia::impl::Sum>(
                      ds(
                          ds(
                              ie,
                              some(
                                  as<mathia::impl::Product>(ds(ds(n2, n2), ds(x, x))))),
                          ds(y, y))) = [&]
        {
            EXPECT_EQ(toString(*ie), "x");
            return true;
        },
        pattern | _ = expr(false));
    EXPECT_TRUE(result);
}

TEST(asCoeffAndRest, sum3)
{
    auto const n2 = constant(2);
    auto const x = symbol("x");
    auto const f3o2 = fraction(3, 2);
    auto const y = symbol("y");

    auto const e = n2 * x + y + f3o2 * x;

    EXPECT_EQ(toString(e), "(+ (* 7/2 x) y)");

    using namespace matchit;
    Id<std::shared_ptr<mathia::impl::Expr>> ie;
    auto result = match(*(e))(
        pattern | as<mathia::impl::Sum>(
                      ds(
                          ds(
                              ie,
                              some(
                                  as<mathia::impl::Product>(ds(ds(fraction(7, 2), fraction(7, 2)), ds(x, x))))),
                          ds(y, y))) = [&]
        {
            EXPECT_EQ(toString(*ie), "x");
            return true;
        },
        pattern | _ = expr(false));
    EXPECT_TRUE(result);
}
