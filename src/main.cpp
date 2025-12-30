#include "LX.hpp"
#include <cstring>
#include <iostream>
#include <string>

int
main ()
{
  const char *input = "(1 2 3) 3 4 \n";
  std::cout << input;
  AR::T arena{};
  LX::L l{ input, arena, 0, std::strlen (input) + 1 };
  l.run ();
  std::cout << std::to_string (l.m_tokens) << std::endl;
}
