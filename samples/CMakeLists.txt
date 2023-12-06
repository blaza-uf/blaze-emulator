cmake_minimum_required(VERSION 3.20)

project(blaze-emulator-samples)

find_program(ASAR asar)

function(add_rom_sample name source)
	cmake_path(ABSOLUTE_PATH source NORMALIZE)

	set(output "${CMAKE_CURRENT_BINARY_DIR}/${name}.sfc")

	add_custom_command(
		OUTPUT "${output}"
		COMMAND "${CMAKE_COMMAND}" -E rm -f -- "${output}"
		COMMAND "${ASAR}" --fix-checksum=on "${source}" "${output}"
		DEPENDS "${source}"
	)

	add_custom_target("${name}" ALL DEPENDS "${output}")
endfunction()

add_rom_sample(hello-world hello-world/hello-world.asm)
add_rom_sample(hello-interrupt hello-interrupt/hello-interrupt.asm)
