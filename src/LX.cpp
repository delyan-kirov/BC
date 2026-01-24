#include "LX.hpp"
#include "UT.hpp"
#include <cstdio>
#include <string>

namespace LX
{

bool
Lexer::match_keyword(
  UT::String keyword, UT::String word)
{
  return UT::strcompare(keyword, word);
}

UT::String
LX::Lexer::get_word(
  size_t idx)
{
  UT::SB sb{};
  this->strip_white_space(idx);
  idx = this->m_cursor;

  for (char c = this->m_input[idx++];       //
       c && (c != ' ') && !std::isdigit(c); // And more, TODO
       c = this->m_input[idx++])
  {
    sb.add(c);
  }
  UT::String string = sb.to_String(this->m_arena);
  this->m_cursor    = idx;

  return string;
}

LX::E
LX::Lexer::find_matching_paren(
  size_t &paren_match_idx)
{
  auto   trace = ER::Trace(this->m_arena, __PRETTY_FUNCTION__, this->m_events);
  size_t stack = 1;

  for (size_t idx = this->m_cursor; this->m_input[idx]; ++idx)
  {
    char c = this->m_input[idx];
    if (')' == c)
    {
      stack -= 1;
    }
    else if ('(' == c)
    {
      stack += 1;
    }
    if (0 == stack)
    {
      paren_match_idx = idx;
      trace << "Found matching paren at: "
            << std::to_string(this->m_cursor).c_str() << trace.end();
      return LX::E::OK;
    }
  }

  LX_ERROR_REPORT(LX::E::PARENTHESIS_UNBALANCED, "");
}

char
Lexer::next_char()
{
  if (this->m_input[this->m_cursor])
  {
    char c = this->m_input[this->m_cursor];
    if ('\n' == c)
    {
      this->m_lines += 1;
    }
    this->m_cursor += 1;
    return c;
  }
  return '\0';
}

LX::E
Lexer::push_int()
{
  auto   trace  = ER::Trace(this->m_arena, __PRETTY_FUNCTION__, this->m_events);
  int    result = 0;
  size_t cursor = this->m_cursor;
  size_t lines  = this->m_lines;

  std::string s{
    this->m_input[this->m_cursor
                  - 1 /* since we entered this function, the point
                         where we need to start parsing is offset by 1 */
  ]
  };
  for (char c = this->next_char(); c; c = this->next_char())
  {
    if (!c || !std::isdigit(c)) break;
    s += c;
  }

  try
  {
    result = std::stoi(s.c_str(), nullptr, 10);

    LX::Token t{ LX::Type::Int };
    t.as.m_int = result;
    this->m_tokens.push(t);
  }
  catch (std::exception &e)
  {
    this->m_cursor = cursor;
    this->m_lines  = lines;
    LX_ERROR_REPORT(E::NUMBER_PARSING_FAILURE, "");
  }

  if (this->m_input[this->m_cursor])
  {
    // We parsed one char more, we need to go back one step
    this->m_cursor -= 1;
  }

  return LX::E::OK;
}

void
Lexer::push_operator(
  char c)
{
  auto trace = ER::Trace(this->m_arena, __PRETTY_FUNCTION__, this->m_events);

  LX::Type t_type = LX::Type::Min;
  switch (c)
  {
  case '-': t_type = LX::Type::Minus; break;
  case '+': t_type = LX::Type::Plus; break;
  case '*': t_type = LX::Type::Mult; break;
  case '/': t_type = LX::Type::Div; break;
  case '%': t_type = LX::Type::Modulus; break;
  default : /* UNREACHABLE */ UT_FAIL_IF("UNERACHABLE");
  }
  this->m_tokens.push(LX::Token{ t_type });
}

LX::E
Lexer::run()
{
  auto trace = ER::Trace(this->m_arena, __PRETTY_FUNCTION__, this->m_events);

  for (char c = this->next_char();           //
       c && (this->m_cursor <= this->m_end); //
       c = this->next_char()                 //
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
      this->push_operator(c);
    }
    break;
    case '(':
    {
      size_t group_begin = this->m_cursor + 1;
      size_t group_end   = this->m_cursor + 1;

      LX_FN_TRY(this->find_matching_paren(group_end));

      LX::Lexer new_l = LX::Lexer(*this, group_begin, group_end);
      LX_FN_TRY(new_l.run());

      this->push_group(new_l);
    }
    break;
    case ')':
    {
      LX_ERROR_REPORT(LX::E::UNREACHABLE_CASE_REACHED,
                      "')' should never match in this branch");
    }
    break;
    case ' ':
    {
      ; // Do nothing
    }
    break;
    case '=':
    {
      LX_ASSERT('>' == this->next_char(), LX::E::OPERATOR_MATCH_FAILURE);
      return E::FAT_ARROW;
    }
    break;
    case '\n':
    {
      ; // Do nothing
    }
    break;
    case '\\': // \<var> = <expr>
    {
      this->strip_white_space(this->m_cursor);
      UT::String var_name = this->get_word(this->m_cursor);

      LX_FN_TRY(this->match_operator('='));

      Lexer body_lexer{
        this->m_input, this->m_arena, this->m_cursor, this->m_end
      };
      body_lexer.run();

      Token fn{};
      fn.m_type             = Type::Fn;
      fn.m_line             = this->m_lines;
      fn.m_cursor           = this->m_cursor;
      fn.as.m_fn.m_var_name = var_name;
      fn.as.m_fn.m_body     = body_lexer.m_tokens;

      this->m_tokens.push(fn);
      this->skip_to(body_lexer);

      return LX::E::OK;
    }
    break;
    default:
    {
      if (std::isdigit(c))
      {
        LX::E result = this->push_int();
        if (LX::E::OK != result)
        {
          LX_ERROR_REPORT(result, "");
        }
      }
      else
      {
        UT::String word = this->get_word(this->m_cursor - 1);
        if (this->match_keyword(LX::KEYWORD_LET, word))
        {
          UT::String var_name = this->get_word(this->m_cursor);

          LX_FN_TRY(this->match_operator('='));

          Lexer let_lexer{
            this->m_input, this->m_arena, this->m_cursor, this->m_end
          };
          let_lexer.run();

          Lexer in_lexer{ let_lexer.m_input,
                          let_lexer.m_arena,
                          let_lexer.m_cursor,
                          this->m_end };
          in_lexer.run();

          // TODO: Token should have an end
          Token token{};
          token.m_type                          = Type::Let;
          token.m_line                          = this->m_lines;
          token.m_cursor                        = this->m_cursor;
          token.as.m_let_in_tokens.m_var_name   = var_name;
          token.as.m_let_in_tokens.m_let_tokens = let_lexer.m_tokens;
          token.as.m_let_in_tokens.m_in_tokens  = in_lexer.m_tokens;

          this->m_tokens.push(token);
          this->skip_to(in_lexer);
        }
        else if (this->match_keyword(LX::KEYWORD_IN, word))
        {
          /* Nothing to do */ return LX::E::OK;
        }
        else if (this->match_keyword(LX::KEYWORD_IF, word))
        {
          Lexer if_condition_lexer{
            this->m_input, this->m_arena, this->m_cursor, this->m_end
          };
          LX_ASSERT(E::FAT_ARROW == if_condition_lexer.run(),
                    E::OPERATOR_MATCH_FAILURE);

          Lexer true_branch_lexer{ if_condition_lexer.m_input,
                                   if_condition_lexer.m_arena,
                                   if_condition_lexer.m_cursor,
                                   this->m_end };
          LX_ASSERT(E::ELSE_KEYWORD == true_branch_lexer.run(),
                    E::CONTROL_STRUCTURE_ERROR);

          Lexer else_branch_lexer{ true_branch_lexer.m_input,
                                   true_branch_lexer.m_arena,
                                   true_branch_lexer.m_cursor,
                                   this->m_end };
          LX_FN_TRY(else_branch_lexer.run());

          Token token{ Type::If };
          token.as.m_if_tokens.m_condition   = if_condition_lexer.m_tokens;
          token.as.m_if_tokens.m_true_branch = true_branch_lexer.m_tokens;
          token.as.m_if_tokens.m_else_branch = else_branch_lexer.m_tokens;

          this->m_tokens.push(token);
          this->skip_to(else_branch_lexer);
        }
        else if (this->match_keyword(LX::KEYWORD_ELSE, word))
        {
          return LX::E::ELSE_KEYWORD;
        }
        else
        {
          LX_ASSERT(word.m_len > 0, LX::E::UNRECOGNIZED_STRING);
          LX::Token t{ LX::Type::Word };
          t.as.m_string = word;
          this->m_tokens.push(t);
        }
      }
    }
    break;
    }
  }

  return LX::E::OK;
}

