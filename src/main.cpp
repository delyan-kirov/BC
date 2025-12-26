#include <cstdio>
#include <cstring>

#include "AR.hpp"
#include "EX.hpp"

int
main ()
{
  const char msg[] = "Hello world\n";
  char *new_msg = nullptr;
  size_t *word = 0;
  AR::T arena{};
  new_msg = (char *)arena.alloc (5000);
  std::strcpy (new_msg, msg);
  word = (size_t *)arena.alloc (sizeof (size_t));
  *word = 69;

  EX::T *expr = (EX::T *)arena.alloc (sizeof (EX::T));
  std::printf("%p\n", expr);

  expr->m_int = 3;
  expr->m_type = EX::Type::Int;

  std::printf ("%d, %s", expr->m_int, new_msg);
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
