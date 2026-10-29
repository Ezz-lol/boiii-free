stb_image_resize = {
  source = path.join(dependencies.basePath, "stb_image_resize"),
}

function stb_image_resize.import()
  stb_image_resize.includes()
end

function stb_image_resize.includes()
  includedirs({
    stb_image_resize.source,
  })
end

function stb_image_resize.project() end

table.insert(dependencies, stb_image_resize)
