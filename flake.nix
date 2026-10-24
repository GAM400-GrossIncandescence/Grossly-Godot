{
  description = "A Nix-flake-based C/C++ development environment";

  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
    sccache = {
      url = "github:mozilla/sccache";
      inputs.nixpkgs.follows = "nixpkgs";
    };
  };

  outputs =
    {
      self,
      nixpkgs,
      flake-utils,
      sccache,
      ...
    }:
    flake-utils.lib.eachDefaultSystem (
      system:
      let
        pkgs = import nixpkgs {
          inherit system;
          overlays = [ sccache.overlays.default ];
        };
        llvm = pkgs.llvmPackages_22;

        moltenvk = pkgs.moltenvk;

        libPath = with pkgs; [
          dotnet-sdk_8
          dotnet-runtime_8
          dotnetPackages.Nuget
          libffi

          vulkan-headers
          vulkan-loader
          vulkan-validation-layers
          vulkan-tools

          moltenvk
          moltenvk.dev

          freetype
          openssl
          fontconfig.lib
          libxkbcommon
          zlib
          zlib.dev
        ];
      in
      {
        devShells = {
          default = pkgs.mkShell.override { stdenv = llvm.stdenv; } {
            name = "grossly godot";
            packages = libPath;

            nativeBuildInputs =
              with pkgs;
              [
                pkgs.sccache
                pkg-config
                python3
                unzip
                scons
                llvm.llvm
                llvm.clang
                llvm.lld
                llvm.libcxxClang
                llvm.clangUseLLVM
              ]
              ++ libPath;

            buildInputs = libPath ++ [
              pkgs.zlib
              pkgs.yasm
              moltenvk
              moltenvk.dev
            ];

            env = rec {
              LD_LIBRARY_PATH = "${pkgs.lib.makeLibraryPath libPath}";
              DYLD_LIBRARY_PATH = LD_LIBRARY_PATH;
              VULKAN_SDK = "${pkgs.vulkan-loader}";
              VK_LAYER_PATH = "${pkgs.vulkan-validation-layers}/share/vulkan/explicit_layer.d";
              VK_ICD_FILENAMES = "${pkgs.moltenvk}/share/vulkan/icd.d/vk_icd_moltenvk.json";
            };

            shellHook = ''
              export NIX_LDFLAGS="-L${pkgs.zlib}/lib $NIX_LDFLAGS"
              export NIX_CFLAGS_COMPILE="-I${pkgs.zlib.dev}/include -isysroot ${pkgs.apple-sdk.sdkroot} $NIX_CFLAGS_COMPILE"
            '';
          };
        };
      }
    );
}
