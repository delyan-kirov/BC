#include "ER.hpp"
#include "LX.hpp"
#include <cstring>
#include <iostream>
#include <string>

int
main ()
{
  const char *input = "(1 2 3 4) ( -(5  4) ) \n";

  AR::T arena{};
  ER::Trace trace{ arena };

  trace << __func__ << " " << input;

  LX::L l{ input, arena, 0, std::strlen (input) + 1 };
  l.run ();

  trace << std::to_string (l.m_tokens).c_str () << "\n";
  trace.~Trace();
  std::cout << trace.m_mem;
}
