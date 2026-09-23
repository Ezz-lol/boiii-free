hksc = {
  source = path.join(dependencies.basePath, "hksc/src"),
  config = path.join(dependencies.basePath, "hksc-config"),
}

function hksc.import()
  links({ "hksc" })
  hksc.includes()
end

function hksc.includes()
  includedirs({
    hksc.config,
    hksc.source,
  })
end

function hksc.project()
  project("hksc")
  language("C")

  hksc.includes()

  files({
    path.join(hksc.source, "*.c"),
    path.join(hksc.source, "*.h"),
  })

  removefiles({
    path.join(hksc.source, "hksc.c"),
    path.join(hksc.source, "hkscaux.c"),
    path.join(hksc.source, "hkscfile.c"),
    path.join(hksc.source, "hkscmalloc.c"),
    path.join(hksc.source, "hksctest.c"),
  })

  defines({ "_CRT_SECURE_NO_WARNINGS" })

  warnings("Off")
  kind("StaticLib")
end

table.insert(dependencies, hksc)
