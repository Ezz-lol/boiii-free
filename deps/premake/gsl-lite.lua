gsl_lite = {
  source = path.join(dependencies.basePath, "gsl-lite"),
}

function gsl_lite.import()
  gsl_lite.includes()
end

function gsl_lite.includes()
  includedirs({
    path.join(gsl_lite.source, "include"),
  })
end

function gsl_lite.project() end

table.insert(dependencies, gsl_lite)
