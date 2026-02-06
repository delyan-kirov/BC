#include "EX.hpp"
#include "LX.hpp"
#include "TL.hpp"
#include "UT.hpp"

#define TSTxCTL 0

namespace TDATA
{
constexpr std::pair<const char *, int> INPUTS[] = {
#if 0
  { "1 / 2", 0 },
  { "3 * 2 + 2 / 2", 7 },
  // { "((3*(244 - 57 + 4)*(244 - 57 + 4) - (74 - 777)) / (5 - (44 + 77 - "
  //   "47)*(44 + 77 - 47)*(44 + 77 - 47)) + ((4 + 27)*(3 - 7)))",
  //   -124 },
  { "2 * 2", 4 },
  { "1 - 1 - 2", -2 },
  { "1 + 2 * 2", 5 },
  // { "2 + - (- 2 * 2) + - - (- ((1 + 1))) * 1 + (1 * 1)", 5 },
  { "-2", -2 },
  { "2 + - 2 * 2", -2 },
  { "(((7 * (3 + 2)) - (5 * (2 + (6 * 1)))) * ((4 + (9 * (3 - 1))) - (2 * (1"
    "+ 3)))) + (((12 * (8 - (3 * (2 + 1)))) - (5 * (6 - (2 * (1 + (2 * "
    "2)))))) * (3 + (4 * (2 + (1 * 5)))))",
    178 },
  { "(1 + 2) * 2", 6 },
  { "1", 1 },
  { "(1)", 1 },
  { "1 + (1 + 1) + 1", 4 },
  { "1 + 1 + (1 + 1) + (((1))) + (((1) + 1))", 7 },
  { "1 - 2 - 3", -4 },
  { "(1 - 2) - (3 - 1)", -3 },
  { "1 - 2", -1 },
  { "2 * 3 * 4", 24 },
  { "2 * -3 * 4", -24 },
  { "2 * -3", -6 },
  // { "2 * - -3", 6 },
  // { "2 * - (- -3) - 1", -7 },
  { "2 + -3", -1 },
  { "-1", -1 },
  { "-1 + 2", 1 },
  // { "- -1 - ((1 + 1)) - 1", -2 },
  { "- (-(1 + 2))", 3 },
  // { "- - - 1", -1 },
  { "1 - (2 - (3 + 43)) + 4 - ( 1 + 3   )", 45 },
  { "10 - 5 - 2", 3 },
  { "10 - (5 - 2)", 7 },
  { "(1 + 2) + (3 + 4)", 10 },
  { "(((((5)))))", 5 },
  { "-5 + ((((-(-5)))))", 0 },
  { "0 - 1", -1 },
  { "0 - (1 + 2 + 3)", -6 },
  { "(8 - 3) - (2 - 1)", 4 },
  { "42", 42 },
  { "(7 + 3) - (2 + 1)", 7 },
  { "1 - (2 - (3 - (4 - 5)))", 3 },
  { "1 - 2 - 3 - 4", -8 },
  { "1 - (2 - 3) - 4", -2 },
  // { "1 - - 4 * 2 + 1", 10 },
  // { "\n\n\n(12 1s 32\n (3) 2123 \n\n234 4\n( 1 2 (3) \n4 5", 3 },
  { "(1 - 2) - (3 - 4)", 0 },
  { "100 - (50 + 25)", 25 },
  { "(100 - 50) + 25", 75 },
  { "1 + (2 + (3 + (4 + 5)))", 15 },
  { "1 - (2 + (3 + (4 + 5)))", -13 },
  { "(1 + 2) - (3 + (4 - 5))", 1 },
  { "((1 + 2) - 3) + (4 - 5)", -1 },
  { "0 - (0 - (0 - (0 - 1)))", 1 },
  { "5 % 3 + 1", 3 },
  { "3 % 2 % 6", 1 },
  { "(1 + 2) + -1 * (1 * (2 * -1) * 1 + ((1 * 1)))", 4 },
  { "2 - 3 * 4 + 1", 3 },
  { "12 / 3 / 2 + 1", 3 },
  { "(3 + ((((((1)) + 1))))) - (-(1 + 2)) - ((((1))))", 9 },
  { "3 - (-(1 + 2))", 6 },
  { "1 - 1", 1 },
  { "1 - 2 - (3 - (4 + (5 - (6 + (7 - (8 - (9 + (10 - (11 + 12)))))))))", 4 },
  { "1 - 2 * (3 * 3) + 5 * -3", 1 },
  { "2 * - (-3)", -6 },
  { "3 * 5 * 3", 45 },
  { "-1 - 2 * 3 + 3", 5 },
  { "5 % - 3 + 1", 3 },
  { "-a", -2 },
  { "(1 + (1 + 2))", 3 },
  { "let a = 3 in a + 1 + 2", -2 },
  //    => (\a = a + 1 + 1) 3
  { "let foo = \\x = x + 1 in (foo 2)", -2 },
  //    => (\foo = foo 2) (\x = x  + 1)
  //    => (\x = x + 1) 2
  //    => 2 + 1
  { "if 1 - 1 => 2 + 3 else let x = 4 in x + 5", 9 },
  { "let x = 3 in if x ?= 1 => 1 else if x ?= 2 => 2 else 3", 3 },
  { "let foo = \\x = x + 1 in foo 1", 2 },
  { "let xtreme = 34 in if xtreme => xtreme + 35 else 3", 69 },
  { "let foo = \\x = x x in foo foo", 2 }, // NOTE: INFINITE RECURSION
  { "let a = 1 in let foo = \\b = b + a + 1 in foo (foo 2)", 6 },
  { "if (let a = 1 in a) => 1 else 0", 0 }, // FIXME: This should be valid
  { "let foo = \\x = (if x => x - 1 else x) in foo 3", 2 }, // FIXME: Bracketting is broken
  { "if 1 => 2 + 3 else let x = 4 in x + 5", 5 },
#endif

};
} // namespace TDATA

namespace
{
bool
run()
{
  for (auto tdata : TDATA::INPUTS)
  {
    AR::Arena   arena{};
    const char *input = tdata.first;
    LX::Lexer   l{ input, arena, 0, std::strlen(input) };
    (void)l.run();
    std::printf("%s\n", UT_TCS(l.m_tokens));
    l.generate_event_report();
    EX::Parser parser{ l };
    parser.run();
    std::printf("Parser: %s\n", UT_TCS(*parser.m_exprs.begin()));

    TL::Instance instance{ *parser.m_exprs.begin(), TL::Env{} };
    TL::Instance result = TL::eval(instance);
    std::printf("Evaluated to %s\n", UT_TCS(result.m_expr));

    if (EX::Type::Int == result.m_expr.m_type)
    {
      UT_FAIL_IF(tdata.second != result.m_expr.as.m_int);
    }
  }

  return true;
}
} // namespace

int
main()
{
  if (!run())
  {
    return 1;
  }
}
