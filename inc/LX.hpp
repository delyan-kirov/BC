#ifndef LX_HEADER
#define LX_HEADER

#include "ER.hpp"
#include "UT.hpp"
#include <string>

namespace LX
{
enum class Type
{
  Unknown = 0,
  Int,
  Plus,
  Minus,
  Div,
  Modulus,
  Mult,
  Group,
};

struct T;
using Tokens = UT::V<T>;

struct T
{
  Type m_type;
  size_t m_line;
  size_t m_cursor;
  union
  {
    int m_int = 0;
    Tokens m_tokens;
  } as;

  T () = default;
  ~T () = default;
  T (Type t) : m_type{ t }, m_line{ 0 }, m_cursor{ 0 }, as{} {};
  T (Tokens tokens) : m_type{ Type::Group }, m_line{ 0 }, m_cursor{ 0 }
  {
    new (&as.m_tokens) Tokens (tokens); // NOTE: placement new
  };
};

namespace
{

} // namespace anonymous

struct L
{
  AR::T &m_arena;
  ER::T m_events;
  const char *m_input;
  Tokens m_tokens;
  size_t m_lines;
  size_t m_cursor;
  size_t m_begin;
  size_t m_end;

  L (const char *const input, AR::T &arena, size_t begin, size_t end)
      : m_arena{ arena },           //
        m_events{ arena },          //
        m_input{ input },           //
        m_tokens{ Tokens (arena) }, //
        m_lines{ 0 },               //
        m_cursor{ 0 },              //
        m_begin{ begin },           //
        m_end{ end }                //
  {
  }

  L (L const &l) : m_arena{ l.m_arena }, m_events (l.m_arena)
  {
    this->m_begin = l.m_begin;
    this->m_end = l.m_end;
    this->m_cursor = l.m_cursor;
    this->m_input = l.m_input;
    new (&this->m_tokens) Tokens{ l.m_arena };
  }

  ~L ()
  {
    for (size_t i = 0; i < this->m_events.m_len; ++i)
    {
      ER::E e = this->m_events[i];
      e.free (e.m_data);
    }
  }

  char next_char ();

  void push_int ();

  void push_operator (char c);

  void run ();
};

} // namespace LX

namespace std
{
inline string
to_string (LX::Type t)
{
  switch (t)
  {
  case LX::Type::Unknown: return "Unknown";
  case LX::Type::Int    : return "Int";
  case LX::Type::Plus   : return "Plus";
  case LX::Type::Minus  : return "Minus";
  case LX::Type::Mult   : return "Mult";
  case LX::Type::Div    : return "Div";
  case LX::Type::Modulus: return "Modulus";
  case LX::Type::Group  : return "Group";
  }
}

inline string to_string (LX::Tokens ts);
inline string
to_string (LX::T t)
{
  switch (t.m_type)
  {
  case LX::Type::Unknown: return "Unknown";
  case LX::Type::Int:
    return string ("Int") + "(" + to_string (t.as.m_int) + ")";
  case LX::Type::Plus:
    return "Op("
           "+"
           ")";
  case LX::Type::Minus:
    return "Op("
           "-"
           ")";
  case LX::Type::Mult:
    return "Op("
           "*"
           ")";
  case LX::Type::Div:
    return "Op("
           "/"
           ")";
  case LX::Type::Modulus:
    return "Op("
           "%"
           ")";
  case LX::Type::Group:
  {
    return to_string (t.as.m_tokens);
  }
  }
  string s = "Unreachable";
  return s;
}

inline string
to_string (LX::Tokens ts)
{
  string s{ "LX::Tokens [ " };
  for (size_t i = 0; i < ts.m_len; ++i)
  {
    LX::T t = ts[i];
    if (LX::Type::Group == t.m_type)
    {
      s += to_string ((LX::Tokens)t.as.m_tokens); //
    }
    else
    {
      s += to_string (t); //
    }
    s += (i != ts.m_len - 1) ? " , " : "";
  }
  s += " ]";
  return s;
}

}

#endif // LX_HEADER
