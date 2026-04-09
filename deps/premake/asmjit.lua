asmjit = {
  source = path.join(dependencies.basePath, "asmjit"),
}

function asmjit.import()
  links({ "asmjit" })
  asmjit.includes()
end

function asmjit.includes()
  includedirs({
    path.join(asmjit.source, "src"),
  })

  defines({
    "ASMJIT_STATIC",
    "ASMJIT_NO_AARCH64",
    "ASMJIT_NO_FOREIGN",
  })
end

function asmjit.project()
  project("asmjit")
  language("C++")

  asmjit.includes()

  files({
    path.join(asmjit.source, "src/**.cpp"),
  })

  warnings("Off")
  kind("StaticLib")

  filter("toolset:msc-clangcl or toolset:clang")
    buildoptions({ "-Wno-enum-enum-conversion", "-Wno-deprecated-enum-enum-conversion" })
  filter({})
end

table.insert(dependencies, asmjit)
