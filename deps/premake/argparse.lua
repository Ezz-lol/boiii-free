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

function argparse.project() end

table.insert(dependencies, argparse)
