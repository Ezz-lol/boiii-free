frozen = {
  source = path.join(dependencies.basePath, "frozen"),
}

function frozen.import()
  frozen.includes()
end

function frozen.includes()
  includedirs({
    path.join(frozen.source, "include"),
  })
end

function frozen.project() end

table.insert(dependencies, frozen)
