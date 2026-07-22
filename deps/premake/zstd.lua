zstd = {
  source = path.join(dependencies.basePath, "zstd", "lib"),
}

function zstd.import()
  links({ "zstd" })
  zstd.includes()
end

function zstd.includes()
  includedirs({
    zstd.source,
  })
end

function zstd.project()
  project("zstd")
  language("C")

  zstd.includes()

  defines({ "DYNAMIC_BMI2=0", "ZSTD_BUILD_STATIC=ON", "STATIC_BMI2=0" })

  files({
    path.join(zstd.source, "**.h"),
    path.join(zstd.source, "**.c"),
  })

  warnings("Off")
  kind("StaticLib")
end

table.insert(dependencies, zstd)
