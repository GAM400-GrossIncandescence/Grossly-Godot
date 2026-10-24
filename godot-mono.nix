{
  lib,
  stdenv,
  pkg-config,
  python3,
  scons,
  dotnet-sdk_8,
  dotnet-runtime_8,
  zlib,
  libffi,
  openssl,
  freetype,
  fontconfig,
  libxkbcommon,
  vulkan-loader,
  vulkan-headers,
  moltenvk,
  sccache,
  llvmPackages_22,
}:

let
  pname = "godot-mono";
  version = "custom";

  # Combine Python build dependencies
  pythonEnv = python3.withPackages (ps: with ps; [ zstandard ]);
in
llvmPackages_22.stdenv.mkDerivation {
  inherit pname version;

  src = ./.; # Assumes this file is placed inside the Godot source tree root

  nativeBuildInputs = [
    pkg-config
    pythonEnv
    scons
    dotnet-sdk_8
    dotnet-runtime_8
    sccache
  ];

  buildInputs = [
    zlib
    libffi
    openssl
    freetype
    fontconfig.lib
    libxkbcommon
    moltenvk
    moltenvk.dev
    vulkan-loader
    vulkan-headers
  ];

  # Prevent SCons from stripping critical Nix store build context maps
  inherit (stdenv) CC CXX;

  # Tell the .NET pipeline where to safely read/write local packages during build isolation
  DOTNET_CLI_HOME = "$TMPDIR/dotnet";
  NUGET_PACKAGES = "$TMPDIR/nuget";

  configurePhase = ''
    runHook preConfigure
    export HOME=$TMPDIR
    mkdir -p $DOTNET_CLI_HOME $NUGET_PACKAGES
    runHook postConfigure
  '';

  buildPhase = ''
    runHook preBuild

    # Determine CPU core layout safely inside the sandbox context
    declare -r parallel_jobs=$NIX_BUILD_CORES

    # Common SCons compile matrix configuration array definitions
    scons_args=(
      "platform=macos"
      "arch=arm64"
      "module_mono_enabled=yes"
      "use_llvm=yes"
      "accesskit=no"
      "angle=no"
      "custom_compiler=sccache"
      "-j$parallel_jobs"
    )

    echo "=== Phase 1: Compiling Editor Binary ==="
    scons "''${scons_args[@]}" target=editor

    echo "=== Phase 2: Generating Mono/C# Engine Glue Structures ==="
    ./bin/godot.macos.editor.arm64.llvm.mono --headless --generate-mono-glue modules/mono/glue

    echo "=== Phase 3: Compiling Export Templates Debug Matrix ==="
    scons "''${scons_args[@]}" target=template_debug

    echo "=== Phase 4: Compiling Export Templates Release Matrix ==="
    scons "''${scons_args[@]}" target=template_release

    echo "=== Phase 5: Processing Managed .NET Engine Assemblies ==="
    python3 ./modules/mono/build_scripts/build_assemblies.py \
      --godot-output-dir=./bin \
      --godot-platform=macos \
      --push-nupkgs-local=$TMPDIR/localnugetsource

    runHook postBuild
  '';

  installPhase = ''
    runHook preInstall

    # Setup isolated install hierarchy structures
    mkdir -p $out/Applications/Godot.app/Contents/{MacOS,Resources,Frameworks}

    # Extract default macOS template definitions bundled with Godot source repository
    cp -r misc/dist/macos_tools.app/* $out/Applications/Godot.app/

    # Copy Compiled Native Engine Core Binary and Managed Nuget Assembly Elements
    cp -r bin/* $out/Applications/Godot.app/Contents/MacOS/

    # Rename target entrypoint matching your bundle descriptor configurations
    mv $out/Applications/Godot.app/Contents/MacOS/godot.macos.editor.arm64.llvm.mono \
       $out/Applications/Godot.app/Contents/MacOS/Godot

    # Ensure dynamic MoltenVK link libraries map out accurately inside your framework layer
    cp ${moltenvk}/lib/libMoltenVK.dylib $out/Applications/Godot.app/Contents/Frameworks/

    # Symlink a quick-launch binary directly to standard target profile execution boundaries
    mkdir -p $out/bin
    ln -s $out/Applications/Godot.app/Contents/MacOS/Godot $out/bin/godot-mono

    runHook postInstall
  '';

  meta = with lib; {
    description = "Godot Engine with advanced C# Mono framework compilation targets for macOS";
    homepage = "https://godotengine.org";
    license = licenses.mit;
    platforms = [ "aarch64-darwin" ];
    maintainers = [ ];
  };
}
