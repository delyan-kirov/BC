#ifndef ER_HEADER
#define ER_HEADER

#include "UT.hpp"
#include <iostream>
#include <string>

namespace ER
{
enum class Type
{
  MIN = 0,
  ERROR,
  WARNING,
  UNFO,
  MAX,
};

struct E
{
  Type m_type = Type::MIN;
  void *m_data = nullptr;
  void (*make) (void *m_data) = nullptr;
  char *(*fmt) (void *m_data) = nullptr;
};

namespace
{
char *
info_trace_fmt (void *m_data)
{
  char *info_event = (char *)m_data;
  return info_event;
}
}

class T : public UT::V<E>
{
public:
  T (AR::T &arena) : UT::V<E>{ arena } {}
  T () = delete;
  ~T () = default;

  using UT::V<E>::push;
  using UT::V<E>::operator[];
  void
  dump_to_stdin ()
  {
    for (size_t i = 0; i < this->m_len; ++i)
    {
      E e = this->m_mem[i];
      char *s = e.fmt (e.m_data);
      std::cout << s << std::endl;
    }
  }

  // void trace (E e, const char *fmt, ...) UT_PRINTF_LIKE (3, 4);
};

class Trace : public UT::B
{
public:
  // TODO: support format
  const char *m_fn_name;
  T &m_event_log;

  Trace (AR::T &arena, const char *fn_name, T &event_log)
      : UT::B{ arena, std::strlen (fn_name) + 2 * UT::V_DEFAULT_MAX_LEN }, //
        m_fn_name{ fn_name },                                              //
        m_event_log{ event_log }                                           //
  {
    this->push (fn_name);
    this->push (" :> ");
    {
      this->push ("begin\n");
    }
    this->push (fn_name);
    this->push (" :> ");
  };

  ~Trace ()
  {
    this->push ("end\n");
    ER::E e{};
    e.m_type = Type::UNFO;
    e.m_data = this->m_mem;
    e.fmt = info_trace_fmt;
    this->m_event_log.push (e);
  }

  B &
  operator<< (const char *s)
  {
    this->push (s);
    return *this;
  }

  const char *
  end ()
  {
    this->push ('\n');
    this->push (m_fn_name);
    this->push (" :> ");
    return "";
  }
};

} // namespace ER

namespace std
{
inline string
to_string (ER::Type type)
{
  switch (type)
  {
  case ER::Type::MIN    : return "MIN";
  case ER::Type::ERROR  : return "ERROR";
  case ER::Type::WARNING: return "WARNING";
  case ER::Type::UNFO   : return "UNFO";
  case ER::Type::MAX    : return "MAX";
  }

  return "UNREACHABLE";
}
}

#endif
