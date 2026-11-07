#!/usr/bin/env nu


def isDarwin [] {
	return ((uname).kernel-name == "Darwin")
}

def isLinux [] {
	return (not isDarwin);
}

def "main scons" [...args: string ] {
    const launcher = "sccache";
	const dev_build = "yes";

	let platform = if (isDarwin) {"macos"} else { "linuxbsd" };
	let arch = if (isDarwin) { "arm64" } else { "x86_64" };

    # let raw_ldflags = $env.NIX_LDFLAGS? | default ""

    let base_flags = [
		# Platform
        $"platform=($platform)"
        $"arch=(arch)"

		# Dotnet
        "module_mono_enabled=yes"

		# Compiler
        $"-j(nproc)"
        "use_llvm=yes"
        $"c_compiler_launcher=($launcher)"
        $"cpp_compiler_launcher=($launcher)"
        $"CC=($env.CC? | default "clang")"
        $"CXX=($env.CXX? | default "clang++")"
        # $"linkflags=($raw_ldflags)" # Pass directly as a space-separated string block

		# Graphics
		$"vulkan=yes"
		$"vulkan_sdk_path=($env.VULKAN_SDK_PATH)"
		$"dev_build=($dev_build)"
		$"use_volk=no"
        "angle=no"

		# Optional Dependencies
        "accesskit=no"
    ]

	print "scons" ($base_flags ++ $args)
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
