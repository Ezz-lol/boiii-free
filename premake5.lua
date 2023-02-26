gitVersioningCommand = "git describe --tags --dirty --always"
gitCurrentBranchCommand = "git symbolic-ref -q --short HEAD"

-- Quote the given string input as a C string
function cstrquote(value)
	if value == nil then
		return "\"\""
	end
	result = value:gsub("\\", "\\\\")
	result = result:gsub("\"", "\\\"")
	result = result:gsub("\n", "\\n")
	result = result:gsub("\t", "\\t")
	result = result:gsub("\r", "\\r")
	result = result:gsub("\a", "\\a")
	result = result:gsub("\b", "\\b")
	result = "\"" .. result .. "\""
	return result
end

-- Converts tags in "vX.X.X" format and given revision number Y to an array of numbers {X,X,X,Y}.
-- In the case where the format does not work fall back to padding with zeroes and just ending with the revision number.
-- partscount can be either 3 or 4.
function vertonumarr(value, vernumber, partscount)
	vernum = {}
	for num in string.gmatch(value or "", "%d+") do
		if #vernum < 3 then
			table.insert(vernum, tonumber(num))
		end
	end
	while #vernum < 3 do
		table.insert(vernum, 0)
	end
	if #vernum < partscount then
		table.insert(vernum, tonumber(vernumber))
	end
	return vernum
end

dependencies = {
	basePath = "./deps"
}

function dependencies.load()
	dir = path.join(dependencies.basePath, "premake/*.lua")
	deps = os.matchfiles(dir)

	for i, dep in pairs(deps) do
		dep = dep:gsub(".lua", "")
		require(dep)
	end
end

function dependencies.imports()
	for i, proj in pairs(dependencies) do
		if type(i) == 'number' then
			proj.import()
		end
	end
end

function dependencies.projects()
	for i, proj in pairs(dependencies) do
		if type(i) == 'number' then
			proj.project()
		end
	end
end

newoption {
	trigger = "copy-to",
	description = "Optional, copy the EXE to a custom folder after build, define the path here if wanted.",
	value = "PATH"
}

newoption {
	trigger = "dev-build",
	description = "Enable development builds of the client."
}

newoption {
	trigger = "no-check",
	description = "Disable ownership checks."
}


newaction {
	trigger = "version",
	description = "Returns the version string for the current commit of the source code.",
	onWorkspace = function(wks)
		-- get current version via git
		local proc = assert(io.popen(gitVersioningCommand, "r"))
		local gitDescribeOutput = assert(proc:read('*a')):gsub("%s+", "")
		proc:close()
		local version = gitDescribeOutput

		proc = assert(io.popen(gitCurrentBranchCommand, "r"))
		local gitCurrentBranchOutput = assert(proc:read('*a')):gsub("%s+", "")
		local gitCurrentBranchSuccess = proc:close()
		if gitCurrentBranchSuccess then
			-- We got a branch name, check if it is a feature branch
			if gitCurrentBranchOutput ~= "develop" and gitCurrentBranchOutput ~= "master" and gitCurrentBranchOutput ~= "main" then
				version = version .. "-" .. gitCurrentBranchOutput
			end
		end

		print(version)
		os.exit(0)
	end
}

