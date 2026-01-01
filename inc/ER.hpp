#ifndef ER_HEADER
#define ER_HEADER

#include "UT.hpp"
#include <cstdio>
#include <cstring>
#include <string>

namespace ER
{
constexpr bool TRACE_ENABLE =
#if TRACE_ENABLED
    true;
#else
    false;
#endif

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
  char *(*fmt) (void *m_data) = nullptr;
  void (*free) (void *m_data) = nullptr;
  void *(*clone) (void *m_data) = nullptr;

  E ();
  E (Type type,
     void *data = nullptr,
     char *(*fmt_fn) (void *) = nullptr,
     void (*free_fn) (void *) = nullptr,
     void *(*clone_fn) (void *) = nullptr)
      : m_type (type), m_data (data), fmt (fmt_fn), free (free_fn),
        clone (clone_fn)
  {
  }
};

namespace
{
char *
info_trace_fmt (void *m_data)
{
  char *info_event = (char *)m_data;
  return info_event;
}

void
info_trace_free (void *m_data)
{
  if (m_data) { delete[] (char *)m_data; }
}

void *
info_trace_clone (void *m_data)
{
  char *new_data = new char[std::strlen ((char *)m_data) + 1];
  std::strcpy (new_data, (char *)m_data);
  return (char *)new_data;
}

char *
stringdup (std::string &str)
{
  const char *str_buf = str.c_str ();
  size_t str_buf_len = std::strlen (str_buf);
  char *c_str = new char[str_buf_len + 1];
  std::memset (c_str, 0, str_buf_len + 1);
  std::memcpy (c_str, str_buf, str_buf_len);

  return c_str;
}

inline char *
stringdup (const char *str)
{
  const char *str_buf = str;
  size_t str_buf_len = std::strlen (str);
  char *c_str = new char[str_buf_len + 1];
  std::memset (c_str, 0, str_buf_len + 1);
  std::memcpy (c_str, str_buf, str_buf_len);

  return c_str;
}
}

struct TraceE : public E
{
  TraceE (void *data)
      : E{
          Type::UNFO,      //
          data,            //
          info_trace_fmt,  //
          info_trace_free, //
          info_trace_clone //
        }
  {
  }
};

struct ErrorE : public E
{
  ErrorE (void *data)
      : E{
          Type::ERROR,     //
          data,            //
          info_trace_fmt,  //
          info_trace_free, //
          info_trace_clone //
        }
  {
    auto s = (char *)data;
    size_t s_len = std::strlen (s);
    char *msg = new char[s_len + 1];
    std::memset (msg, 0, s_len + 1);
    std::strcpy (msg, s);
    this->m_data = (void *)msg;
  }
  ErrorE (const char *fn_name, void *data)
      : E{
          Type::ERROR,     //
          data,            //
          info_trace_fmt,  //
          info_trace_free, //
          info_trace_clone //
        }
  {
    // TODO: string builder
    auto s = (char *)data;
    size_t s_len = std::strlen (s);
    size_t fn_name_len = std::strlen (fn_name);
    size_t msg_len = s_len + fn_name_len;

    char *msg = new char[msg_len + 1];
    std::memset (msg, 0, msg_len + 1);
    std::strcpy (msg, fn_name);
    std::strcpy (msg + fn_name_len, s);
    this->m_data = (void *)msg;
  }
};

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
      const char *prefix = "";
      switch (e.m_type)
      {
      case ER::Type::ERROR: prefix = "\033[31mERROR\033[0m"; break;
      case ER::Type::UNFO : prefix = "\033[32mINFO\033[0m"; break;
      default             : break;
      }
      std::printf ("[%s]%s\n", prefix, s);
    }
  }

  // void trace (E e, const char *fmt, ...) UT_PRINTF_LIKE (3, 4);
};

class Trace : public UT::B
{
public:
  // TODO: support format
  std::string m_fn_name;
  T &m_event_log;

  Trace (AR::T &arena, const char *fn_name, T &event_log)
      : UT::B{ arena, std::strlen (fn_name) + 2 * UT::V_DEFAULT_MAX_LEN }, //
        m_fn_name{ fn_name },                                              //
        m_event_log{ event_log }                                           //
  {
    if (TRACE_ENABLE)
    {
      std::string prolog_s = { "" };
      prolog_s += fn_name;
      prolog_s += ":> begin";
      const char *prolog = stringdup (prolog_s);

      ER::TraceE e{ (void *)prolog };
      this->m_event_log.push (e);

      this->push (this->m_fn_name.c_str ());
      this->push (":> ");
    }
  };

  ~Trace ()
  {
    if (TRACE_ENABLE)
    {
      std::string epilog_s = { this->m_fn_name };
      epilog_s += ":> end";
      const char *prolog = stringdup (epilog_s);

      ER::TraceE e{ (void *)prolog };
      this->m_event_log.push (e);
    }
  }

  B &
  operator<< (const char *s)
  {
    if (TRACE_ENABLE)
    {
      this->push (s); //
    }
    return *this;
  }

  const char *
  end ()
  {
    if (TRACE_ENABLE)
    {
      char *msg = new char[this->m_len + 1];
      std::strcpy (msg, this->m_mem);

      ER::TraceE e{ msg };
      this->m_event_log.push (e);

      std::memset (this->m_mem, 0, this->m_len);
      this->m_len = 0;

      this->push (this->m_fn_name.c_str ());
      this->push (" :> ");
    }
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
