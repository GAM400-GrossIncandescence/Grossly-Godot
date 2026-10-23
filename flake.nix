{
  description = "A Nix-flake-based C/C++ development environment";

  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs =
    {
      self,
      nixpkgs,
      flake-utils,
      ...
    }:
    flake-utils.lib.eachDefaultSystem (
      system:
      let
        pkgs = nixpkgs.legacyPackages.${system};
        llvm = pkgs.llvmPackages_19;

        runtimeLibs =
          with pkgs;
          (pkgs.lib.options pkgs.stdenv.isDarwin [
            # xorg.libX11
            # xorg.libXcursor
            # xorg.libXinerama
            # xorg.libXrandr
            # xorg.libXi
            # xorg.libXext
            # libGL
            # alsa-lib
            # libpulseaudio
            # wayland
            # systemd
            # dbus
            # fontconfig
            # freetype
            # zlib
            # stdenv.cc.cc.lib
            # libxkbcommon
          ])
          ++ pkgs.lib.optionals mono [
            dotnet-sdk_8
            dotnet-runtime_8
          ];
      in
      {

        devShells = {
          default = pkgs.mkShell.override { stdenv = llvm.stdenv; } {
            name = "grossly godot";
            packages = with pkgs; [
              python3
              sccache
              cmake
              extra-cmake-modules
              unzip
              doxygen
            ];

            buildInputs =
              with pkgs;
              [
                pkg-config
              ]
              ++ runtimeLibs;
          };
        };
      }
    );
}
