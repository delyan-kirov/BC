#include <cstddef>
#include <exception>
#include <iostream>
#include <ostream>
#include <string>
#include <vector>

#include "parser.hpp"
#include "tokenizer.hpp"

using std::string;
using std::vector;

int
main ()
{
  string input = "1 - (2 - (3 + 43)) + 4 - 1";
  // string input = "1 + 1";
  std::cout << input << std::endl;
  vector<Token> tokens = tokenize (input);

  // for (auto token : tokens)
  // {
  //   std::cout << std::to_string (token) << '\n';
  // }

  Expr *expr = new Expr;

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
}

/*
 * TODO:
     - Split code so that it's not as crazy as this
     - Use arenas
     - Add testing
     - Use cpp for building instead of makefile
 */
