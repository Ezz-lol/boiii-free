gtl = {
  source = path.join(dependencies.basePath, "gtl"),
}

function gtl.import()
  gtl.includes()
end

function gtl.includes()
  includedirs({
    path.join(gtl.source, "include"),
  })
end

function gtl.project() end

table.insert(dependencies, gtl)
