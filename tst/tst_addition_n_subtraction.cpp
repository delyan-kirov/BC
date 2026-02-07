#include "TL.hpp"
#include "UT.hpp"

constexpr UT::String sut_file       = "./examples/addition.se";
constexpr UT::String sut_file_basic = "./examples/basic.se";

int
main()
{
  AR::Arena arena{};
  (void)sut_file_basic;
  (void)sut_file;

  TL::Mod mod(sut_file, arena);
}
