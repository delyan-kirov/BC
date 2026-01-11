#ifndef UT_HEADER
#define UT_HEADER

#include "AR.hpp"
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <initializer_list>
#include <string>
#include <utility>

#if defined(__GNUC__) || defined(__clang__)
#define UT_PRINTF_LIKE(fmt_idx, arg_idx)                                      \
  __attribute__ ((format (printf, fmt_idx, arg_idx)))
#else
#define UT_PRINTF_LIKE(fmt_idx, arg_idx)
#endif

#define UT_TODO(TODO_MSG)                                                     \
  UT::IMPL::fail_if (__PRETTY_FUNCTION__, "TODO", #TODO_MSG)

#define UT_FAIL_IF(CONDITION)                                                 \
  do                                                                          \
  {                                                                           \
    if (CONDITION)                                                            \
    {                                                                         \
      UT::IMPL::fail_if (__PRETTY_FUNCTION__, "ERROR", #CONDITION);         \
    }                                                                         \
  } while (false)

namespace UT
{

namespace IMPL
{

inline void
abort ()
{
#if defined(_MSC_VER)
  std::abort ();
#elif defined(__x86_64__) || defined(_M_X64) || defined(__i386__)             \
    || defined(_M_IX86)
  asm ("int3");
#else
  std::abort ();
#endif
};

inline void
fail_if (const char *fn_name, const char *prefix, const char *msg)
{
  if (msg)
  {
    std::printf ("[%s] %s: %s\n", prefix, fn_name, msg);
    UT::IMPL::abort ();
  }
}

} // namespace UT::IMPL

constexpr size_t V_DEFAULT_MAX_LEN = 1 << 6;

template <typename O> struct Vu
{
  size_t m_len;
  O *m_mem;

  Vu (O *o, size_t len) : m_len{ len }, m_mem{ o } {};

  Vu (const char *s) : m_mem{ s }
  {
    if (s) { this->m_len = std::strlen (s); }
    else { UT_FAIL_IF ("Provided string is null"); }
  };

  Vu (std::string s) : m_mem{ s.c_str () } { this->m_len = s.size (); }

  bool
  is_empty ()
  {
    return 0 == m_len;
  };

  O *
  get ()
  {
    return this->m_mem;
  };

  O &
  operator[] (size_t i)
  { // for writing
    return this->m_mem[i];
  };

  const O &
  operator[] (size_t i) const
  { // for reading from const objects
    return this->m_mem[i];
  };

  const O *
  begin () const
  {
    return this->m_mem;
  };
  const O *
  end () const
  {
    return this->m_mem + this->m_len;
  };
  O *
  begin ()
  {
    return this->m_mem;
  };
  O *
  end ()
  {
    return this->m_mem + this->m_len;
  };

  O *
  last ()
  {
    return this->m_mem + (this->m_len - 1);
  };
};

template <typename O> struct Vec
{
  size_t m_len;
  size_t m_max_len;
  O *m_mem;
  AR::Arena *m_arena;

  Vec () = default;
  ~Vec () = default;
  Vec (const Vec &other) = default;
  Vec &operator= (const Vec &) = default;

  Vec (Vec &&other) : Vec{ other }
  {
    other.m_arena = nullptr;
    other.m_len = 0;
    other.m_max_len = 0;
    other.m_mem = nullptr;
  }

  Vec (AR::Arena &arena, size_t len = 0)
      : m_len{ 0 }, m_max_len{ V_DEFAULT_MAX_LEN }, m_arena{ &arena }
  {
    size_t alloc_len = (0 == len) ? V_DEFAULT_MAX_LEN : len;
    this->m_mem = (O *)arena.alloc<O> (alloc_len);
  };

  Vec (std::initializer_list<size_t> lst)
      : m_arena{ 0 }, m_len{ 0 }, m_max_len{ 0 }, m_mem{ 0 }
  {
    (void)lst;
  };

  Vec (AR::Arena &arena, std::initializer_list<O> lst)
      : m_arena{ 0 }, m_len{ 0 }, m_max_len{ V_DEFAULT_MAX_LEN }, m_mem{ 0 }
  {
    this->m_mem = (O *)arena.alloc<O> (this->m_max_len);
    for (const O &o : lst)
    {
      this->push (o);
    }
  };

  const O *
  begin () const
  {
    return this->m_mem;
  };
  const O *
  end () const
  {
    return this->m_mem + this->m_len;
  };
  O *
  begin ()
  {
    return this->m_mem;
  };
  O *
  end ()
  {
    return this->m_mem + this->m_len;
  };

  O *
  last ()
  {
    return this->m_mem + (this->m_len - 1);
  };

  O &
  operator[] (size_t i)
  { // for writing
    return this->m_mem[i];
  };

  const O &
  operator[] (size_t i) const
  { // for reading from const objects
    return this->m_mem[i];
  };

  void
  push (O o)
  {
    if (this->m_len >= this->m_max_len)
    {
      // We need more space
      O *new_mem
          = (O *)this->m_arena->alloc (sizeof (O) * 2 * this->m_max_len);
      std::memcpy ((void *)new_mem, this->m_mem, sizeof (O) * this->m_len);

      this->m_mem = new_mem;
      this->m_max_len *= 2;
    }
    this->m_mem[this->m_len] = o;
    this->m_len += 1;
  };

  bool
  is_empty ()
  {
    return 0 == this->m_len;
  }
};

class SB
{
  size_t m_len;
  size_t m_max_len;
  char *m_mem;

public:
  SB () : m_len{ 0 }
  {
    this->m_mem = new char[sizeof (char) * V_DEFAULT_MAX_LEN];
    this->m_max_len = V_DEFAULT_MAX_LEN;
    std::memset (this->m_mem, 0, this->m_max_len);
  }

  static const char *
  strdup (const char *s)
  {
    SB sb{};
    sb.concat (s);
    return sb.collect ();
  }

  SB (const SB &) = delete;            // copy constructor
  SB &operator= (const SB &) = delete; // copy assignment
  SB (SB &&) = delete;                 // move constructor
  SB &operator= (SB &&) = delete;      // move assignment

  ~SB ()
  {
    if (this->m_mem)
    {
      delete[] this->m_mem;
      this->m_mem = nullptr;
    }
  }

  void
  resize (size_t new_len)
  {
    size_t new_max_len = 2 * (this->m_max_len + new_len);
    char *new_mem = new char[new_max_len];
    std::memset (new_mem, 0, new_max_len);
    std::strcpy (new_mem, this->m_mem);
    delete[] this->m_mem;
    this->m_mem = new_mem;
    this->m_max_len = new_max_len;
  }

  void
  add (const char *s)
  {
    size_t available_space = this->m_max_len - this->m_len;
    size_t s_len = std::strlen (s);
    if (available_space < s_len) { this->resize (s_len); }
    std::strcat (this->m_mem, s);
    this->m_len += s_len;
  }

  template <typename... Args> void concat (Args &&...args);
  template <typename... Args> void concatf (const char *fmt, Args &&...args);
  template <typename... Args> void append (Args &&...args);

  const char *
  collect ()
  {
    const char *mem = this->m_mem;
    this->m_mem = nullptr;
    return mem;
  }
};

template <typename... Args>
void
SB::concatf (const char *fmt, Args &&...args)
{
  char *buffer;
  asprintf (&buffer, fmt, std::forward<Args> (args)...);
  this->concat (buffer);
  std::free (buffer);
}

template <typename... Args>
void
SB::concat (Args &&...args)
{
  (..., this->add (std::forward<Args> (args)));
}

template <typename... Args>
void
SB::append (Args &&...args)
{
  (..., this->concat (std::forward<Args> (args), " "));
}

class Block : public Vec<char>
{
public:
  Block () = delete;

  Block (AR::Arena &arena, size_t size)
  {
    this->m_arena = &arena;
    this->m_len = 0;
    this->m_max_len = size;
    this->m_mem = (char *)arena.alloc (this->m_max_len * sizeof (char));
    std::memset (this->m_mem, 0, this->m_max_len);
  }

  Block (AR::Arena &arena) : Block{ arena, V_DEFAULT_MAX_LEN } {}

  using Vec<char>::push;
  void
  push (const char *s)
  {
    if (!s || !(*s)) { return; }
    size_t s_len = std::strlen (s);
    size_t available_mem = this->m_max_len - this->m_len;
    if (s_len >= available_mem)
    {
      size_t new_max_len = 2 * (this->m_max_len + s_len);
      char *new_mem
          = (char *)this->m_arena->alloc (new_max_len * sizeof (char));
      std::memset (new_mem, 0, new_max_len);
      std::memcpy (new_mem, this->m_mem, this->m_len);
      this->m_max_len = new_max_len;
      this->m_mem = new_mem;
    }
    std::memcpy (this->m_mem + this->m_len, s, s_len);
    this->m_len += s_len;
  }

  Block &
  operator<< (char c)
  {
    this->push (c);
    return *this;
  }

  Block &
  operator<< (const char *s)
  {
    while (*s)
    {
      char c = *s;
      this->push (c);
      ++s;
    }
    return *this;
  }
};
}

#endif // UT_HEADER
