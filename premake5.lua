GIT_VERSIONING_COMMAND = "git describe --tags --always"
GIT_CURRENT_BRANCH_COMMAND = "git symbolic-ref -q --short HEAD"

-- Quote the given string input as a C string
local function cstrquote(value)
  if value == nil then
    return '""'
  end
  local result = value:gsub("\\", "\\\\")
  result = result:gsub('"', '\\"')
  result = result:gsub("\n", "\\n")
  result = result:gsub("\t", "\\t")
  result = result:gsub("\r", "\\r")
  result = result:gsub("\a", "\\a")
  result = result:gsub("\b", "\\b")
  result = '"' .. result .. '"'
  return result
end

local function mergeTables(...)
  local result = {}
  for _, currentTable in ipairs({ ... }) do
    for k, v in pairs(currentTable) do
      result[k] = v
    end
  end
  return result
end

local function map(tbl, cb)
  local new_tbl = {}
  for i, v in ipairs(tbl) do
    new_tbl[i] = cb(v)
  end
  return new_tbl
end

local function clang_cl_flag(clang_flag)
  return "/clang:" .. clang_flag
end

local function clang_ld_flag(ld_flag)
  return "-Wl," .. ld_flag
end

local function split_env_flags(env_var_name)
  local value = os.getenv(env_var_name)
  local flags = {}
  if value then
    for flag in string.gmatch(value, "[^%s]+") do
      table.insert(flags, flag)
    end
  end
  return flags
end

-- Converts tags in "vX.X.X" format and given revision number Y to an array of numbers {X,X,X,Y}.
-- In the case where the format does not work fall back to padding with zeroes and just ending with the revision number.
-- partscount can be either 3 or 4.
local function vertonumarr(value, vernumber, partscount)
  local vernum = {}
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
  basePath = "./deps",
}

function dependencies.load()
  local dir = path.join(dependencies.basePath, "premake/*.lua")
  local deps = os.matchfiles(dir)

  for _, dep in pairs(deps) do
    dep = dep:gsub(".lua", "")
    include(dep)
  end
end

function dependencies.imports()
  for i, proj in pairs(dependencies) do
    if type(i) == "number" then
      proj.import()
    end
  end
end

function dependencies.projects()
  for i, proj in pairs(dependencies) do
    if type(i) == "number" then
      proj.project()
    end
  end
end

