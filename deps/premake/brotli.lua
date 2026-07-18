brotli = {
  source = path.join(dependencies.basePath, "brotli", "c"),
}

function brotli.import()
  links({ "brotli" })
  brotli.includes()
end

function brotli.includes()
  includedirs({
    path.join(brotli.source, "include"),
  })
end

function brotli.project()
  project("brotli")
  language("C")

  brotli.includes()

  files({
    path.join(brotli.source, "common/**.h"),
    path.join(brotli.source, "dec/**.h"),
    path.join(brotli.source, "enc/**.h"),
    path.join(brotli.source, "include/**.h"),
    path.join(brotli.source, "common/**.c"),
    path.join(brotli.source, "dec/**.c"),
    path.join(brotli.source, "enc/**.c"),
    path.join(brotli.source, "include/**.c"),
  })

  warnings("Off")
  kind("StaticLib")
end

table.insert(dependencies, brotli)
