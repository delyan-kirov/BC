#include "LX.hpp"
#include "ER.hpp"
#include <cassert>
#include <string>

namespace LX
{
LX::E
LX::L::find_matching_paren (size_t &paren_match_idx)
{
  auto trace = ER::Trace (this->m_arena, __PRETTY_FUNCTION__, this->m_events);
  size_t stack = 1;

  for (size_t idx = this->m_cursor; this->m_input[idx]; ++idx)
  {
    char c = this->m_input[idx];
    if (')' == c) { stack -= 1; }
    else if ('(' == c) { stack += 1; }
    if (0 == stack)
    {
      paren_match_idx = idx;
      trace << "Found matching paren at: "
            << std::to_string (this->m_cursor).c_str () << trace.end ();
      return LX::E::OK;
    }
  }

  LX_ERROR_REPORT (LX::E::PARENTHESIS_UNBALANCED, "");
}

char
L::next_char ()
{
  if (this->m_input[this->m_cursor])
  {
    char c = this->m_input[this->m_cursor];
    if ('\n' == c) { this->m_lines += 1; }
    this->m_cursor += 1;
    return c;
  }
  return '\0';
}

LX::E
L::push_int ()
{
  auto trace = ER::Trace (this->m_arena, __PRETTY_FUNCTION__, this->m_events);
  int result = 0;
  size_t cursor = this->m_cursor;
  size_t lines = this->m_lines;

  std::string s{
    this->m_input[this->m_cursor
                  - 1 /* since we entered this function, the point
                         where we need to start parsing is offset by 1 */
  ]
  };
  for (char c = this->next_char (); c; c = this->next_char ())
  {
    if (!c) { break; }
    if (std::isdigit (c)) { s += c; }
    else { break; }
  }

  try
  {
    result = std::stoi (s.c_str (), nullptr, 10);

    LX::T t{ LX::Type::Int };
    t.as.m_int = result;
    this->m_tokens.push (t);
  }
  catch (std::exception &e)
  {
    this->m_cursor = cursor;
    this->m_lines = lines;
    LX_ERROR_REPORT (E::NUMBER_PARSING_FAILURE, "");
  }

  if (this->m_input[this->m_cursor])
  {
    // We parsed one char more, we need to go back one step
    this->m_cursor -= 1;
  }

  return LX::E::OK;
}

void
L::push_operator (char c)
{
  auto trace = ER::Trace (this->m_arena, __PRETTY_FUNCTION__, this->m_events);

  LX::Type t_type = LX::Type::Unknown;
  switch (c)
  {
  case '-': t_type = LX::Type::Minus; break;
  case '+': t_type = LX::Type::Plus; break;
  case '*': t_type = LX::Type::Mult; break;
  case '/': t_type = LX::Type::Div; break;
  case '%': t_type = LX::Type::Modulus; break;
  default : /* UNREACHABLE */ assert (false && "push_operator");
  }
  this->m_tokens.push (LX::T{ t_type });
}

LX::E
L::run ()
{
  auto trace = ER::Trace (this->m_arena, __PRETTY_FUNCTION__, this->m_events);

  for (char c = this->next_char ();          //
       c && (this->m_cursor <= this->m_end); //
       c = this->next_char ()                //
  )
  {
    switch (c)
    {
    case '-':
    case '+':
    case '*':
    case '/':
    case '%':
    {
      this->push_operator (c);
    }
    break;
    case '(':
    {
      auto result = LX::E::OK;

      size_t group_begin = this->m_cursor + 1;
      size_t group_end = this->m_cursor + 1;

      result = this->find_matching_paren (group_end);
      if (LX::E::OK != result)
      {
        LX_ERROR_REPORT (result, "Function run failed");
      }

      LX::L new_l = LX::L (*this, group_begin, group_end);
      result = new_l.run ();

      if (LX::E::OK == result) { this->subsume_sub_lexer (new_l); }
      else { LX_ERROR_REPORT (result, "new_l failed"); }
    }
    break;
    case ')':
    {
      assert (0 && "case ')'");
    }
    break;
    case ' ':
    {
      ; // Do nothing
    }
    break;
    case '\n':
    {
      ; // Do nothing
    }
    break;
    default:
    {
      LX::E result = this->push_int ();
      if (LX::E::OK != result) { LX_ERROR_REPORT (result, ""); }
    }
    break;
    }
  }

  return LX::E::OK;
}

void
L::generate_event_report ()
{
  ER::T events = this->m_events;
  for (size_t i = 0; i < events.m_len; ++i)
  {
    ER::E e = events.m_mem[i];
    if (ER::Type::ERROR == e.m_type)
    {
      LX::E event = *(LX::E *)e.m_data;
      const char *prefix = "\033[31mERROR\033[0m";
      std::printf ("[%s] %s\n", prefix, std::to_string (event).c_str ());

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
L::subsume_sub_lexer (L &l)
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
} // namespace LX
