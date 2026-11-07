{
  description = "Godot development shell for Apple Silicon macOS and Linux x86_64";

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
    flake-utils.lib.eachSystem
      [
        "aarch64-darwin"
        "x86_64-linux"
      ]
      (
        system:
        let
          pkgs = import nixpkgs {
            inherit system;
          };

          lib = pkgs.lib;

          isDarwin = pkgs.stdenv.hostPlatform.isDarwin;
          isLinux = pkgs.stdenv.hostPlatform.isLinux;

          llvm = pkgs.llvmPackages_22;

          moltenvkStatic = pkgs.moltenvk.override {
            enableStatic = true;
          };

          moltenvkDynamic = pkgs.moltenvk;

          moltenvkSdk = pkgs.runCommand "godot-moltenvk-sdk" { } ''
            mkdir -p $out/macOS/lib

            # Dynamic MoltenVK library
            ln -s ${moltenvkDynamic}/lib/libMoltenVK.dylib \
              $out/macOS/lib/libMoltenVK.dylib

            # Static archive retained for Godot SDK detection
            SDK="$out/macOS/lib/MoltenVK.xcframework/macos-arm64_x86_64"
            mkdir -p "$SDK"

            ln -s ${moltenvkStatic}/lib/libMoltenVK.a \
              "$SDK/libMoltenVK.a"
          '';

          commonPackages = with pkgs; [
            # Toolchain
            llvm.clang
            llvm.llvm
            llvm.lld

            # Build tools
            python3
            scons
            pkg-config

            # Cache
            sccache

            # Libraries
            freetype
            fontconfig
            openssl
            zlib
            libffi

            # Vulkan
            vulkan-headers
            vulkan-loader
            vulkan-tools
            vulkan-validation-layers

            # Utilities
            curl
            wget
            unzip
            zip
          ];

          darwinPackages = with pkgs; [
            # macOS SDK
            apple-sdk

            # Vulkan implementation
            moltenvkSdk
          ];

          linuxPackages = with pkgs; [
            # X11
            libx11
            libxcursor
            libxext
            libxfixes
            libxi
            libxinerama
            libxrandr
            libxrender

            # Wayland
            wayland
            wayland-protocols
            libxkbcommon

            # Graphics
            mesa
            libGL

            # Audio
            alsa-lib
            libpulseaudio

            # Device management
            systemd

            # Desktop integration
            dbus
            speechd
          ];

          platformPackages =
            commonPackages ++ lib.optionals isDarwin darwinPackages ++ lib.optionals isLinux linuxPackages;

          linuxLibraryPath = lib.makeLibraryPath (
            with pkgs;
            [
              freetype
              fontconfig
              openssl
              zlib
              libffi
              vulkan-loader
              mesa
              libGL
              libx11
              libxcursor
              libxinerama
              libxi
              libxrandr
              libxkbcommon
              wayland
              alsa-lib
              libpulseaudio
              systemd
            ]
          );

          platformEnv = {
            CC = "${llvm.clang}/bin/clang";
            CXX = "${llvm.clang}/bin/clang++";
          }
          // (lib.optionalAttrs isLinux {

            LD_LIBRARY_PATH = linuxLibraryPath;

            VK_LAYER_PATH = "${pkgs.vulkan-validation-layers}/share/vulkan/explicit_layer.d";

            NIX_LDFLAGS = "-L${pkgs.zlib}/lib";
            NIX_CFLAGS_COMPILE = "-I${pkgs.zlib.dev}/include";
          })
          // (lib.optionalAttrs isDarwin {
            MACOSX_DEPLOYMENT_TARGET = "14.0";

            DYLD_FALLBACK_LIBRARY_PATH = lib.makeLibraryPath (
              with pkgs;
              [
                freetype
                fontconfig
                openssl
                zlib
                libffi
                moltenvk
              ]
            );

            VULKAN_SDK_PATH = "${moltenvkSdk}";
          });

        in
        {
          devShells.default = pkgs.mkShell (
            {
              name = "godot-development";

              packages = platformPackages;
            }
            // platformEnv
          );
        }
      );
}
