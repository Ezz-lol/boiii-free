sqlite_orm = {
  source = path.join(dependencies.basePath, "sqlite_orm"),
}

function sqlite_orm.import()
  sqlite_orm.includes()
end

function sqlite_orm.includes()
  includedirs({
    path.join(sqlite_orm.source, "include"),
  })
end

function sqlite_orm.project() end

table.insert(dependencies, sqlite_orm)
