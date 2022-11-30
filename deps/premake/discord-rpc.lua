discordrpc = {
	source = path.join(dependencies.basePath, "discord-rpc"),
}

function discordrpc.import()
	links { "discord-rpc" }
	discordrpc.includes()
end

function discordrpc.includes()
	includedirs {
		path.join(discordrpc.source, "include"),
	}
end

function discordrpc.project()
	project "discord-rpc"
		language "C++"

		discordrpc.includes()
		rapidjson.import();

		files {
			path.join(discordrpc.source, "src/*.h"),
			path.join(discordrpc.source, "src/*.cpp"),
		}
		
		removefiles {
			path.join(discordrpc.source, "src/dllmain.cpp"),
			path.join(discordrpc.source, "src/*_linux.cpp"),
			path.join(discordrpc.source, "src/*_unix.cpp"),
			path.join(discordrpc.source, "src/*_osx.cpp"),
		}

		warnings "Off"
		kind "StaticLib"
end

table.insert(dependencies, discordrpc)