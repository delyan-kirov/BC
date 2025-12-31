#ifndef ER_HEADER
#define ER_HEADER

#include "UT.hpp"
#include <cstring>
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
  void (*free) (void *m_data) = nullptr;
  void* (*clone) (void *m_data) = nullptr;
};

namespace
{
char *
info_trace_fmt (void *m_data)
{
  char *info_event = (char *)m_data;
  return info_event;
}

void info_trace_free (void *m_data)
{
  delete (char*)m_data;
}

void *info_trace_clone (void *m_data)
{
  char *new_data = new char[std::strlen((char*)m_data)];
  std::strcpy(new_data, (char*)m_data);
  return (char*)new_data;
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
    size_t fn_name_len = std::strlen (fn_name);
    char *fn_name_cpy = new char[fn_name_len + 1];
    std::strcpy (fn_name_cpy, fn_name);
    this->m_fn_name = fn_name_cpy;

    std::string prolog_s = { "" };
    prolog_s += fn_name;
    prolog_s += ":> begin";
    const char *prolog = prolog_s.c_str ();
    size_t prolog_len = prolog_s.size ();
    char *msg = new char[prolog_len];
    std::memcpy (msg, prolog, prolog_len);

    ER::E e{};
    e.m_type = Type::UNFO;
    e.m_data = msg;
    e.fmt = info_trace_fmt;
    e.free = info_trace_free;
    e.clone = info_trace_clone;
    this->m_event_log.push (e);

    this->push (m_fn_name);
    this->push (":> ");
  };

  ~Trace ()
  {
    std::string epilog_s = { this->m_fn_name };
    epilog_s += ":> end";
    const char *prolog = epilog_s.c_str ();
    size_t prolog_len = epilog_s.size ();
    char *msg = new char[prolog_len];
    std::memcpy (msg, prolog, prolog_len);

    ER::E e{};
    e.m_type = Type::UNFO;
    e.m_data = msg;
    e.fmt = info_trace_fmt;
    e.free = info_trace_free;
    e.clone = info_trace_clone;
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
    char *msg = new char[this->m_len + 1];
    std::strcpy (msg, this->m_mem);

    ER::E e{};
    e.m_type = Type::UNFO;
    e.m_data = msg;
    e.fmt = info_trace_fmt;
    e.free = info_trace_free;
    e.clone = info_trace_clone;
    this->m_event_log.push (e);

    std::memset (this->m_mem, 0, this->m_len);
    this->m_len = 0;

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
