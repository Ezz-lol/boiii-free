sqlite = {
  source = path.join(dependencies.basePath, "sqlite"),
}

function sqlite.import()
  links({ "sqlite" })
  sqlite.includes()
end

function sqlite.includes()
  includedirs({
    sqlite.source,
  })
end

function sqlite.project()
  project("sqlite")
  language("C")

  sqlite.includes()

  files({
    path.join(sqlite.source, "sqlite3.h"),
    path.join(sqlite.source, "sqlite3.c"),
  })

  defines({
    "SQLITE_DQS=0",
    "SQLITE_OMIT_LOAD_EXTENSION",
  })

  warnings("Off")
  kind("StaticLib")
end

table.insert(dependencies, sqlite)
