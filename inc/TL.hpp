#ifndef TL_HEADER
#define TL_HEADER

#include "EX.hpp"
#include "UT.hpp"

namespace TL
{

#define TypeEnumVariants                                                       \
  X(IntDef)                                                                    \
  X(ExtDef)

enum class Type
{
#define X(enum) enum,
  TypeEnumVariants
#undef X
};

struct Def
{
  Type       m_type;
  UT::String m_name;
  EX::Expr   m_expr;
};

struct Mod
{
  UT::String   m_name;
  UT::Vec<Def> m_defs;

  Mod(UT::String file_name);
};

EX::Expr eval(EX::Expr expr);
} // namespace TL

namespace std
{
inline string
to_string(
  TL::Type type)
{
  switch (type)
  {
#define X(X_enum)                                                              \
  case TL::Type::X_enum: return #X_enum;
    TypeEnumVariants
  }
}

} // namespace std

#endif // TL_HEADER
