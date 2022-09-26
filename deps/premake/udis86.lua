udis86 = {
	source = path.join(dependencies.basePath, "udis86"),
}

function udis86.import()
	links {
		"udis86"
	}

	udis86.includes()
end

function udis86.includes()
	includedirs {
		udis86.source,
		path.join(udis86.source, "libudis86"),
		path.join(dependencies.basePath, "extra/udis86"),
		path.join(dependencies.basePath, "extra/udis86/libudis86"),
	}
end

function udis86.project()
	project "udis86"
		language "C"

		udis86.includes()

		files {
			path.join(udis86.source, "libudis86/*.c"),
			path.join(dependencies.basePath, "extra/udis86/libudis86/*.c"),
		}

		warnings "Off"
		kind "StaticLib"
end

table.insert(dependencies, udis86)
