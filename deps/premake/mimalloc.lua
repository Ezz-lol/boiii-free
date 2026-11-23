mimalloc = {
  source = path.join(dependencies.basePath, "mimalloc"),
}

function mimalloc.includes()
  filter({ "configurations:Release" })
  defines({
    "MI_MALLOC_OVERRIDE",
  })
  filter({})

  includedirs({
    path.join(mimalloc.source, "include"),
  })
end

function mimalloc.import()
  -- Link the static library and force all of its objects into the link.
  -- wholearchive is the portable Premake way to do /WHOLEARCHIVE or
  -- -Wl,--whole-archive, which guarantees the malloc/free overrides
  -- are not discarded by the linker.
  links({ "mimalloc-static" })
  wholearchive({ "mimalloc-static" })

  mimalloc.includes()
end

function mimalloc.project()
  project("mimalloc-static")
  language("C")

  mimalloc.includes()

  local static_c_path = path.join(mimalloc.source, "src/static.c")
  -- Compile the single translation unit directly into the consuming
  -- project. This produces an object file that is linked before
  -- static library archives - including CRT - ensuring the overrides take effect.
  files({
    static_c_path,
  })

  -- Prevent the C++ precompiled header from being applied to this C source file
  filter({ "files:" .. static_c_path })
  enablepch("Off")
  filter({})

  warnings("Off")
  kind("StaticLib")
end

table.insert(dependencies, mimalloc)
