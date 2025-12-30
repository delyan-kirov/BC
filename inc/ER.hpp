#ifndef ER_HEADER
#define ER_HEADER

#include "UT.hpp"

namespace ER
{
struct E
{
  void *m_data = nullptr;
  void (*make) (void) = nullptr;
  char *(*fmt) (void *options) = nullptr;
};

struct T
{
  AR::T &m_arena;
  UT::V<E> m_es;

  void
  push (E e)
  {
    m_es.push (e);
  }

  void
  trace (const char *fmt, ...) UT_PRINTF_LIKE(2, 3)
  {
    (void)fmt;
  }

  E &
  operator[] (size_t i)
  { // for writing
    return this->m_es[i];
  }

  const E &
  operator[] (size_t i) const
  { // for reading from const objects
    return this->m_es[i];
  }

  ~T () = default;
  T (AR::T &arena) : m_arena (arena) { m_es = UT::V<E>{ arena }; }
};

}

#endif
