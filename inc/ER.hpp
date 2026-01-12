#ifndef ER_HEADER
#define ER_HEADER

#include "AR.hpp"
#include "UT.hpp"
#include <cstdio>
#include <cstring>
#include <string>

// TODO: We should have proper event reporting with the arena
//    with types and with levels

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
  AR::Arena *m_arena = nullptr;
  void *m_data = nullptr;
  char *(*fmt) (void *m_data) = nullptr;

  E ();
  E (Type type,
     AR::Arena &arena,
     void *data = nullptr,
     char *(*fmt_fn) (void *) = nullptr)
      : m_type{ type },    //
        m_arena{ &arena }, //
        m_data{ data },    //
        fmt{ fmt_fn }      //
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

}

struct TraceE : public E
{
  TraceE (void *data, AR::Arena &arena)
      : E{
          Type::UNFO, //
          arena,
          data,           //
          info_trace_fmt, //
        }
  {
  }
};

class Events : public UT::Vec<E>
{
public:
  Events (AR::Arena &arena) : UT::Vec<E>{ arena } {}
  Events () = delete;
  ~Events () = default;
  Events (const Events &other) = default;
  Events (Events &&other)
  {
    this->m_arena = other.m_arena;
    this->m_len = other.m_len;
    this->m_max_len = other.m_max_len;
    this->m_mem = other.m_mem;

    other.m_mem = nullptr;
    other.m_len = 0;
    other.m_max_len = 0;
    other.m_arena = nullptr;
  }

  using UT::Vec<E>::push;
  using UT::Vec<E>::operator[];

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

class Trace : public UT::Block
{
public:
  const char *m_fn_name;
  Events &m_event_log;

  Trace (AR::Arena &arena, const char *fn_name, Events &event_log)
      : UT::Block{ arena,
                   std::strlen (fn_name) + 2 * UT::V_DEFAULT_MAX_LEN }, //
        m_fn_name{ fn_name },                                           //
        m_event_log{ event_log }                                        //
  {
    if (TRACE_ENABLE)
    {
      UT::SB sb{};
      sb.concatf ("%s :> begin", fn_name);

      auto s = sb.collect ();
      auto data = UT::memcopy (*this->m_arena, s, sb.m_len);
      ER::TraceE e{ data.m_mem, arena };
      this->m_event_log.push (e);

      this->push (this->m_fn_name);
      this->push (":> ");

      std::free ((void *)s);
    }
  };

  ~Trace ()
  {
    if (TRACE_ENABLE)
    {
      UT::SB sb{};
      sb.concatf ("%s :> end", this->m_fn_name);

      ER::TraceE e{ (void *)sb.collect (), *this->m_arena };
      this->m_event_log.push (e);
    }
  }

  Block &
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
      ER::TraceE e{ (void *)UT::SB::strdup (this->m_mem), *this->m_arena };
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
