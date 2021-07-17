#include "matchit.h"
#include "mathiu/core.h"
#include "mathiu/solve.h"
#include "mathiu/inequation.h"
#include "mathiu/function_range.h"

namespace mathiu::impl
{
    // implement limit

    Interval functionRangeImplIntervalDomain(ExprPtr const &function, ExprPtr const &symbol, ExprPtr const &domain)
    {
#if DEBUG
        std::cout << "functionRangeImplIntervalDomain: " << toString(function) << ",\t" << toString(symbol) << ",\t" << toString(domain) << std::endl;
#endif // DEBUG

        auto const domain_ = std::get<Interval>(*domain);
        auto const subs = [&](ExprPtr const &dst)
        { return substitute(function, symbol >> dst); };
        auto const firstP = IntervalEnd{subs(domain_.first.first), domain_.first.second};
        auto const secondP = IntervalEnd{subs(domain_.second.first), domain_.second.second};
        auto const derivative = diff(function, symbol);
        auto const criticalPoints = solve(derivative, symbol, domain);
        auto min_ = evald(firstP.first) < evald(secondP.first) ? firstP : secondP;
        auto max_ = evald(firstP.first) < evald(secondP.first) ? secondP : firstP;
        // optimize me
        for (auto const &e : std::get<Set>(*criticalPoints))
        {
            auto const v = IntervalEnd{subs(e), true};
            min_ = evald(min_.first) < evald(v.first) ? min_ : v;
            max_ = evald(max_.first) > evald(v.first) ? max_ : v;
        }
        assert(evald(min_.first) <= evald(max_.first));
        auto result = Interval{min_, max_};
#if DEBUG
        std::cout << "functionRangeImplIntervalDomain: " << toString(function) << ",\t" << toString(symbol) << ",\t" << toString(domain) << ",\t resutl: " << toString(std::make_shared<Expr const>(result)) << std::endl;
#endif // DEBUG

        return result;
    }

    ExprPtr unionInterval(Interval const &lhs, Interval const &rhs)
    {
            assert(lhs.first.first != nullptr);
            assert(rhs.second.first != nullptr);

#if DEBUG
            std::cout << "unionInterval: " << toString(std::make_shared<Expr const>(lhs)) << ",\t" << toString(std::make_shared<Expr const>(rhs)) << std::endl;
#endif // DEBUG

            Id<IntervalEnd> iIEL1, iIER1, iIEL2, iIER2;
            return match(lhs, rhs)(
                pattern | ds(realInterval(iIEL1, iIER1), realInterval(iIEL2, iIER2)) = [&]
                {
                    auto dL1 = evald((*iIEL1).first);
                    auto dL2 = evald((*iIEL2).first);
                    auto dR1 = evald((*iIER1).first);
                    auto dR2 = evald((*iIER2).first);
                    auto const left = dL1 <= dL2 ? *iIEL1 : *iIEL2;
                    auto const right = dR1 >= dR2 ? *iIER1 : *iIER2;
                    if (dL1 <= dR2 && dL2 <= dR1)
                    {
                        return std::make_shared<Expr const>(Interval{left, right});
                    }
                    return std::make_shared<Expr const>(SetOp{Union{{std::make_shared<Expr const>(lhs), std::make_shared<Expr const>(rhs)}}});
                },
                pattern | _ = [&]
                {
                    throw std::logic_error{"Mismatch!"};
                    return std::make_shared<Expr const>(lhs);
                });
    }

        auto mergeU(Union const& c1, Union const& c2) -> Union
        {
            if (c1.size() < c2.size())
            {
                return mergeU(c2, c1);
            }
            Union result = c1;
            for (auto const& e : c2)
            {
                result.insert(e);
            }
            return result;
        }


    auto mergeUnion(Union const& c1, Union const& c2)
    {
        auto result = mergeU(c1, c2);
        if (result.size() == 1)
        {
            return (*result.begin());
        }
        return std::make_shared<Expr const>(SetOp{std::move(result)});
    }

