#ifndef ER_HEADER
#define ER_HEADER

#include "UT.hpp"
#include <cstring>
#include <string>

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

  void trace (E e, const char *fmt, ...) UT_PRINTF_LIKE (3, 4);

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

struct Trace : UT::B
{
  const char *m_fn_name = 0;

  Trace () = default;
  Trace (AR::T &arena)
  {
    this->m_arena = &arena;
    this->m_len = 0;
    this->m_max_len = UT::V_DEFAULT_MAX_LEN;
    this->m_mem = (char *)arena.alloc (UT::V_DEFAULT_MAX_LEN * sizeof (char));
    std::memset (this->m_mem, 0, this->m_max_len);
  };

  ~Trace ()
  {
    const char *s = this->m_fn_name;
    this->push ('\n');
    while (*s)
    {
      char c = *s;
      this->push (c);
      ++s;
    }
  }

  B &
  operator<< (const char *s)
  {

    if (!m_fn_name)
    {
      std::string prefix_string
          = std::string ("ER::Trace(") + std::string (s);
      prefix_string += ")>: ";

      const char *prefix = prefix_string.c_str ();
      size_t prefix_len = std::strlen (prefix);

      for (size_t i = 0; i < prefix_len; ++i)
      {
        this->push (prefix[i]);
      }
      this->m_fn_name
          = (const char *)this->m_arena->alloc (sizeof (char) * prefix_len);
      std::strcpy ((char *)this->m_fn_name, s);
    }

    while (*s)
    {
      char c = *s;
      this->push (c);
      ++s;
    }

    return *this;
  }
};

}

#endif
