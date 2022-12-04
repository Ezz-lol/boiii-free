libtomcrypt = {
	source = path.join(dependencies.basePath, "libtomcrypt"),
}

function libtomcrypt.import()
	links {
		"libtomcrypt"
	}

	libtomcrypt.includes()
end

function libtomcrypt.includes()
	includedirs {
		path.join(libtomcrypt.source, "src/headers")
	}

	defines {
		"LTC_NO_FAST",
		"LTC_NO_PROTOTYPES",
		"LTC_NO_RSA_BLINDING",
		"LTC_NO_FILE",
		"ARGTYPE=4",
	}
end

function libtomcrypt.project()
	project "libtomcrypt"
		language "C"

		libtomcrypt.includes()
		libtommath.import()

		files {
			path.join(libtomcrypt.source, "src/**.c"),
		}

		removefiles {
			path.join(libtomcrypt.source, "src/**/*_test.c"),
			path.join(libtomcrypt.source, "src/**/*tab.c"),
			path.join(libtomcrypt.source, "src/encauth/ocb3/**.c"),
		}

		defines {
			"_CRT_SECURE_NO_WARNINGS",
			"LTC_SOURCE",
			"_LIB",
			"USE_LTM"
		}

		removedefines {
			"_DLL",
			"_USRDLL"
		}

		linkoptions {
			"-IGNORE:4221"
		}

		warnings "Off"
		kind "StaticLib"
end

table.insert(dependencies, libtomcrypt)
