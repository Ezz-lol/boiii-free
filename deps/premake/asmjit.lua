asmjit = {
  source = path.join(dependencies.basePath, "asmjit"),
}

function asmjit.import()
  links({ "asmjit" })
  asmjit.includes()
end

function asmjit.includes()
  includedirs({
    asmjit.source,
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
    path.join(asmjit.source, "asmjit/**.cpp"),
  })

  warnings("Off")
  kind("StaticLib")

  filter("toolset:msc-clangcl or toolset:clang")
  buildoptions({ "-Wno-enum-enum-conversion", "-Wno-deprecated-enum-enum-conversion" })
  filter({})
end

table.insert(dependencies, asmjit)
