cmake_minimum_required(VERSION 3.10)

set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake"
	CACHE STRING "Vcpkg toolchain file")

project(AdaptiveResolutionISMs VERSION 1.0)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_COMPILE_WARNING_AS_ERROR ON)

# Only generate Debug and Release configurations
set(CMAKE_CONFIGURATION_TYPES "Debug;RelWithDebInfo;Release" CACHE STRING "" FORCE)

# Compiler options
function(compile_options target)
	if(MSVC)
		target_compile_options(${target} PRIVATE
			/W3            # Warning level
			/MP            # Multi-processor compilation
			/WX            # Warnings as errors

			$<$<CONFIG:Debug>:
				/Od        # No optimisation
				/Zi        # Debug info (PDB)
			>

			$<$<CONFIG:Release>:
				/O2        # Full optimisation
			>

			$<$<CONFIG:RelWithDebInfo>:
				/O2        # Full optimisation
				/Zi        # Debug info (PDB)
			>
		)

		target_link_options(${target} PRIVATE
			$<$<CONFIG:Debug>:
				/DEBUG
			>

			# optional but useful for crash reports
			$<$<CONFIG:Release>:
				/DEBUG
			>

			$<$<CONFIG:RelWithDebInfo>:
				/DEBUG
			>
		)
	else()
		target_compile_options(${target} PRIVATE
			-Wall
			-Wextra
			-Wpedantic
		)
	endif()
endfunction()

add_subdirectory(Core)
add_subdirectory(App)
add_subdirectory(PointCloudConverter)
add_subdirectory(RMAECalculator)

# Enable solution folders
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Visual Studio startup project
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
             PROPERTY VS_STARTUP_PROJECT App)
