src_ta = ['main.c', '../common.c', 'object.c', 'key.c']
ta_uuid = '8189b937-e77d-417f-9114-9dda34f8683a'

# Write uuid to user_ta_header_defines.h
# cfgdata = configuration_data()
# cfgdata.set_quoted ('UUID', ta_uuid)
# configure_file (input: 'user_ta_header_defines.h.in', output: 'user_ta_header_defines.h', configuration: cfgdata)

tdk_path = get_option ('ta-dev-kit')
tdk_include_path = join_paths (tdk_path, 'include')
tdk_src_path = join_paths (tdk_path, 'src')
tdk_lib_path = join_paths (tdk_path, 'lib')
ta_sign_script = join_paths (tdk_path, 'scripts', 'sign.py')
ta_sign_key = join_paths (tdk_path, 'keys', 'default_ta.pem')
ta_install_path = join_paths (get_option ('libdir'), 'optee_armtz')
ta_bin_name = '@0@.ta'.format (ta_uuid)

# config.h
cfgdata = configuration_data()
cfgdata.set ('CONFIG_TA', true)
configure_file (output: 'config.h', configuration: cfgdata)

cc = meson.get_compiler ('c')
dep_utee = cc.find_library ('utee', dirs: tdk_lib_path)
dep_utils = cc.find_library ('utils', dirs: tdk_lib_path)
dep_mbedtls = cc.find_library ('mbedtls', dirs: tdk_lib_path)
dep_mpa = cc.find_library ('mpa', dirs: tdk_lib_path, required: false)

ta_arch = get_option ('ta-arch')
if ta_arch == 'arm64'
	ld_opts = '-DARM64'
elif ta_arch == 'arm32'
	ld_opts = '-DARM32'
endif

# Copy ta.ld.S and user_ta_header.h into your build directory.
link_script = configure_file (input: join_paths (tdk_src_path, 'ta.ld.S'), output: 'ta.ld.S', copy: true)
src_ta += configure_file (input: join_paths (tdk_src_path, 'user_ta_header.c'), output: 'user_ta_header.c', copy: true)

# Call c preprocessor
cc = find_program ('c')
gen_ld = custom_target ('gen-ld', input: link_script, output: 'ta.ld', command: [cc, '-E', '-Wp,-P', '-DASM', '-nostdinc', '-DMBEDTLS_SELF_TEST', '-D__LP64__', ld_opts, '@INPUT@', '-o@OUTPUT@'])

bin = executable (
		ta_uuid,
		src_ta,
		name_suffix: 'elf',
		c_args: ['-fPIC'],
		link_depends: gen_ld,
		dependencies: [dep_utee, dep_mbedtls, dep_utils, dep_mpa],
		link_args: [
				'-T', gen_ld.full_path(),
				'-e__ta_entry',
				'-nostdlib',
				'-s',
				'-z',
				'max-page-size=4096',
				'-Wl,--sort-section=alignment'
			],
		pie: true,
		include_directories: include_directories (tdk_include_path)
	)

# -Wp,-P,-MT,out/arm-plat-vexpress/ta/avb/ta.lds,-MD,out/arm-plat-vexpress/ta/avb/.ta.ld.d -DASM=1 -nostdinc -isystem /usr/lib/gcc/aarch64-linux-gnu/9.2.0/include -Ita/avb/include -Ita/avb/. -Iout/arm-plat-vexpress/ta/avb -DARM64=1 -D__LP64__=1 -DMBEDTLS_SELF_TEST -include out/arm-plat-vexpress/include/generated/conf.h -DTRACE_LEVEL=1 -Iout/arm-plat-vexpress/export-ta_arm64/include out/arm-plat-vexpress/export-ta_arm64/src/ta.ld.S > out/arm-plat-vexpress/ta/avb/ta.lds

# aarch64-linux-gnu-objcopy --rename-section .rodata=.rodata.out/arm-plat-vexpress/export-ta_arm64/src/user_ta_header.c --rename-section .rodata.str1.1=.rodata.str1.1.out/arm-plat-vexpress/export-ta_arm64/src/user_ta_header.c out/arm-plat-vexpress/ta/avb/user_ta_header.o
#aarch64-linux-gnu-objcopy --rename-section .rodata=.rodata.ta/avb/entry.c --rename-section .rodata.str1.1=.rodata.str1.1.ta/avb/entry.c out/arm-plat-vexpress/ta/avb/entry.o
# Sign the binary and install it

sign = custom_target (
		'sign',
		input: bin,
		output: ta_bin_name,
		command: [
				ta_sign_script,
				'--in', '@INPUT@',
				'--out', '@OUTPUT@',
				'--key', ta_sign_key,
				'--uuid', ta_uuid
			],
		install: true,
		install_dir: ta_install_path,
		build_by_default: true,
		install_mode: 'r--r--r--'
	)
