#ifndef LX_HEADER
#define LX_HEADER

#include "AR.hpp"
#include "ER.hpp"
#include "UT.hpp"
#include <stdio.h>

namespace LX
{

#define LX_ERROR_REPORT(LX_ERROR_E, LX_ERROR_MSG)                             \
  do                                                                          \
  {                                                                           \
    this->m_events.push (LX::ErrorE{ this->m_arena, __PRETTY_FUNCTION__,      \
                                     (LX_ERROR_MSG), (LX_ERROR_E) });         \
    return (LX_ERROR_E);                                                      \
  } while (false)

enum class E
{
  MIN = (ssize_t)-1,
  OK,
  PARENTHESIS_UNBALANCED,
  NUMBER_PARSING_FAILURE,
  MAX,
};

struct ErrorE : public ER::E
{
  ErrorE (AR::Arena &arena, const char *fn_name, const char *data, LX::E error)
      : E{
          ER::Type::ERROR,    //
          arena,              //
          (void *)data,       //
          ER::info_trace_fmt, //
        }
  {
    UT::SB sb{};
    sb.concatf ("%16c%s %s", '-', fn_name, data);
    UT::Vu<char> msg    = UT::memcopy (*this->m_arena, sb.vu ().m_mem);
    *(LX::E *)msg.m_mem = error;
    this->m_data        = (void *)msg.m_mem;
  }
};

enum class Type
{
  Min = 0,
  Int,
  Plus,
  Minus,
  Div,
  Modulus,
  Mult,
  Group,
  Max,
};

struct Token;
using Tokens = UT::Vec<Token>;

struct Token
{
  Type m_type;
  size_t m_line;
  size_t m_cursor;
  union
  {
    Tokens m_tokens;
    ssize_t m_int = 0;
  } as;

  Token ()  = default;
  ~Token () = default;
  Token (Type t) : m_type{ t }, m_line{ 0 }, m_cursor{ 0 }, as{} {};
  Token (Tokens tokens) : m_type{ Type::Group }, m_line{ 0 }, m_cursor{ 0 }
  {
    new (&as.m_tokens) Tokens{ tokens }; // NOTE: placement new
  };
};

class Lexer
{
public:
  AR::Arena &m_arena;
  ER::Events m_events;
  const char *m_input;
  Tokens m_tokens;
  size_t m_lines;
  size_t m_cursor;
  size_t m_begin;
  size_t m_end;

  Lexer (const char *const input, AR::Arena &arena, size_t begin, size_t end)
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

  Lexer (Lexer const &l)
      : m_arena (l.m_arena),               //
        m_events (std::move (l.m_events)), //
        m_input{ l.m_input },              //
        m_tokens (l.m_tokens),             //
        m_lines (l.m_lines),               //
        m_cursor (l.m_cursor),             //
        m_begin (l.m_begin),               //
        m_end (l.m_end)                    //
  {
    for (size_t i = 0; i < l.m_events.m_len; ++i)
    {
      ER::E e = l.m_events[i];
      this->m_events.push (e);
    }
  };

  Lexer (Lexer const &l, size_t begin, size_t end)
      : m_arena{ l.m_arena }, m_events (l.m_arena)
  {
    this->m_begin  = l.m_begin;
    this->m_end    = l.m_end;
    this->m_cursor = l.m_cursor;
    this->m_input  = l.m_input;
    this->m_begin  = begin;
    this->m_end    = end;
    new (&this->m_tokens) Tokens{ l.m_arena };
  }

  ~Lexer () {}

  void generate_event_report ();

  void subsume_sub_lexer (Lexer &l);

  LX::E find_matching_paren (size_t &paren_match_idx);

  char next_char ();

  E push_int ();

  void push_operator (char c);

  E run ();
};

} // namespace LX

namespace std
{

inline string
to_string (LX::E e)
{
  switch (e)
  {
  case LX::E::MIN                   : return "MIN";
  case LX::E::OK                    : return "OK";
  case LX::E::PARENTHESIS_UNBALANCED: return "PARENTHESIS_UNBALANCED";
  case LX::E::NUMBER_PARSING_FAILURE: return "NUMBER_PARSING_FAILURE";
  case LX::E::MAX                   : return "MAX";
  }

  string s{ "ERROR: " };
  s += (__FUNCTION__);
  s += " UNREACHABLE PATH REACHED!";
  return s;
};

inline string
to_string (LX::Type t)
{
  switch (t)
  {
  case LX::Type::Min    : return "Min";
  case LX::Type::Int    : return "Int";
  case LX::Type::Plus   : return "Plus";
  case LX::Type::Minus  : return "Minus";
  case LX::Type::Mult   : return "Mult";
  case LX::Type::Div    : return "Div";
  case LX::Type::Modulus: return "Modulus";
  case LX::Type::Group  : return "Group";
  case LX::Type::Max    : return "Max";
  }

  string s{ "ERROR: " };
  s += (__FUNCTION__);
  s += " UNREACHABLE PATH REACHED!";
  return s;
}

inline string to_string (LX::Tokens ts);
inline string
to_string (LX::Token t)
{
  switch (t.m_type)
  {
  case LX::Type::Min:
  case LX::Type::Max: return "Unknown";
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
    LX::Token t = ts[i];
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