newaction {
	trigger = "generate-buildinfo",
	description = "Sets up build information file like version.h.",
	onWorkspace = function(wks)
		-- get old version number from version.hpp if any
		local oldVersion = "(none)"
		local oldVersionHeader = io.open(wks.location .. "/src/version.h", "r")
		if oldVersionHeader ~= nil then
			local oldVersionHeaderContent = assert(oldVersionHeader:read('*l'))
			while oldVersionHeaderContent do
				m = string.match(oldVersionHeaderContent, "#define GIT_DESCRIBE (.+)%s*$")
				if m ~= nil then
						oldVersion = m
				end

				oldVersionHeaderContent = oldVersionHeader:read('*l')
			end
		end

		-- get current version via git
		local proc = assert(io.popen(gitVersioningCommand, "r"))
		local gitDescribeOutput = assert(proc:read('*a')):gsub("%s+", "")
		proc:close()

		-- generate version.hpp with a revision number if not equal
		gitDescribeOutputQuoted = cstrquote(gitDescribeOutput)
		if oldVersion ~= gitDescribeOutputQuoted then
			-- get current git hash and write to version.txt (used by the preliminary updater)
			-- TODO - remove once proper updater and release versioning exists
			local proc = assert(io.popen("git rev-parse HEAD", "r"))
			local gitCommitHash = assert(proc:read('*a')):gsub("%s+", "")
			proc:close()

			-- get whether this is a clean revision (no uncommitted changes)
			proc = assert(io.popen("git status --porcelain", "r"))
			local revDirty = (assert(proc:read('*a')) ~= "")
			if revDirty then revDirty = 1 else revDirty = 0 end
			proc:close()

			-- get current tag name
			proc = assert(io.popen("git describe --tags --abbrev=0"))
			local tagName = proc:read('*l')

			-- get current branch name
			proc = assert(io.popen("git branch --show-current"))
			local branchName = proc:read('*l')

			-- branch for ci
			if branchName == nil or branchName == '' then
				proc = assert(io.popen("git show -s --pretty=%d HEAD"))
				local branchInfo = proc:read('*l')
				m = string.match(branchInfo, ".+,.+, ([^)]+)")
				if m ~= nil then
					branchName = m
				end
			end

			if branchName == nil then
				branchName = "develop"
			end

			print("Detected branch: " .. branchName)

			-- get revision number via git
			local proc = assert(io.popen("git rev-list --count HEAD", "r"))
			local revNumber = assert(proc:read('*a')):gsub("%s+", "")

			print ("Update " .. oldVersion .. " -> " .. gitDescribeOutputQuoted)

			-- write to version.txt for preliminary updater
			-- NOTE - remove this once we have a proper updater and proper release versioning
			local versionFile = assert(io.open(wks.location .. "/version.txt", "w"))
			versionFile:write(gitCommitHash)
			versionFile:close()

			-- write version header
			local versionHeader = assert(io.open(wks.location .. "/src/version.h", "w"))
			versionHeader:write("/*\n")
			versionHeader:write(" * Automatically generated by premake5.\n")
			versionHeader:write(" * Do not touch!\n")
			versionHeader:write(" */\n")
			versionHeader:write("\n")
			versionHeader:write("#define GIT_DESCRIBE " .. gitDescribeOutputQuoted .. "\n")
			versionHeader:write("#define GIT_DIRTY " .. revDirty .. "\n")
			versionHeader:write("#define GIT_HASH " .. cstrquote(gitCommitHash) .. "\n")
			versionHeader:write("#define GIT_TAG " .. cstrquote(tagName) .. "\n")
			versionHeader:write("#define GIT_BRANCH " .. cstrquote(branchName) .. "\n")
			versionHeader:write("\n")
			versionHeader:write("// Version transformed for RC files\n")
			versionHeader:write("#define VERSION_PRODUCT_RC " .. table.concat(vertonumarr(tagName, revNumber, 3), ",") .. "\n")
			versionHeader:write("#define VERSION_PRODUCT " .. cstrquote(table.concat(vertonumarr(tagName, revNumber, 3), ".")) .. "\n")
			versionHeader:write("#define VERSION_FILE_RC " .. table.concat(vertonumarr(tagName, revNumber, 4), ",") .. "\n")
			versionHeader:write("#define VERSION_FILE " .. cstrquote(table.concat(vertonumarr(tagName, revNumber, 4), ".")) .. "\n")
			versionHeader:write("\n")
			versionHeader:write("// Alias definitions\n")
			versionHeader:write("#define VERSION GIT_DESCRIBE\n")
			versionHeader:write("#define SHORTVERSION VERSION_PRODUCT\n")
			versionHeader:close()
			local versionHeader = assert(io.open(wks.location .. "/src/version.hpp", "w"))
			versionHeader:write("/*\n")
			versionHeader:write(" * Automatically generated by premake5.\n")
			versionHeader:write(" * Do not touch!\n")
			versionHeader:write(" *\n")
			versionHeader:write(" * This file exists for reasons of complying with our coding standards.\n")
			versionHeader:write(" *\n")
			versionHeader:write(" * The Resource Compiler will ignore any content from C++ header files if they're not from STDInclude.hpp.\n")
			versionHeader:write(" * That's the reason why we now place all version info in version.h instead.\n")
			versionHeader:write(" */\n")
			versionHeader:write("\n")
			versionHeader:write("#include \".\\version.h\"\n")
			versionHeader:close()
		end
	end
}

