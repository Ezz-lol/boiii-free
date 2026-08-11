gtl = {
  source = path.join(dependencies.basePath, "gtl"),
}

function gtl.import()
  links({ "gtl" })
  gtl.includes()
end

function gtl.includes()
  includedirs({
    path.join(gtl.source, "include"),
  })
end

function gtl.project()
  project("gtl")
  kind("StaticLib")
  language("C++")
  cppdialect("C++20")

  gtl.includes()

  files({
    path.join(gtl.source, "include/**.hpp"),
  })

  warnings("Off")
end

table.insert(dependencies, gtl)