local function generate_buildinfo(wks)
  -- get old version number from version.hpp if any
  local oldVersion = "(none)"
  local oldVersionHeader = io.open(wks.location .. "/src/version.h", "r")
  if oldVersionHeader ~= nil then
    local oldVersionHeaderContent = assert(oldVersionHeader:read("*l"))
    while oldVersionHeaderContent do
      local m = string.match(oldVersionHeaderContent, "#define GIT_DESCRIBE (.+)%s*$")
      if m ~= nil then
        oldVersion = m
      end

      oldVersionHeaderContent = oldVersionHeader:read("*l")
    end
  end

  -- get current version via git
  local proc = assert(io.popen(GIT_VERSIONING_COMMAND, "r"))
  local gitDescribeOutput = assert(proc:read("*a")):gsub("%s+", "")
  proc:close()

  -- generate version.hpp with a revision number if not equal
  local gitDescribeOutputQuoted = cstrquote(gitDescribeOutput)
  if oldVersion ~= gitDescribeOutputQuoted then
    -- get current git hash and write to version.txt (used by the preliminary updater)
    -- TODO - remove once proper updater and release versioning exists
    proc = assert(io.popen("git rev-parse HEAD", "r"))
    local gitCommitHash = assert(proc:read("*a")):gsub("%s+", "")
    proc:close()

    -- get whether this is a clean revision (no uncommitted changes)
    proc = assert(io.popen("git status --porcelain", "r"))
    ---@type integer
    local revDirty = 0
    if assert(proc:read("*a")) ~= "" then
      revDirty = 1
    end
    proc:close()

    -- get current tag name
    proc = assert(io.popen("git describe --tags --abbrev=0"))
    local tagName = proc:read("*l")

    -- get current branch name
    proc = assert(io.popen("git branch --show-current"))
    local branchName = proc:read("*l")

    -- branch for ci
    if branchName == nil or branchName == "" then
      proc = assert(io.popen("git show -s --pretty=%d HEAD"))
      local branchInfo = proc:read("*l")
      local m = string.match(branchInfo, ".+,.+, ([^)]+)")
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
    local revNumber = assert(proc:read("*a")):gsub("%s+", "")

    print("Update " .. oldVersion .. " -> " .. gitDescribeOutputQuoted)

    -- write to version.txt for preliminary updater
    -- NOTE - remove this once we have a proper updater and proper release versioning
    local versionFile = assert(io.open(wks.location .. "/version.txt", "w"))
    versionFile:write(gitCommitHash)
    versionFile:close()

    local defines = {
      "GIT_DESCRIBE",
      "GIT_DIRTY",
      "GIT_HASH",
      "GIT_TAG",
      "GIT_BRANCH",
      "VERSION_PRODUCT_RC",
      "VERSION_PRODUCT",
      "VERSION_FILE",
      "VERSION",
    }

    -- write version header
    local versionHeader = assert(io.open(wks.location .. "/src/version.h", "w"))
    versionHeader:write("#pragma once\n\n")
    versionHeader:write("/*\n")
    versionHeader:write(" * Automatically generated by premake5.\n")
    versionHeader:write(" * Do not touch!\n")
    versionHeader:write(" */\n")
    versionHeader:write("\n")
    for _, def in ipairs(defines) do
      versionHeader:write("#ifdef " .. def .. "\n" .. "#undef " .. def .. "\n" .. "#endif\n")
    end
    versionHeader:write("#define GIT_DESCRIBE " .. gitDescribeOutputQuoted .. "\n")
    versionHeader:write("#define GIT_DIRTY " .. revDirty .. "\n")
    versionHeader:write("#define GIT_HASH " .. cstrquote(gitCommitHash) .. "\n")
    versionHeader:write("#define GIT_TAG " .. cstrquote(tagName) .. "\n")
    versionHeader:write("#define GIT_BRANCH " .. cstrquote(branchName) .. "\n")
    versionHeader:write("\n")
    versionHeader:write("// Version transformed for RC files\n")
    versionHeader:write("#define VERSION_PRODUCT_RC " .. table.concat(vertonumarr(tagName, revNumber, 3), ",") .. "\n")
    versionHeader:write(
      "#define VERSION_PRODUCT " .. cstrquote(table.concat(vertonumarr(tagName, revNumber, 3), ".")) .. "\n"
    )
    versionHeader:write("#define VERSION_FILE_RC " .. table.concat(vertonumarr(tagName, revNumber, 4), ",") .. "\n")
    versionHeader:write(
      "#define VERSION_FILE " .. cstrquote(table.concat(vertonumarr(tagName, revNumber, 4), ".")) .. "\n"
    )
    versionHeader:write("\n")
    versionHeader:write("// Alias definitions\n")

    versionHeader:write("#define VERSION GIT_DESCRIBE\n")
    versionHeader:write("#define SHORTVERSION VERSION_PRODUCT\n")
    versionHeader:flush()
    versionHeader:close()
    local versionHeaderHpp = assert(io.open(wks.location .. "/src/version.hpp", "w"))
    versionHeaderHpp:write("#pragma once\n\n")
    versionHeaderHpp:write("/*\n")
    versionHeaderHpp:write(" * Automatically generated by premake5.\n")
    versionHeaderHpp:write(" * Do not touch!\n")
    versionHeaderHpp:write(" *\n")
    versionHeaderHpp:write(" * This file exists for reasons of complying with our coding standards.\n")
    versionHeaderHpp:write(" *\n")
    versionHeaderHpp:write(
      " * The Resource Compiler will ignore any content from C++ header files if they're not from STDInclude.hpp.\n"
    )
    versionHeaderHpp:write(" * That's the reason why we now place all version info in version.h instead.\n")
    versionHeaderHpp:write(" */\n")
    versionHeaderHpp:write("\n")
    versionHeaderHpp:write('#include "version.h"\n')
    versionHeaderHpp:flush()
    versionHeaderHpp:close()
  end
