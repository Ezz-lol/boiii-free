minizip = {
	source = path.join(dependencies.basePath, "zlib/contrib/minizip"),
}

function minizip.import()
	links { "minizip" }
	zlib.import()
	minizip.includes()
end

function minizip.includes()
	includedirs {
		minizip.source
	}

	zlib.includes()
end

function minizip.project()
	project "minizip"
		language "C"

		minizip.includes()

		files {
			path.join(minizip.source, "*.h"),
			path.join(minizip.source, "*.c"),
		}

		removefiles {
			path.join(minizip.source, "miniunz.c"),
			path.join(minizip.source, "minizip.c"),
		}

		defines {
			"_CRT_SECURE_NO_DEPRECATE",
		}

		warnings "Off"
		kind "StaticLib"
end

table.insert(dependencies, minizip)
