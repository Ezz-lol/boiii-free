argparse = {
  source = path.join(dependencies.basePath, "argparse"),
}

function argparse.import()
  argparse.includes()
end

function argparse.includes()
  includedirs({
    path.join(argparse.source, "include"),
  })
end

function argparse.project()
  project("argparse")
  language("C++")

  argparse.includes()

  files({
    path.join(argparse.source, "include/**.hpp"),
  })

  warnings("Off")
  kind("StaticLib")
end

table.insert(dependencies, argparse)