end

newoption({
  trigger = "copy-to",
  description = "Optional, copy the EXE to a custom folder after build, define the path here if wanted.",
  value = "PATH",
})

newoption({
  trigger = "dev-build",
  description = "Enable development builds of the client.",
})

newoption({
  trigger = "no-check",
  description = "Disable ownership checks.",
})

newaction({
  trigger = "version",
  description = "Returns the version string for the current commit of the source code.",
  onWorkspace = function(_)
    -- get current version via git
    local proc = assert(io.popen(GIT_VERSIONING_COMMAND, "r"))
    local gitDescribeOutput = assert(proc:read("*a")):gsub("%s+", "")
    proc:close()
    local version = gitDescribeOutput

    proc = assert(io.popen(GIT_CURRENT_BRANCH_COMMAND, "r"))
    local gitCurrentBranchOutput = assert(proc:read("*a")):gsub("%s+", "")
    local gitCurrentBranchSuccess = proc:close()
    if gitCurrentBranchSuccess then
      -- We got a branch name, check if it is a feature branch
      if
        gitCurrentBranchOutput ~= "develop"
        and gitCurrentBranchOutput ~= "master"
        and gitCurrentBranchOutput ~= "main"
      then
        version = version .. "-" .. gitCurrentBranchOutput
      end
    end

    print(version)
    os.exit(0)
  end,
})

dependencies.load()

workspace("boiii")

filter("action:vs*")
toolset("msc-clangcl")
filter({})
filter("action:not vs*")
toolset("clang")
filter({})

linker("LLD")
startproject("client")
location("./build")
objdir("%{wks.location}/obj")
targetdir("%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}")

configurations({ "Debug", "Release" })

language("C++")
cppdialect("C++20")

architecture("x86_64")
platforms("x64")

systemversion("latest")
filter("configurations:Release")
symbols("Off")
filter({})
filter("configurations:Debug")
symbols("On")
filter({})

staticruntime("On")
editandcontinue("Off")
warnings("Extra")
characterset("ASCII")
manifest("Off")

if _OPTIONS["dev-build"] then
  defines({ "DEV_BUILD" })
end

if _OPTIONS["no-check"] then
  defines({ "NO_CHECK" })
end

if os.getenv("CI") then
  defines({ "CI" })
end

incrementallink("Off")
minimalrebuild("Off")
multiprocessorcompile("On")
enable64bitchecks("Off")

filter("platforms:x64")
defines({ "_WINDOWS", "WIN32" })
filter({})

filter("configurations:Release")
intrinsics("On")
linktimeoptimization("On")
defines({ "NDEBUG" })
-- flags({ "FatalCompileWarnings" })
filter({})

-- Read flags from the environment
local env_cflags = split_env_flags("CFLAGS")
local env_cxxflags = split_env_flags("CXXFLAGS")
local env_ldflags = split_env_flags("LDFLAGS")
local env_resflags = split_env_flags("RESFLAGS")

-- Append CFLAGS to all C/C++ source compilation args
if #env_cflags > 0 then
  buildoptions({ env_cflags })
end

-- Append CXXFLAGS to C++ source compilation args
if #env_cxxflags > 0 then
  filter("language:C++")
  buildoptions({ env_cxxflags })
  filter({})
end

-- Append LDFLAGS to linker args
if #env_ldflags > 0 then
  linkoptions({ env_ldflags })
end

-- Append RESFLAGS to resource compiler args
if #env_resflags > 0 then
  resoptions({ env_resflags })
