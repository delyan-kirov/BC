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
    UT::SB sb{};
    sb.concatf ("%s %s", data);
    this->m_data = (void *)sb.collect ();
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
    UT::SB sb{};
    sb.concatf ("%s %s", fn_name, data);
    this->m_data = (void *)sb.collect ();
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
  const char *m_fn_name;
  T &m_event_log;

  Trace (AR::T &arena, const char *fn_name, T &event_log)
      : UT::B{ arena, std::strlen (fn_name) + 2 * UT::V_DEFAULT_MAX_LEN }, //
        m_fn_name{ fn_name },                                              //
        m_event_log{ event_log }                                           //
  {
    if (TRACE_ENABLE)
    {
      UT::SB sb{};
      sb.concatf ("%s :> begin", fn_name);

      ER::TraceE e{ (void *)sb.collect () };
      this->m_event_log.push (e);

      this->push (this->m_fn_name);
      this->push (":> ");
    }
  };

  ~Trace ()
  {
    if (TRACE_ENABLE)
    {
      UT::SB sb{};
      sb.concatf ("%s :> end", this->m_fn_name);

      ER::TraceE e{ (void *)sb.collect () };
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
      ER::TraceE e{ (void *)UT::SB::strdup (this->m_mem) };
      this->m_event_log.push (e);

      std::memset (this->m_mem, 0, this->m_len);
      this->m_len = 0;

      this->push (this->m_fn_name);
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
