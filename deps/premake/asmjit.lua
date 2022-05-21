asmjit = {
	source = path.join(dependencies.basePath, "asmjit"),
}

function asmjit.import()
	links { "asmjit" }
	asmjit.includes()
end

function asmjit.includes()
	includedirs {
		path.join(asmjit.source, "src")
	}
	
	defines {
		"ASMJIT_STATIC"
	}
end

function asmjit.project()
	project "asmjit"
		language "C++"

		asmjit.includes()

		files {
			path.join(asmjit.source, "src/**.cpp"),
		}

		warnings "Off"
		kind "StaticLib"
end

table.insert(dependencies, asmjit)
