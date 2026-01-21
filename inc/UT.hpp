#ifndef UT_HEADER
#define UT_HEADER

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <initializer_list>
#include <string>
#include <utility>

#if defined(__GNUC__) || defined(__clang__)
#define UT_PRINTF_LIKE(fmt_idx, arg_idx)                                       \
  __attribute__((format(printf, fmt_idx, arg_idx)))
#else
#define UT_PRINTF_LIKE(fmt_idx, arg_idx)
#endif

// TODO: should report line and file
#define UT_TODO(TODO_MSG)                                                      \
  UT::IMPL::fail_if(                                                           \
    __FILE__, __PRETTY_FUNCTION__, __LINE__, UT::STODO, #TODO_MSG)

#define UT_FAIL_IF(CONDITION)                                                  \
  do                                                                           \
  {                                                                            \
    if (CONDITION)                                                             \
    {                                                                          \
      UT::IMPL::fail_if(                                                       \
        __FILE__, __PRETTY_FUNCTION__, __LINE__, UT::SERROR, #CONDITION);      \
    }                                                                          \
  } while (false)

#define UT_TCS(o) (std::to_string(o).c_str())

namespace AR
{
constexpr size_t BLOCK_DEFAULT_LEN = (1 << 10);

class Arena;
class Block
{
  friend class Arena;

private:
  size_t  len;
  size_t  max_len;
  uint8_t mem[];

  Block()  = delete;
  ~Block() = delete;

  Block(const Block &)            = delete;
  Block &operator=(const Block &) = delete;

  Block(Block &&)            = delete;
  Block &operator=(Block &&) = delete;
};

class Arena
{
public:
  void *alloc(size_t size);

  template <typename Type>
  void *
  alloc()
  {
    return alloc(sizeof(Type));
  }

  template <typename Type>
  void *
  alloc(
    size_t size)
  {
    return alloc(size * sizeof(Type));
  }

  template <typename Type>
  void *
  alloc(
    Type *t)
  {
    return alloc(sizeof(t));
  }

  Arena();
  ~Arena();

private:
  size_t  len;
  size_t  max_len;
  Block **mem;
};

constexpr size_t DEFAULT_T_MEM_SIZE = 8;

inline AR::Arena::Arena()
{
  this->len     = 1;
  this->max_len = DEFAULT_T_MEM_SIZE;
  this->mem     = (Block **)malloc(sizeof(Block *) * DEFAULT_T_MEM_SIZE);

  Block *block = (Block *)std::malloc(
    sizeof(Block) + sizeof(uint8_t) * AR::BLOCK_DEFAULT_LEN);

  block->max_len = AR::BLOCK_DEFAULT_LEN;
  block->len     = 0;

  this->mem[0] = block;
}

inline AR::Arena::~Arena()
{
  for (size_t i = 0; i < this->len; ++i)
  {
    Block *block = this->mem[i];
    std::free(block);
  }
  std::free(this->mem);

  return;
}

inline void *
AR::Arena::alloc(
  size_t size)
{
  if (!size)
  {
    return nullptr;
  }
now_allocate:
  Block *block       = this->mem[this->len - 1];
  size_t size_of_ptr = sizeof(void *);
  size_t alloc_size  = ((size + size_of_ptr - 1) / size_of_ptr) * size_of_ptr;
  size_t mem_left    = block->max_len - block->len;

  void *ptr = nullptr;

  if (mem_left >= alloc_size)
  {
    ptr = block->mem + block->len;
    block->len += alloc_size;
  }
  else // The current block is full
  {
    if (this->max_len > this->len) // Create a new block
    {
      size_t block_new_size
        = sizeof(Block)
          + sizeof(uint8_t) * std::max(alloc_size, AR::BLOCK_DEFAULT_LEN);

      Block *block   = (Block *)malloc(block_new_size);
      block->len     = 0;
      block->max_len = block_new_size - sizeof(Block);

      this->mem[this->len] = block;
      this->len += 1;

      goto now_allocate;
    }
    else // The aray is full, we need to resize it
    {
      size_t block_new_len = this->len * 2;
      auto   new_mem
        = (Block **)std::realloc(this->mem, block_new_len * sizeof(Block *));

      this->mem     = new_mem;
      this->max_len = block_new_len;

      goto now_allocate;
    }
  }

  return ptr;
}

} // namespace AR


namespace UT
{

constexpr const char *STODO  = "TODO";
constexpr const char *SERROR = "\033[31mERROR\033[0m";

namespace IMPL
{

inline void
abort()
{
#if defined(_MSC_VER)
  std::abort();
#elif defined(__x86_64__) || defined(_M_X64) || defined(__i386__)              \
  || defined(_M_IX86)
  asm("int3");
#else
  std::abort();
#endif
};

inline void
fail_if(
  const char *file,    //
  const char *fn_name, //
  const int   line,    //
  const char *prefix,  //
  const char *msg)
{
  if (msg)
  {
    std::printf("[%s] %s : %s\n", prefix, file, fn_name);
    std::printf("  %d | \033[1;37m%s\033[0m\n", line, msg);
    UT::IMPL::abort();
  }
}

} // namespace IMPL

constexpr size_t V_DEFAULT_MAX_LEN = 1 << 6;

template <typename O> struct Vu
{
  O     *m_mem;
  size_t m_len;

  Vu()
      : m_mem{ nullptr },
        m_len{ 0 } {};
  constexpr Vu(O *o, size_t len)
      : m_mem{ o },
        m_len{ len } {};
  constexpr Vu(const char *s, size_t len)
      : m_mem{ (char *)s },
        m_len{ len } {};

  Vu(
    const char *s)
      : m_mem{ s }
  {
    if (s)
    {
      this->m_len = std::strlen(s);
    }
    else
    {
      UT_FAIL_IF("Provided string is null");
    }
  };

  Vu(
    std::string s)
      : m_mem{ s.c_str() }
  {
    this->m_len = s.size();
  }

  bool
  is_empty()
  {
    return 0 == m_len;
  };

  O *
  get()
  {
    return this->m_mem;
  };

  O &
  operator[](
    size_t i)
  { // for writing
    return this->m_mem[i];
  };

  const O &
  operator[](
    size_t i) const
  { // for reading from const objects
    return this->m_mem[i];
  };

  const O *
  begin() const
  {
    return this->m_mem;
  };
  const O *
  end() const
  {
    return this->m_mem + this->m_len;
  };
  O *
  begin()
  {
    return this->m_mem;
  };
  O *
  end()
  {
    return this->m_mem + this->m_len;
  };

  O *
  last()
  {
    return this->m_mem + (this->m_len - 1);
  };
};

struct String : public Vu<char>
{
  template <size_t N>
  constexpr String(
    const char (&mem)[N])
      : Vu<char>{ mem, N - 1 }
  {
  }
  // Construct from pointer + length
  String(
    const char *mem, size_t len)
      : Vu<char>{ (char *)mem, len }
  {
  }
  String(
    char *mem, size_t len)
      : Vu<char>{ mem, len }
  {
  }

  String()                          = default;
  String(const String &)            = default;
  String(String &&)                 = default;
  String &operator=(const String &) = default;
  String &operator=(String &&)      = default;

  const char *
  to_cstr(
    AR::Arena &arena)
  {
    char *mem = (char *)arena.alloc((this->m_len + 1) * sizeof(char));
    std::memset(mem, 0, this->m_len + 1);
    std::strcpy(mem, this->m_mem);
    return mem;
  }
};

inline String
memcopy(
  AR::Arena &arena, const char *s)
{
  size_t s_len = std::strlen(s);
  auto   new_s = (char *)arena.alloc(s_len + 1);
  (void)std::memcpy(new_s, s, s_len);
  String result{ new_s, s_len };
  return result;
}

inline String
strdup(
  AR::Arena &arena, const char *s, size_t len)
{
  auto new_s = (char *)arena.alloc(len);
  (void)std::memcpy(new_s, s, len);
  String result{ new_s, len };
  return result;
}

inline bool
strcompare(
  const String s1, const String s2)
{
  return s1.m_len == s2.m_len && 0 == std::memcmp(s1.m_mem, s2.m_mem, s1.m_len);
}

inline String
strdup(
  AR::Arena &arena, String s)
{
  auto new_s = (char *)arena.alloc(s.m_len + 1);
  (void)std::memcpy(new_s, s.m_mem, s.m_len);
  new_s[s.m_len] = 0;
  String result{ new_s, s.m_len };
  return result;
}

template <typename O> struct Vec
{
  O         *m_mem;
  size_t     m_len;
  size_t     m_max_len;
  AR::Arena *m_arena;

  Vec()                       = default;
  ~Vec()                      = default;
  Vec(const Vec &other)       = default;
  Vec &operator=(const Vec &) = default;

  Vec(
    Vec &&other)
      : Vec{ other }
  {
    other.m_arena   = nullptr;
    other.m_len     = 0;
    other.m_max_len = 0;
    other.m_mem     = nullptr;
  }

  Vec(
    AR::Arena &arena, size_t len = 0)
      : m_len{ 0 },
        m_max_len{ V_DEFAULT_MAX_LEN },
        m_arena{ &arena }
  {
    size_t alloc_len = (0 == len) ? V_DEFAULT_MAX_LEN : len;
    this->m_mem      = (O *)arena.alloc<O>(alloc_len);
  };

  Vec(
    std::initializer_list<size_t> lst)
      : m_arena{ 0 },
        m_len{ 0 },
        m_max_len{ 0 },
        m_mem{ 0 }
  {
    (void)lst;
  };

  Vec(
    AR::Arena &arena, std::initializer_list<O> lst)
      : m_arena{ 0 },
        m_len{ 0 },
        m_max_len{ V_DEFAULT_MAX_LEN },
        m_mem{ 0 }
  {
    this->m_mem = (O *)arena.alloc<O>(this->m_max_len);
    for (const O &o : lst)
    {
      this->push(o);
    }
  };

  const O *
  begin() const
  {
    return this->m_mem;
  };
  const O *
  end() const
  {
    return this->m_mem + this->m_len;
  };
  O *
  begin()
  {
    return this->m_mem;
  };
  O *
  end()
  {
    return this->m_mem + this->m_len;
  };

  O *
  last()
  {
    return this->m_mem + (this->m_len - 1);
  };

  O &
  operator[](
    size_t i)
  { // for writing
    return this->m_mem[i];
  };

  const O &
  operator[](
    size_t i) const
  { // for reading from const objects
    return this->m_mem[i];
  };

  void
  push(
    O o)
  {
    if (this->m_len >= this->m_max_len)
    {
      // We need more space
      O *new_mem = (O *)this->m_arena->alloc(sizeof(O) * 2 * this->m_max_len);
      std::memcpy((void *)new_mem, this->m_mem, sizeof(O) * this->m_len);

      this->m_mem = new_mem;
      this->m_max_len *= 2;
    }
    this->m_mem[this->m_len] = o;
    this->m_len += 1;
  };

  bool
  is_empty()
  {
    return 0 == this->m_len;
  }
};

class SB
{
public:
  char  *m_mem;
  size_t m_len;
  size_t m_max_len;

  SB()
      : m_len{ 0 }
  {
    this->m_mem     = new char[sizeof(char) * V_DEFAULT_MAX_LEN];
    this->m_max_len = V_DEFAULT_MAX_LEN;
    std::memset(this->m_mem, 0, this->m_max_len);
  }

  SB(const SB &)            = delete; // copy constructor
  SB &operator=(const SB &) = delete; // copy assignment
  SB(SB &&)                 = delete; // move constructor
  SB &operator=(SB &&)      = delete; // move assignment

  ~SB()
  {
    if (this->m_mem)
    {
      delete[] this->m_mem;
      this->m_mem = nullptr;
    }
  }

  void
  resize(
    size_t new_len)
  {
    size_t new_max_len = 2 * (this->m_max_len + new_len);
    char  *new_mem     = new char[new_max_len];
    std::memset(new_mem, 0, new_max_len);
    std::strcpy(new_mem, this->m_mem);
    delete[] this->m_mem;
    this->m_mem     = new_mem;
    this->m_max_len = new_max_len;
  }

  void
  add(
    const char *s)
  {
    size_t available_space = this->m_max_len - this->m_len;
    size_t s_len           = std::strlen(s);
    if (available_space < s_len)
    {
      this->resize(s_len);
    }
    std::strcat(this->m_mem, s);
    this->m_len += s_len;
  }

  void
  add(
    const char c)
  {
    size_t available_space = this->m_max_len - this->m_len;
    size_t s_len           = 1;
    if (available_space < s_len)
    {
      this->resize(s_len);
    }
    this->m_mem[this->m_len] = c;
    this->m_len += s_len;
  }

  template <typename... Args> void concat(Args &&...args);
  template <typename... Args> void concatf(const char *fmt, Args &&...args);
  template <typename... Args> void append(Args &&...args);

  const String
  to_String(
    AR::Arena &arena)
  {
    char *mem = (char *)arena.alloc(sizeof(char) * this->m_len + 1);
    std::memset(mem, 0, this->m_len + 1);
    std::memcpy(mem, this->m_mem, this->m_len);
    return String{ mem, this->m_len };
  }

  const char *
  to_cstr(
    AR::Arena &arena)
  {
    char *mem = (char *)arena.alloc(sizeof(char) * this->m_len + 1);
    std::memset(mem, 0, this->m_len + 1);
    std::memcpy(mem, this->m_mem, this->m_len);
    return mem;
  }

  const String
  vu()
  {
    return String{ this->m_mem, this->m_len };
  }

  SB &
  operator>>(
    const char *s)
  {
    this->add(s);
    return *this;
  }

  SB &
  operator>>(
    String str)
  {
    this->add(str.m_mem);
    return *this;
  }

  SB &
  operator>>(
    SB &sb)
  {
    String vu = sb.vu();
    this->add(vu.m_mem);
    return *this;
  }

  template <typename T>
  SB &
  operator>>(
    T &t)
  {
    std::string s = std::to_string(t);
    this->add(s.c_str());
    return *this;
  }
};

template <typename... Args>
void
SB::concatf(
  const char *fmt, Args &&...args)
{
  char *buffer;
  (void)asprintf(&buffer, fmt, std::forward<Args>(args)...);
  this->concat(buffer);
  std::free(buffer);
}

template <typename... Args>
void
SB::concat(
  Args &&...args)
{
  (..., this->add(std::forward<Args>(args)));
}

template <typename... Args>
void
SB::append(
  Args &&...args)
{
  (..., this->concat(std::forward<Args>(args), " "));
}

class Block : public Vec<char>
{
public:
  Block() = delete;

  Block(
    AR::Arena &arena, size_t size)
  {
    this->m_arena   = &arena;
    this->m_len     = 0;
    this->m_max_len = size;
    this->m_mem     = (char *)arena.alloc(this->m_max_len * sizeof(char));
    std::memset(this->m_mem, 0, this->m_max_len);
  }

  Block(
    AR::Arena &arena)
      : Block{ arena, V_DEFAULT_MAX_LEN }
  {
  }

  using Vec<char>::push;
  void
  push(
    const char *s)
  {
    if (!s || !(*s))
    {
      return;
    }
    size_t s_len         = std::strlen(s);
    size_t available_mem = this->m_max_len - this->m_len;
    if (s_len >= available_mem)
    {
      size_t new_max_len = 2 * (this->m_max_len + s_len);
      char  *new_mem = (char *)this->m_arena->alloc(new_max_len * sizeof(char));
      std::memset(new_mem, 0, new_max_len);
      std::memcpy(new_mem, this->m_mem, this->m_len);
      this->m_max_len = new_max_len;
      this->m_mem     = new_mem;
    }
    std::memcpy(this->m_mem + this->m_len, s, s_len);
    this->m_len += s_len;
  }

  Block &
  operator<<(
    char c)
  {
    this->push(c);
    return *this;
  }

  Block &
  operator<<(
    const char *s)
  {
    while (*s)
    {
      char c = *s;
      this->push(c);
      ++s;
    }
    return *this;
  }
};
} // namespace UT

namespace std
{
inline string
to_string(
  UT::String s)
{
  const char *var_mem = new char[s.m_len + 1];
  memset((void *)var_mem, 0, s.m_len + 1);
  memcpy((void *)var_mem, s.begin(), s.m_len);
  string result{ var_mem };
  delete[] var_mem;
  return result;
}
} // namespace std

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
  Type       m_type          = Type::MIN;
  AR::Arena *m_arena         = nullptr;
  void      *m_data          = nullptr;
  char *(*fmt)(void *m_data) = nullptr;

  E();
  E(
    Type       type,
    AR::Arena &arena,
    void      *data         = nullptr,
    char *(*fmt_fn)(void *) = nullptr)
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
info_trace_fmt(
  void *m_data)
{
  char *info_event = (char *)m_data;
  return info_event;
}

} // namespace

struct TraceE : public E
{
  TraceE(
    void *data, AR::Arena &arena)
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
  Events(
    AR::Arena &arena)
      : UT::Vec<E>{ arena }
  {
  }
  Events()                    = delete;
  ~Events()                   = default;
  Events(const Events &other) = default;
  Events(
    Events &&other)
  {
    this->m_arena   = other.m_arena;
    this->m_len     = other.m_len;
    this->m_max_len = other.m_max_len;
    this->m_mem     = other.m_mem;

    other.m_mem     = nullptr;
    other.m_len     = 0;
    other.m_max_len = 0;
    other.m_arena   = nullptr;
  }

  using UT::Vec<E>::push;
  using UT::Vec<E>::operator[];

  void
  dump_to_stdin()
  {
    for (size_t i = 0; i < this->m_len; ++i)
    {
      E           e      = this->m_mem[i];
      char       *s      = e.fmt(e.m_data);
      const char *prefix = "";
      switch (e.m_type)
      {
      case ER::Type::ERROR: prefix = "\033[31mERROR\033[0m"; break;
      case ER::Type::UNFO : prefix = "\033[32mINFO\033[0m"; break;
      default             : break;
      }
      std::printf("[%s]%s\n", prefix, s);
    }
  }

  // void trace (E e, const char *fmt, ...) UT_PRINTF_LIKE (3, 4);
};

class Trace : public UT::Block
{
public:
  const char *m_fn_name;
  Events     &m_event_log;

  Trace(
    AR::Arena &arena, const char *fn_name, Events &event_log)
      : UT::Block{ arena, std::strlen(fn_name) + 2 * UT::V_DEFAULT_MAX_LEN }, //
        m_fn_name{ fn_name },                                                 //
        m_event_log{ event_log }                                              //
  {
    if (TRACE_ENABLE)
    {
      UT::SB sb{};
      sb.concatf("%s :> begin", fn_name);

      auto       s = sb.to_String(*this->m_arena);
      ER::TraceE e{ s.m_mem, arena };
      this->m_event_log.push(e);

      this->push(this->m_fn_name);
      this->push(":> ");
    }
  };

  ~Trace()
  {
    if (TRACE_ENABLE)
    {
      UT::SB sb{};
      sb.concatf("%s :> end", this->m_fn_name);

      ER::TraceE e{ (void *)sb.to_String(*this->m_arena).m_mem, *this->m_arena };
      this->m_event_log.push(e);
    }
  }

  Block &
  operator<<(
    const char *s)
  {
    if (TRACE_ENABLE)
    {
      this->push(s); //
    }
    return *this;
  }

  const char *
  end()
  {
    if (TRACE_ENABLE)
    {
      ER::TraceE e{ (void *)this->m_mem, *this->m_arena };
      this->m_event_log.push(e);

      std::memset(this->m_mem, 0, this->m_len);
      this->m_len = 0;

      this->push(this->m_fn_name);
      this->push(" :> ");
    }
    return "";
  }
};

} // namespace ER

namespace std
{
inline string
to_string(
  ER::Type type)
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
} // namespace std


#endif // UT_HEADER