end

generate_buildinfo(workspace())

if _OPTIONS["copy-to"] then
  local copy_recursively = "cp -r"
  if os.host() == "windows" then
    copy_recursively = "copy /y"
  end
  postbuildcommands({ copy_recursively .. ' "$(TargetPath)" "' .. _OPTIONS["copy-to"] .. '"' })
end

local archFlags = {
  "-march=x86-64",
  "-m64",
  -- Only used after positive test for support at runtime.
  "-mno-sse4.1",
  "-mno-sse4.2",
}
local parserFlags = {
  "-fbracket-depth=1024",
}

local linkerOptimizationFlags = {
  -- Link-time optimization
  "-opt:lldltocgo=3",
  "-opt:lldlto=3",

  -- Data de-duplication
  "-opt:ref", -- Removes dead code, unreferenced data

  -- General function de-duplication
  "-opt:icf=safe",
  "-opt:lldtailmerge",

  -- Symbol table and debug data removal
  "-debug:none",
}

local optimizationFlags = {
  -- General
  "-Oz",

  -- Link-time optimization
  "-flto=full",

  -- Data de-duplication
  "-fdata-sections",
  "-fmerge-all-constants",
  "-fvisibility=hidden",

  -- Virtual function de-duplication
  "-fwhole-program-vtables",
  "-fvirtual-function-elimination",
  "-fdevirtualize-speculatively",

  -- General function de-duplication
  "-ffunction-sections",

  -- Loop optimization
  "-funroll-loops",
  "-fvectorize",
  "-floop-interchange",

  -- General IR optimization
  "-flifetime-dse",
  "-fstrict-aliasing",
  "-fclangir",

  -- Function inlining optimization
  "-finline-functions",
  -- Note: this flag increases binary size by ~100KB as of writing this.
  -- Only inlines functions explicitly marked as `inline`, so this is still
  -- likely preferable.
  "-finline-hint-functions",

  -- Compile time optimization
  "-pipe",

  -- Misc. optimization
  "-fomit-frame-pointer",
  "-fcf-protection=none",

  -- Use LLD as linker
  "-fuse-ld=lld",
}

local clangLinkerOptimizationFlags = map(linkerOptimizationFlags, clang_ld_flag)

local clArchFlags = map(archFlags, clang_cl_flag)
local clParserFlags = map(parserFlags, clang_cl_flag)

filter({ "configurations:Release", "toolset:msc*" })
optimize("Size")
local warningFlags = {
  "-Wno-unused-command-line-argument",
  "-Wno-microsoft-cast",
  "-Wno-dangling-else",
  "-Wno-unused",
  "-Wno-format",
  "-Wno-sign-compare",
}
buildoptions(mergeTables(warningFlags, clArchFlags))
linkoptions({ "/IGNORE:4702", "/LTCG" })
filter({})
filter({ "configurations:Release", "toolset:not msc*" })

local clangOptimizationFlags = mergeTables(optimizationFlags, clangLinkerOptimizationFlags)
buildoptions(mergeTables(archFlags, clangOptimizationFlags))
linkoptions(clangOptimizationFlags)
filter({})

filter("configurations:Debug")
optimize("Debug")
defines({ "DEBUG", "_DEBUG", "_CRT_DEBUG" })
filter({})
filter({ "configurations:Debug", "toolset:msc*" })
buildoptions(mergeTables({
  "/MDd",
}, clParserFlags, clArchFlags))
linkoptions({
  "/DEBUG",
  "/NODEFAULTLIB:libcmt.lib",
  "/NODEFAULTLIB:libucrt.lib",
  "-l libcmtd.lib",
  "/MTd",
})
filter({})
filter({ "configurations:Debug", "toolset:not msc*" })
buildoptions(mergeTables(archFlags, {
  "-Wl,/DEBUG",
  "-Wl,/NODEFAULTLIB:libcmt.lib",
  "-Wl,/NODEFAULTLIB:libucrt.lib",
  "-l libcmtd.lib",
}))
linkoptions({
  "-fms-extensions",
  "-fms-runtime-lib=static_dbg",
  "-Wl,/DEBUG",
  "-Wl,/NODEFAULTLIB:libcmt.lib",
  "-Wl,/NODEFAULTLIB:libucrt.lib",
  "-l libcmtd.lib",
})
filter({})

