#include "ER.hpp"
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

  char *ptr = nullptr;
  {
    ER::Trace trace{
      arena,
    };
    ptr = trace.m_mem;

    trace << __func__ ;
  }
  std::cout << std::string (ptr) << std::endl;

  LX::L l{ input, arena, 0, std::strlen (input) + 1 };
  l.run ();
  std::cout << std::to_string (l.m_tokens) << std::endl;
}
