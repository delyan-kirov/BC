{
  description = "C++ dev environment for BC";

  inputs.nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";

  outputs = { self, nixpkgs }: 
    let
      system = "x86_64-linux";
      pkgs = import nixpkgs { inherit system; };
    in
    {
      devShells.${system}.default = pkgs.mkShell {
        buildInputs = [
          pkgs.clang
          pkgs.gcc
          pkgs.gnumake
          pkgs.bear
          pkgs.git
          pkgs.valgrind

          # Build tools for lib
          pkgs.autoconf
          pkgs.automake
          pkgs.libtool
          pkgs.texinfo
          pkgs.libffi
          pkgs.cmakeWithGui

          # Extra libs
          pkgs.raylib
          pkgs.libffi

          # X11 support for raylib
          pkgs.xorg.libX11
          pkgs.xorg.libX11.dev
          pkgs.xorg.libXcursor
          pkgs.xorg.libXi
          pkgs.xorg.libXinerama
          pkgs.xorg.libXrandr
        ];

        shellHook = ''
        '';
      };
    };
}