void
Lexer::generate_event_report()
{
  ER::Events events = this->m_events;
  for (size_t i = 0; i < events.m_len; ++i)
  {
    ER::E e = events.m_mem[i];
    if (ER::Type::ERROR == e.m_type)
    {
      LX::E event = *(LX::E *)e.m_data;
      std::printf("[%s] %s\n", UT::SERROR, std::to_string(event).c_str());

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
        if (i == this->m_cursor - 1)
        {
          break;
        }
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
      std::printf("   %ld |   \033[1;37m%s\033[0m\n", line, msg.c_str());
      std::printf("%*c\033[31m^\033[0m\n", (int)offset + 7, ' ');

      return;
    }
  }
}
void
Lexer::subsume_sub_lexer(
  Lexer &l)
{
  for (auto t : l.m_tokens)
  {
    LX::Token token{ t };
    this->m_tokens.push(token);
  }
  this->m_cursor = l.m_cursor;

  for (size_t i = 0; i < l.m_events.m_len; ++i)
  {
    ER::E e = l.m_events[i];
    this->m_events.push(e);
  }
}

E
Lexer::match_operator(
  char c)
{
  this->strip_white_space(this->m_cursor);
  LX_ASSERT(c == this->next_char(), E::UNRECOGNIZED_STRING);
  return E::OK;
};

void
Lexer::strip_white_space(
  size_t idx)
{
  char   c         = this->m_input[idx];
  size_t new_lines = 0;

  while (' ' == c || '\n' == c)
  {
    if ('\n' == c) new_lines += 1;
    idx += 1;
    c = this->m_input[idx];
  }

  this->m_lines += new_lines;
  this->m_cursor = idx;
};

void
Lexer::push_group(
  Lexer l)
{
  Token t{ l.m_tokens };
  this->m_tokens.push(t);
  this->m_cursor = l.m_cursor;
}
} // namespace LX
