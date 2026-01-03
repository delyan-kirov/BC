#ifndef LX_HEADER
#define LX_HEADER

#include "ER.hpp"
#include "UT.hpp"
#include <limits>
#include <stdio.h>

namespace LX
{

constexpr size_t TOKENIZER_FAILED = std::numeric_limits<size_t>::max ();

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

  L (L const &l)
      : m_arena (l.m_arena),   //
        m_events (l.m_events), //
        m_input{ l.m_input },  //
        m_tokens (l.m_tokens), //
        m_lines (l.m_lines),   //
        m_cursor (l.m_cursor), //
        m_begin (l.m_begin),   //
        m_end (l.m_end)        //
  {
    for (size_t i = 0; i < l.m_events.m_len; ++i)
    {
      ER::E e = l.m_events[i];
      char *e_new_m_data = (char *)e.clone (e.m_data);

      ER::E new_e = e;
      new_e.m_data = (void *)e_new_m_data;
      this->m_events.push (new_e);
    }
  };

  L (L const &l, size_t begin, size_t end)
      : m_arena{ l.m_arena }, m_events (l.m_arena)
  {
    this->m_begin = l.m_begin;
    this->m_end = l.m_end;
    this->m_cursor = l.m_cursor;
    this->m_input = l.m_input;
    this->m_begin = begin;
    this->m_end = end;
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

  void
  generate_event_report ()
  {
    ER::T events = this->m_events;
    for (size_t i = 0; i < events.m_len; ++i)
    {
      ER::E e = events.m_mem[i];
      if (ER::Type::ERROR == e.m_type)
      {
        char *s = e.fmt (e.m_data);
        const char *prefix = "\033[31mERROR\033[0m";
        std::printf ("[%s] %s\n", prefix, s);

        // Find the line with the error
        size_t line = 1;
        size_t line_begin = this->m_begin;
        size_t line_end = this->m_end;

        // Locate the start of the line
        for (size_t i = this->m_begin; i < this->m_end; ++i)
        {
          if (this->m_input[i] == '\n')
          {
            line_begin = i + 1;
            line += 1;
          }
          if (i == this->m_cursor - 1) { break; }
        }

        // Locate the end of the line
        for (size_t i = line_begin + 1; i < this->m_end; ++i)
        {
          if (this->m_input[i] == '\n')
          {
            line_end = i;
            break;
          }
        }

        // Extract the line
        std::string msg;
        for (size_t i = line_begin; i < line_end; ++i)
        {
          msg += this->m_input[i];
        }
        size_t offset = (this->m_cursor - line_begin) + 1;

        // Print the error context
        std::printf ("   %ld |   \033[1;37m%s\033[0m\n", line, msg.c_str ());
        std::printf ("%*c\033[31m^\033[0m\n", (int)offset + 7, ' ');

        return;
      }
    }
  }

  void
  subsume_sub_lexer (L &l)
  {
    LX::T token{ l.m_tokens };

    this->m_tokens.push (token);
    this->m_cursor = l.m_cursor;

    for (size_t i = 0; i < l.m_events.m_len; ++i)
    {
      ER::E e = l.m_events[i];
      char *e_new_m_data = (char *)e.clone (e.m_data);

      ER::E new_e = e;
      new_e.m_data = (void *)e_new_m_data;
      this->m_events.push (new_e);
    }
  }

  char next_char ();

  size_t push_int ();

  void push_operator (char c);

  size_t run ();
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

  string s{ "ERROR: " };
  s += (__FUNCTION__);
  s += " UNREACHABLE PATCH REACHED!";
  return s;
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
  string s{ "ERROR: " };
  s += (__FUNCTION__);
  s += " UNREACHABLE PATCH REACHED!";
  return s;
}

inline string
to_string (LX::Tokens ts)
{
  string s{ "[ " };
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
