#!/usr/bin/env nu

def main [] {
    let custom_compiler = "sccache";
    let platform = "macos";
    let local_package_dir = "/Users/bishan_/code/Gross_Incandescence/localnugetsource"

    # 1. Capture the Nix properties
    let nix_ldflags = $env.NIX_LDFLAGS? | default ""
    let nix_cc = $env.CC? | default "clang"
    let nix_cxx = $env.CXX? | default "clang++"

    # 2. Re-clean flags into an escaped list format for Python evaluation
    let sanitized_flags = ($nix_ldflags | split row " " | to json -r)

    # 3. Create an inline SCons Python command script that safely injects the flags into the running builder context
    let inline_scons_patch = $"import SCons.Script; env = SCons.Script.DefaultEnvironment\(\); flags = ($sanitized_flags); env.Append\(LINKFLAGS=flags\); env.Append\(ENV=\{'PATH': '($env.PATH)'\}\)"

    # 4. Assemble compile flags matrix
    let base_flags = [
        $"platform=($platform)"
        "arch=arm64"
        "module_mono_enabled=yes"
        "use_llvm=yes"
        "accesskit=no"
        "angle=no"
        $"custom_compiler=($custom_compiler)"
        $"CC=($nix_cc)"
        $"CXX=($nix_cxx)"
    ]

    # 5. Build Targets - passing the inline python snippet execution via -c parameter
    scons ...$base_flags target=editor -j(nproc) -c ($inline_scons_patch)
    scons ...$base_flags target=template_debug -j(nproc) -c ($inline_scons_patch)
    scons ...$base_flags target=template_release -j(nproc) -c ($inline_scons_patch)

    ./bin/godot.macos.editor.arm64.llvm.mono --headless --generate-mono-glue modules/mono/glue

    python3 ./modules/mono/build_scripts/build_assemblies.py --godot-output-dir=./bin --godot-platform=macos --push-nupkgs-local ($local_package_dir)
}
