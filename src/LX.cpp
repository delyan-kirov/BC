#include "LX.hpp"
#include "ER.hpp"
#include "UT.hpp"
#include <cassert>
#include <cctype>
#include <cstdio>
#include <cstring>
#include <string>

namespace LX
{

bool
Lexer::match_keyword (UT::String keyword, UT::String word)
{
  return UT::strcompare (keyword, word);
}

UT::String
LX::Lexer::get_word ()
{
  UT::SB sb{};
  char c = this->m_input[this->m_cursor - 1];
  sb.add (c);
  while ((c = this->next_char ()) //
         && (c != ' ')            //
         && !std::isdigit (c)
         // And more, TODO
  )
  {
    sb.add (c);
  }
  UT::String string = sb.collect (this->m_arena);

  return string;
}

LX::E
LX::Lexer::find_matching_paren (size_t &paren_match_idx)
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
Lexer::next_char ()
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
Lexer::push_int ()
{
  auto trace = ER::Trace (this->m_arena, __PRETTY_FUNCTION__, this->m_events);
  int result = 0;
  size_t cursor = this->m_cursor;
  size_t lines  = this->m_lines;

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
    else
    {
      break;
    }
  }

  try
  {
    result = std::stoi (s.c_str (), nullptr, 10);

    LX::Token t{ LX::Type::Int };
    t.as.m_int = result;
    this->m_tokens.push (t);
  }
  catch (std::exception &e)
  {
    this->m_cursor = cursor;
    this->m_lines  = lines;
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
Lexer::push_operator (char c)
{
  auto trace = ER::Trace (this->m_arena, __PRETTY_FUNCTION__, this->m_events);

  LX::Type t_type = LX::Type::Min;
  switch (c)
  {
  case '-': t_type = LX::Type::Minus; break;
  case '+': t_type = LX::Type::Plus; break;
  case '*': t_type = LX::Type::Mult; break;
  case '/': t_type = LX::Type::Div; break;
  case '%': t_type = LX::Type::Modulus; break;
  default : /* UNREACHABLE */ assert (false && "push_operator");
  }
  this->m_tokens.push (LX::Token{ t_type });
}

LX::E
Lexer::run ()
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
      size_t group_end   = this->m_cursor + 1;

      result = this->find_matching_paren (group_end);
      if (LX::E::OK != result)
      {
        LX_ERROR_REPORT (result, "Function run failed");
      }

      LX::Lexer new_l = LX::Lexer (*this, group_begin, group_end);
      result          = new_l.run ();

      if (LX::E::OK == result) { this->subsume_sub_lexer (new_l); }
      else
      {
        LX_ERROR_REPORT (result, "new_l failed");
      }
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
    case '=':
    {
      /* Nothing to do */ return LX::E::OK;
    }
    break;
    case '\n':
    {
      ; // Do nothing
    }
    break;
    default:
    {
      if (std::isdigit (c))
      {
        LX::E result = this->push_int ();
        if (LX::E::OK != result) { LX_ERROR_REPORT (result, ""); }
      }
      else
      {
        // TODO: This is complete garbage
        // We should cut spaces and we should be operator aware
        UT::String word = this->get_word ();
        if (this->match_keyword (LX::KEYWORD_LET, word))
        {
          // TODO: this should be gone
          this->next_char ();
          UT::String var_name = this->get_word ();
          std::printf ("var name is %.*s\n", (int)var_name.m_len,
                       var_name.m_mem);

          this->match_operator ('=');

          Lexer let_lexer{ *this, this->m_cursor };
          let_lexer.run ();

          Lexer in_lexer{ *this, let_lexer.m_cursor };
          in_lexer.run ();

          Token letin{};
          letin.m_type                = Type::LetIn;
          letin.m_line                = this->m_lines;
          letin.m_cursor              = this->m_cursor;
          letin.as.m_lenin.var_name   = var_name;
          letin.as.m_lenin.let_tokens = let_lexer.m_tokens;
          letin.as.m_lenin.in_tokens  = in_lexer.m_tokens;

          this->m_tokens.push (letin);
          // TODO: do not do that, there should be a new method that advances
          // from the previous tokenizer
          this->m_cursor = in_lexer.m_cursor;
        }
        else if (this->match_keyword (LX::KEYWORD_IN, word))
        {
          /* Nothing to do */ return LX::E::OK;
        }
        else
        {
          LX_ERROR_REPORT (LX::E::UNRECOGNIZED_STRING, "");
        }
      }
    }
    break;
    }
  }

  return LX::E::OK;
}

void
Lexer::generate_event_report ()
{
  ER::Events events = this->m_events;
  for (size_t i = 0; i < events.m_len; ++i)
  {
    ER::E e = events.m_mem[i];
    if (ER::Type::ERROR == e.m_type)
    {
      LX::E event = *(LX::E *)e.m_data;
      std::printf ("[%s] %s\n", UT::SERROR, std::to_string (event).c_str ());

      // Find the line with the error
      size_t line       = 1;
      size_t line_begin = this->m_begin;
      size_t line_end   = this->m_end;

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
Lexer::subsume_sub_lexer (Lexer &l)
{
  for (auto t : l.m_tokens)
  {
    LX::Token token{ t };
    this->m_tokens.push (token);
  }
  this->m_cursor = l.m_cursor;

  for (size_t i = 0; i < l.m_events.m_len; ++i)
  {
    ER::E e = l.m_events[i];
    this->m_events.push (e);
  }
}

E
Lexer::match_operator (char c)
{
  return c == this->next_char () ? E::OK : E::UNRECOGNIZED_STRING;
};
} // namespace LX
