
workspace "night time"
	architecture "x64"

	configurations
	{
		"Debug",
		"Release",
		"ReleaseDBAR",
		"Dist"
	}

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

IncludeDir = {}
IncludeDir["SDL3"] = "$(SolutionDir)/night/vendor/sdl3/include"
IncludeDir["SDL3_image"] = "$(SolutionDir)/night/vendor/SDL3_image/include"
IncludeDir["ImGui"] = "$(SolutionDir)/night/vendor/imgui"
IncludeDir["Glew"] = "$(SolutionDir)/night/vendor/glew/include"

LibDir = {}
LibDir["SDL3"] = "$(SolutionDir)/night/vendor/SDL3/lib/x64"
LibDir["SDL3_image"] = "$(SolutionDir)/night/vendor/SDL3_image/lib/x64"
LibDir["ImGui"] = "$(SolutionDir)/night/vendor/imgui/bin/Debug-windows-x86_64/ImGui"
LibDir["Glew"] = "$(SolutionDir)/night/vendor/glew/lib/Release/x64"

include "night/vendor/imgui"
-- TODO: static link glew

project "night"
	location "night"
	kind "SharedLib" -- TODO: add static library macro
	language "C++"
	cppdialect "C++20"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	pchheader "nightpch.h"
	pchsource "night/src/nightpch.cpp"

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp"
	}

	includedirs
	{
		"%{prj.name}/src",
		"%{prj.name}/vendor",
		--"%{prj.name}/vendor/spdlog/include",
		"%{prj.name}/vendor",
		"%{prj.name}/vendor/glm",
		"%{IncludeDir.SDL3}",
		"%{IncludeDir.SDL3_image}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.Glew}"
	}

	libdirs 
	{
		"%{LibDir.SDL3}",
		"%{LibDir.SDL3_image}",
		"%{LibDir.Glew}"
	}

	links
	{
		"SDL3.lib",
		"SDL3_test.lib",
		"SDL3_image.lib",
		"ImGui",
		"glew32.lib",
		"opengl32.lib"
	}

	filter "system:windows"
		cppdialect "C++20"
		--staticruntime "On"
		systemversion "latest"

		defines
		{
			"NIGHT_PLATFORM_WINDOWS",
			--"NIGHT_USE_DOUBLE_PRECISION",
			"NIGHT_BUILD_DLL"
		}

		postbuildcommands
		{
			("{COPY} %{cfg.buildtarget.relpath} ../bin/" .. outputdir .. "/sandbox")
		}

		filter "configurations:Debug"
			defines 
			{ 
				"NIGHT_DEBUG",
				"NIGHT_ENABLE_LOGGING",
				"NIGHT_ENABLE_DEBUG_RENDERER",
				"NIGHT_DB_RENDERER_ENABLE_ALGO_RENDER",
				"NIGHT_CORE"
			}
			symbols "On"

		filter "configurations:Release"
			defines 
			{
				"NIGHT_RELEASE",
				"NIGHT_ENABLE_LOGGING",
				"NIGHT_ENABLE_DEBUG_RENDERER",
				"NIGHT_CORE"
			}
			optimize "On"

		filter "configurations:ReleaseDBAR"
			defines 
			{
				"NIGHT_RELEASE",
				"NIGHT_ENABLE_LOGGING",
				"NIGHT_ENABLE_DEBUG_RENDERER",
				"NIGHT_DB_RENDERER_ENABLE_ALGO_RENDER",
				"NIGHT_CORE"
			}
			optimize "On"

		filter "configurations:Dist"
			defines 
			{
				"NIGHT_DIST",
				"NIGHT_CORE"
			}
			optimize "On"

project "sandbox"
	location "sandbox"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++20"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	pchheader "sandboxpch.h"
	pchsource "sandbox/src/sandboxpch.cpp"

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp"
	}

	includedirs
	{
		"%{prj.name}/src",
		"night/vendor/glm",
		"night/src",
		"night/vendor"
	}

	links
	{
		"night"
	}

	filter "system:windows"
		cppdialect "C++20"
		--staticruntime "On"
		systemversion "latest"

		defines
		{
			"NIGHT_PLATFORM_WINDOWS",
		}

		filter "configurations:Debug"
			defines
			{
				"NIGHT_DEBUG",
				"NIGHT_ENABLE_LOGGING",
				"NIGHT_ENABLE_DEBUG_RENDERER",
				"NIGHT_DB_RENDERER_ENABLE_ALGO_RENDER"
			}
			symbols "On"

		filter "configurations:Release"
			defines 
			{
				"NIGHT_RELEASE",
				"NIGHT_ENABLE_LOGGING",
				"NIGHT_ENABLE_DEBUG_RENDERER",
			}
			optimize "On"

		filter "configurations:ReleaseDBAR"
			defines 
			{
				"NIGHT_RELEASE",
				"NIGHT_ENABLE_LOGGING",
				"NIGHT_ENABLE_DEBUG_RENDERER",
				"NIGHT_DB_RENDERER_ENABLE_ALGO_RENDER",
			}
			optimize "On"

		filter "configurations:Dist"
			defines "NIGHT_DIST"
			optimize "On"