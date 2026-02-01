#include "TL.hpp"
#include "UT.hpp"

constexpr UT::String sut_file = "./examples/addition.se";

int
main()
{
  TL::Mod mod(sut_file);

  // AR::Arena  arena;
  // UT::String addition_se = UT::read_entrie_file(sut_file, arena);
  // LX::Lexer  l{ addition_se.m_mem, arena, 0, addition_se.m_len };
  // (void)l.run();
  // std::printf("%s\n", UT_TCS(l.m_tokens));
  // l.generate_event_report();
  // EX::Parser parser{ l };
  // parser.run();
  // std::printf("Parser: %s\n", UT_TCS(*parser.m_exprs.begin()));
  // EX::Expr result = TL::eval(*parser.m_exprs.begin());
  // std::printf("Evaluated to %s\n", UT_TCS(result));
}