    ExprPtr union_(ExprPtr const &lhs, ExprPtr const &rhs)
    {
#if DEBUG
        std::cout << "union_: " << toString(lhs) << ",\t" << toString(rhs) << std::endl;
#endif // DEBUG

        Id<Interval> iInterval1, iInterval2, iInterval3;
        Id<Union> iUnion1, iUnion2;
        return match(lhs, rhs)(
            pattern | ds(some(as<Interval>(iInterval1)), some(as<Interval>(iInterval2))) = [&]
            {
                return unionInterval(*iInterval1, *iInterval2);
            },
            pattern | ds(false_, _) = expr(rhs),
            pattern | ds(_, false_) = expr(lhs),
            pattern | ds(some(as<SetOp>(as<Union>(iUnion1))), some(as<SetOp>(as<Union>(iUnion2)))) = [&]
            {
                return mergeUnion(*iUnion1, *iUnion2);
            },
            pattern | ds(some(as<Interval>(iInterval1.at(realInterval(_, _)))),
                        some(as<SetOp>(as<Union>(ds(some(as<Interval>(iInterval2.at(realInterval(_, _)))),
                                                    some(as<Interval>(iInterval3.at(realInterval(_, _))))))))) = [&]
            {
                auto result = unionInterval(*iInterval1, *iInterval2);
                auto newI = std::get<Interval>(*result);
                return unionInterval(newI, *iInterval3);
            }, 
            pattern | ds(_, some(as<SetOp>(as<Union>(iUnion2)))) = [&]
            {
                return mergeUnion(Union{{lhs}}, *iUnion2);
            },
            pattern | ds(some(as<SetOp>(as<Union>(iUnion1))), _) = [&]
            {
                return mergeUnion(Union{{rhs}}, *iUnion2);
            },
            pattern | _ = [&]
            {
                return std::make_shared<Expr const>(SetOp{Union{{lhs, rhs}}});
            });
    }

    ExprPtr functionRangeImpl(ExprPtr const &function, ExprPtr const &symbol, ExprPtr const &domain)
    {
#if DEBUG
        std::cout << "functionRangeImpl: " << toString(function) << ",\t" << toString(symbol) << ",\t" << toString(domain) << std::endl;
#endif // DEBUG

        Id<Union> iUnion;
        return match(*domain)(
            pattern | as<Interval>(_) = [&]
            { return std::make_shared<Expr const>(functionRangeImplIntervalDomain(function, symbol, domain)); },
            pattern | as<SetOp>(as<Union>(iUnion)) = [&]
            {
                ExprPtr result = false_;
                for (auto &&e : *iUnion)
                {
                    result = union_(result, functionRangeImpl(function, symbol, e));
                }
                return result;
            },
            pattern | _ = [&]
            {
                throw std::logic_error{"Mismatch in functionRangeImpl!"};
                return false_;
            });
    }

    ExprPtr functionRange(ExprPtr const &function, ExprPtr const &symbol, ExprPtr const &domain)
    {
#if DEBUG
        std::cout << "functionRange: " << toString(function) << ",\t" << toString(symbol) << ",\t" << toString(domain) << std::endl;
#endif // DEBUG

        Id<PieceWise> iPieceWise;
        return match(*function)(
            pattern | as<PieceWise>(iPieceWise) = [&]
            {
                // FIXME union expr instead of interval
                return std::accumulate((*iPieceWise).begin(), (*iPieceWise).end(), false_, [&](auto &&result, auto &&e)
                                       {
                                           auto const newDomain = intersect(solveInequation(e.second, symbol), domain);
                                           if (equal(newDomain, false_))
                                           {
                                               return result;
                                           }
                                           return union_(result, functionRangeImpl(e.first, symbol, newDomain));
                                       });
            },
            pattern | _ = [&]
            { return functionRangeImpl(function, symbol, domain); });
    }
} // namespace mathiu::impl