dependencies.load()

workspace "boiii"
	startproject "client"
	location "./build"
	objdir "%{wks.location}/obj"
	targetdir "%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}"

	configurations {"Debug", "Release"}

	language "C++"
	cppdialect "C++20"

	architecture "x86_64"
	platforms "x64"

	systemversion "latest"
	symbols "On"
	staticruntime "On"
	editandcontinue "Off"
	warnings "Extra"
	characterset "ASCII"

	if _OPTIONS["dev-build"] then
		defines {"DEV_BUILD"}
	end
	
	if _OPTIONS["no-check"] then
		defines {"NO_CHECK"}
	end

	if os.getenv("CI") then
		defines {"CI"}
	end

	flags {"NoIncrementalLink", "NoMinimalRebuild", "MultiProcessorCompile", "No64BitChecks"}

	filter "platforms:x64"
		defines {"_WINDOWS", "WIN32"}
	filter {}

	filter "configurations:Release"
		optimize "Size"
		buildoptions {"/GL"}
		linkoptions {"/IGNORE:4702", "/LTCG"}
		defines {"NDEBUG"}
		flags {"FatalCompileWarnings"}
	filter {}

	filter "configurations:Debug"
		optimize "Debug"
		defines {"DEBUG", "_DEBUG"}
	filter {}

project "common"
	kind "StaticLib"
	language "C++"

	files {"./src/common/**.hpp", "./src/common/**.cpp"}

	includedirs {"./src/common", "%{prj.location}/src"}

	resincludedirs {"$(ProjectDir)src"}

	dependencies.imports()

project "client"
	kind "WindowedApp"
	language "C++"

	targetname "boiii"

	pchheader "std_include.hpp"
	pchsource "src/client/std_include.cpp"

	files {"./src/client/**.rc", "./src/client/**.hpp", "./src/client/**.cpp", "./src/client/resources/**.*"}

	includedirs {"./src/client", "./src/common", "%{prj.location}/src"}

	resincludedirs {"$(ProjectDir)src"}

	dependson {"tlsdll"}

	links {"common"}

	prebuildcommands {"pushd %{_MAIN_SCRIPT_DIR}", "tools\\premake5 generate-buildinfo", "popd"}

	if _OPTIONS["copy-to"] then
		postbuildcommands {"copy /y \"$(TargetPath)\" \"" .. _OPTIONS["copy-to"] .. "\""}
	end

	dependencies.imports()

project "tlsdll"
	kind "SharedLib"
	language "C++"

	symbols 'Off'
	exceptionhandling "Off"

	flags {"NoRuntimeChecks", "NoBufferSecurityCheck",  "OmitDefaultLibrary"}

	buildoptions {"/Zc:threadSafeInit-"}
	linkoptions {"/NODEFAULTLIB", "/IGNORE:4210"}

	removebuildoptions {"/GL"}
	removelinkoptions {"/LTCG"}

	files {"./src/tlsdll/**.rc", "./src/tlsdll/**.hpp", "./src/tlsdll/**.cpp", "./src/tlsdll/resources/**.*"}

	includedirs {"./src/tlsdll", "%{prj.location}/src"}

	links {"common"}

	resincludedirs {"$(ProjectDir)src"}


group "Dependencies"
	dependencies.projects()
