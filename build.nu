#!/usr/bin/env nu

def "main scons" [...args: string ] {
    let platform = if (uname | get "kernel-name") == "Darwin" { "macos"} else { "linux" };
    let custom_compiler = "sccache";

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
		"vulkan=yes"
		$"vulkan_sdk_path=($env.VULKAN_SDK)"
		# "verbose=yes"
        $"c_compiler_launcher=($custom_compiler)"
        $"cpp_compiler_launcher=($custom_compiler)"
        $"CC=($nix_cc)"
        $"CXX=($nix_cxx)"
        $"linkflags=($raw_ldflags)" # Pass directly as a space-separated string block
        $"-j(nproc)"
    ]

	scons ...$base_flags ...$args

	return
}


def "main godot" [...args: string] {
	./bin/godot.macos.editor.arm64.mono ...$args
	return
}

def "main generate_mono_glue" [] {
    let local_package_dir = "/Users/bishan_/code/Gross_Incandescence/localnugetsource"

	print "Generating mono glue . . ."
	main godot "--headless" "--generate-mono-glue" "modules/mono/glue"

	print "Generating mono assemblies . . ."
	python3 ./modules/mono/build_scripts/build_assemblies.py --godot-output-dir=./bin --godot-platform=$platform --push-nupkgs-local ($local_package_dir)
	return
}


def "main editor" [] {
	print "Building editor . .  ."

	main scons "target=editor" "tools=yes"

	main generate_mono_glue

	main scons "platform=macos" "arch=arm64" "generate_bundle=yes" "tools=yes"
	return
}


def "main export_templates" [] {
	main scons "target=template_debug"
	main scons "target=template_release"
	return
}

def main [] {
	main editor
	print

}
