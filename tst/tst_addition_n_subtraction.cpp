#include <exception>
#include <iostream>
#include <string>

#include "EX.hpp"
#include "LX.hpp"

using std::string;
using std::vector;

namespace TDATA
{
constexpr std::pair<const char *, int> INPUTS[] = {
  { "1 - (2 - (3 + 43)) + 4 - ( 1 + 3   )", 45 },
  { "1+2", 3 },
  { "-2", -2 },
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
  { "(1 - 2) - (3 - 4)", 0 },
  { "100 - (50 + 25)", 25 },
  { "(100 - 50) + 25", 75 },
  { "1 + (2 + (3 + (4 + 5)))", 15 },
  { "1 - (2 + (3 + (4 + 5)))", -13 },
  { "(1 + 2) - (3 + (4 - 5))", 1 },
  { "((1 + 2) - 3) + (4 - 5)", -1 },
  { "0 - (0 - (0 - (0 - 1)))", 1 },
  { "1 - 2 - (3 - (4 + (5 - (6 + (7 - (8 - (9 + (10 - (11 + 12)))))))))", 4 },
};
}

namespace
{
int
eval (EX::T *expr)
{
  int result = 0;

  while (expr)
  {
    switch (expr->m_type)
    {
    case EX::Type::Plus:
    {
      return result = eval (expr->m_left) + eval (expr->m_right);
    }
    break;
    case EX::Type::Minus:
    {
      return result = eval (expr->m_left) - eval (expr->m_right);
    }
    break;
    case EX::Type::Int:
    {
      return result = expr->m_int;
    }
    break;
    case EX::Type::Unknown:
    {
      throw std::exception{};
    }
    break;
    }
  }

  return result;
}

bool
run ()
{
  bool result = true;
  AR::T arena{};
  for (auto tdata : TDATA::INPUTS)
  {
    const char *input = tdata.first;
    int expect = tdata.second;

    vector<LX::T> tokens = LX::run (input);
    EX::T *expr = (EX::T *)arena.alloc<EX::T> ();
    try
    {
      size_t result = parse (tokens, arena, 0, tokens.size (), expr);
      if (0 == result || EX::PARSER_FAILED == result)
      {
        std::cerr << "ERROR: Parser failed: " << result << std::endl;
        std::cerr << "       " << std::to_string (expr) << "\n";
        return -1;
      }
    }
    catch (std::exception &e)
    {
      std::cerr << "ERROR: parser failed!\n";
      return false;
    }
    catch (...)
    {
      std::cerr << "ERROR: unknown exception occured\n";
      return false;
    }

    int got = eval (expr);
    bool new_result = (got == expect);

    if (!new_result)
    {
      std::cerr << "ERROR: expected: " << expect << " but got: " << got
                << std::endl;
      std::cerr << "       input: " << input << " | "
                << "parsed: " << std::to_string (expr) << std::endl;
    }
    else
    {
      std::cout << "\033[32m" << "OK: " << "\033[0m" << input << " -> " << got
                << " | (" << std::to_string (expr) << ")" << std::endl;
    }

    result |= new_result;
  }
  return result;
}
}

int
main ()
{
  if (!run ()) { return 1; }
}
