#ifndef AR_HEADER
#define AR_HEADER

#include <cstddef>
#include <cstdint>

namespace AR
{
constexpr size_t BLOCK_DEFAULT_LEN = (1 << 10);

class Arena;
class Block
{
  friend class Arena;

private:
  size_t len;
  size_t max_len;
  uint8_t mem[];

  Block ()  = delete;
  ~Block () = delete;

  Block (const Block &)            = delete;
  Block &operator= (const Block &) = delete;

  Block (Block &&)            = delete;
  Block &operator= (Block &&) = delete;
};

class Arena
{
public:
  void *alloc (size_t size);

  template <typename Type>
  void *
  alloc ()
  {
    return alloc (sizeof (Type));
  }

  template <typename Type>
  void *
  alloc (size_t size)
  {
    return alloc (size * sizeof (Type));
  }

  template <typename Type>
  void *
  alloc (Type *t)
  {
    return alloc (sizeof (t));
  }

  Arena ();
  ~Arena ();

private:
  size_t len;
  size_t max_len;
  Block **mem;
};

} // namespace AR

#endif // AR_HEADER
