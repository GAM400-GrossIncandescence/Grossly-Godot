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

        libPath = with pkgs; [
          dotnet-sdk_8
          dotnet-runtime_8
          libffi
          spirv-tools
          vulkan-volk
          vulkan-tools
          vulkan-loader
          vulkan-headers
          vulkan-validation-layers
          freetype
          openssl
          fontconfig.lib
          libxkbcommon
          darwin.moltenvk
          darwin.moltenvk.dev
          zlib
          zlib.dev
        ];
      in
      {
        packages.default = pkgs.callPackage ./godot-mono.nix { };

        devShells = {
          default = pkgs.mkShell.override { stdenv = llvm.stdenv; } {
            name = "grossly godot";
            packages = with pkgs; [
              pkgs.sccache
              python3
              unzip
              scons
              llvm.llvm
              llvm.clang
              llvm.lld
              llvm.libcxxClang
              llvm.clangUseLLVM
            ];

            buildInputs =
              with pkgs;
              [
                pkg-config
              ]
              ++ libPath
              # Modern unified SDK implementation
              ++ pkgs.lib.optionals pkgs.stdenv.hostPlatform.isDarwin [
                pkgs.apple-sdk
              ];

            env = {
              LD_LIBRARY_PATH = "${pkgs.lib.makeLibraryPath libPath}";
              DYLD_LIBRARY_PATH = "${pkgs.lib.makeLibraryPath libPath}";
              VK_LAYER_PATH = "${pkgs.vulkan-validation-layers}/share/vulkan/explicit_layer.d";
              VULKAN_SDK = "${pkgs.vulkan-validation-layers}/share/vulkan/explicit_layer.d";
            };

            shellHook = ''
              # export NIX_LDFLAGS="-L${pkgs.zlib}/lib $NIX_LDFLAGS"
              export NIX_CFLAGS_COMPILE="-I${pkgs.zlib.dev}/include -isysroot ${pkgs.apple-sdk.sdkroot} $NIX_CFLAGS_COMPILE"
            '';
          };
        };
      }
    );
}
