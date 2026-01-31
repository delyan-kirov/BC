#include "TL.hpp"
#include "UT.hpp"
#include <cstdio>

UT::String
read_entrie_file(
  const char *file_name, AR::Arena &arena)
{
  FILE *file_stream = std::fopen(file_name, "rb");

  if (!file_stream)
  {
    std::fprintf(stderr, "ERROR: could not open file: %s\n", file_name);
  }

  std::fseek(file_stream, 0, SEEK_END);
  size_t file_len = ftell(file_stream);
  std::rewind(file_stream);
  char *buffer     = (char *)arena.alloc(sizeof(char) * (file_len + 1));
  buffer[file_len] = 0;

  // TODO: optimize
  size_t result = std::fread(buffer, file_len, 1, file_stream);
  if (result <= 0)
  {
    std::fprintf(
      stderr, "ERROR: could not map file %s to memory buffer\n", file_name);
  }

  return UT::String{ buffer, file_len };
}

constexpr const char *sut_file = "./examples/addition.se";

int
main()
{
  AR::Arena  arena;
  UT::String addition_se = read_entrie_file(sut_file, arena);
  LX::Lexer  l{ addition_se.m_mem, arena, 0, addition_se.m_len };
  (void)l.run();
  std::printf("%s\n", UT_TCS(l.m_tokens));
  l.generate_event_report();
  EX::Parser parser{ l };
  parser.run();
  std::printf("Parser: %s\n", UT_TCS(*parser.m_exprs.begin()));
  EX::Expr result = TL::eval(*parser.m_exprs.begin());
  std::printf("Evaluated to %s\n", UT_TCS(result));
}
