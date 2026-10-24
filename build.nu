#!/usr/bin/env nu

def main [] {
    let custom_compiler = "sccache";
    let platform = "macos";
    let local_package_dir = "/Users/bishan_/code/Gross_Incandescence/localnugetsource"

    let nix_cc = $env.CC? | default "clang"
    let nix_cxx = $env.CXX? | default "clang++"
    let raw_ldflags = $env.NIX_LDFLAGS? | default ""

    let base_flags = [
        $"platform=($platform)"
        "arch=arm64"
        "module_mono_enabled=yes"
        "use_llvm=yes"
        "accesskit=no"
        "angle=no"
		# "verbose=yes"
        $"c_compiler_launcher=($custom_compiler)"
        $"cpp_compiler_launcher=($custom_compiler)"
        $"CC=($nix_cc)"
        $"CXX=($nix_cxx)"
        $"linkflags=($raw_ldflags)" # Pass directly as a space-separated string block
        $"-j(nproc)"
    ]

    print "Executing clean SCons Godot engine build sequence..."

    with-env {
        ENV: {
            PATH: $env.PATH,
            NIX_LDFLAGS: $raw_ldflags,
			CC: $nix_cc,
			CXX: $nix_cxx,
            NIX_CFLAGS_COMPILE: ($env.NIX_CFLAGS_COMPILE? | default "")
			LD_LIBRARY_PATH: $env.LD_LIBRARY_PATH,
			DYLD_LIBRARY_PATH: $env.DYLD_LIBRARY_PATH,
        }
    } {
        scons ...$base_flags target=editor

		alias godot = ./bin/godot.macos.editor.arm64.mono;

		godot --headless --generate-mono-glue modules/mono/glue

		python3 ./modules/mono/build_scripts/build_assemblies.py --godot-output-dir=./bin --godot-platform=$platform --push-nupkgs-local ($local_package_dir)

		scons platform=macos arch=arm64 generate_bundle=yes

		scons platform=macos target=template_debug arch=arm64
		scons platform=macos target=template_release arch=arm64
    }
}