project("common")
kind("StaticLib")
language("C++")

filter({ "toolset: msc*" })
buildoptions({ "-Qunused-arguments", "-Wno-dangling-else" })
filter({})
filter({ "toolset:not msc*" })
buildoptions({ "-Wno-dangling-else" })
filter({})

files({ "./src/common/**.hpp", "./src/common/**.cpp" })

includedirs({
  "./src/common",
  "./src",
  -- version.h and version.hpp headers
  "%{prj.location}/src",
})

resincludedirs({ "$(ProjectDir)src" })

dependencies.imports()

project("client")
kind("WindowedApp")
language("C++")

filter({ "toolset: msc*" })
buildoptions({ "-Qunused-arguments", "-Wno-dangling-else" })
filter({})
filter({ "toolset:not msc*" })
buildoptions({ "-Wno-dangling-else" })
filter({})

targetname("boiii")

pchheader("std_include.hpp")
pchsource("src/client/std_include.cpp")

files({
  "./src/client/**.rc",
  "./src/client/**.hpp",
  "./src/client/**.cpp",
  "./src/client/resources/**.*",
})
includedirs({
  "./deps",
  "./src/client",
  "./src/common",
  "./src",
  -- version.h and version.hpp headers
  "%{prj.location}/src",
})

resincludedirs({ "$(ProjectDir)src" })

dependson({ "tlsdll" })

links({ "common" })

dependencies.imports()
filter("toolset:msc*")
linkoptions({ "/base:0x170000000" })
filter({})
filter("toolset:not msc*")
linkoptions({ "-Wl,/base:0x170000000" })
filter({})

project("tlsdll")
kind("SharedLib")
language("C++")

filter({ "toolset: msc*" })
buildoptions({ "-Qunused-arguments", "-Wno-dangling-else" })
filter({})
filter({ "toolset:not msc*" })
buildoptions({ "-Wno-dangling-else" })
filter({})

symbols("Off")
exceptionhandling("Off")

optimize("Off")
intrinsics("Off")

runtimechecks("Off")
buffersecuritycheck("Off")
nodefaultlib("On")

-- check if CXX is cl, use /Zc:threadSafeInit- if so
-- If g++ or clang++ is used, skip this, as this is the default; they can only disable it using -fno-threadsafe-statics, but we don't want that
filter("toolset:msc*")

buildoptions({ "/Zc:threadSafeInit-" })

linkoptions({ "/NODEFAULTLIB", "/IGNORE:4210" })
filter({})

filter("toolset:not msc*")

buildoptions({ "-fno-lto" })
-- equivalents for /NODEFAULTLIB and /IGNORE:4210 for gcc/clang
linkoptions({
  "-nodefaultlibs",
  "-nostdlib",
  "-fno-lto",
})
filter({})

removebuildoptions({ "-fwhole-program-vtables", "-fvirtual-function-elimination", "-fdevirtualize-speculatively" })
removelinkoptions({
  "/LTCG",
  "-fwhole-program-vtables",
  "-fvirtual-function-elimination",
  "-fdevirtualize-speculatively",
})

files({ "./src/tlsdll/**.rc", "./src/tlsdll/**.hpp", "./src/tlsdll/**.cpp", "./src/tlsdll/resources/**.*" })

includedirs({ "%{prj.location}/src/tlsdll", "%{prj.location}/src" })

links({ "common" })

resincludedirs({ "$(ProjectDir)src" })

group("Dependencies")
dependencies.projects()
