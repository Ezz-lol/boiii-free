imgui = {
	source = path.join(dependencies.basePath, "imgui"),
}

function imgui.import()
	links { "imgui" }
	imgui.includes()
end

function imgui.includes()
	includedirs {
		path.join(imgui.source),
		path.join(imgui.source, "backends"),
	}
end

function imgui.project()
	project "imgui"
		language "C++"
		kind "StaticLib"

		imgui.includes()

		files {
			path.join(imgui.source, "imgui.cpp"),
			path.join(imgui.source, "imgui_draw.cpp"),
			path.join(imgui.source, "imgui_tables.cpp"),
			path.join(imgui.source, "imgui_widgets.cpp"),
			path.join(imgui.source, "backends/imgui_impl_dx11.cpp"),
			path.join(imgui.source, "backends/imgui_impl_win32.cpp"),
		}

		defines { "IMGUI_IMPL_WIN32_DISABLE_GAMEPAD" }

		warnings "Off"
end

table.insert(dependencies, imgui)
