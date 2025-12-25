#include <exception>
#include <iostream>

#include "EX.hpp"
#include "LX.hpp"

using std::string;
using std::vector;

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

int
main ()
{
  string input = "1 - (2 - (3 + 43)) + 4 - ( 1 + 3   )";
  // string input = "1 + 1";
  std::cout << input << std::endl;
  vector<LX::T> tokens = LX::run (input);

  // for (auto token : tokens)
  // {
  //   std::cout << std::to_string (token) << '\n';
  // }

  EX::T *expr = new EX::T;

  try
  {
    size_t end = 0;
    parse (tokens, end, end, tokens.size (), expr);
  }
  catch (std::exception &e)
  {
    std::cerr << "ERROR: parser failed!\n";
  }
  catch (...)
  {
    std::cerr << "ERROR: unknown exception occured\n";
  }

  std::cout << std::to_string (expr) << "\n";

  int result = eval (expr);
  std::cout << "INFO(result): " << result << std::endl;
}

/*
 * TODO:
     - Use arenas
     - Add tracing
     - Add error reporting and handling
     - Add top level comments
     - Add more operators
     - Support functions
     - Support streams
     - Add testing
 */
