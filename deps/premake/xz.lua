xz = {
  source = path.join(dependencies.basePath, "xz"),
}

function xz.import()
  links({ "xz" })
  xz.includes()
end

function xz.includes()
  includedirs({
    path.join(xz.source, "src/liblzma/api"),
  })

  defines({ "LZMA_API_STATIC" })
end

function xz.project()
  project("xz")
  language("C")

  xz.includes()

  includedirs({
    path.join(xz.source, "src/common"),
    path.join(xz.source, "src/liblzma/common"),
    path.join(xz.source, "src/liblzma/check"),
    path.join(xz.source, "src/liblzma/lz"),
    path.join(xz.source, "src/liblzma/lzma"),
    path.join(xz.source, "src/liblzma/rangecoder"),
    path.join(xz.source, "src/liblzma/delta"),
    path.join(xz.source, "src/liblzma/simple"),
  })

  defines({
    "HAVE_STDBOOL_H",
    "HAVE__BOOL",
    "HAVE_STDINT_H",
    "HAVE_INTTYPES_H",
    "HAVE_CHECK_CRC32",
    "HAVE_DECODERS",
    "HAVE_DECODER_LZMA1",
    "HAVE_DECODER_LZMA2",
    "HAVE_VISIBILITY=0",
    "TUKLIB_SYMBOL_PREFIX=lzma_",
  })

  files({
    path.join(xz.source, "src/common/tuklib_physmem.c"),
    path.join(xz.source, "src/liblzma/check/check.c"),
    path.join(xz.source, "src/liblzma/check/crc32_fast.c"),
    path.join(xz.source, "src/liblzma/common/*.c"),
    path.join(xz.source, "src/liblzma/lz/lz_decoder.c"),
    path.join(xz.source, "src/liblzma/lzma/lzma_decoder.c"),
    path.join(xz.source, "src/liblzma/lzma/lzma2_decoder.c"),
    path.join(xz.source, "src/liblzma/lzma/lzma_encoder_presets.c"),
  })

  removefiles({
    path.join(xz.source, "src/liblzma/common/*encoder*.c"),
    path.join(xz.source, "src/liblzma/common/hardware_cputhreads.c"),
    path.join(xz.source, "src/liblzma/common/outqueue.c"),
    path.join(xz.source, "src/liblzma/common/stream_decoder_mt.c"),
    path.join(xz.source, "src/liblzma/common/lzip_decoder.c"),
    path.join(xz.source, "src/liblzma/common/microlzma_decoder.c"),
  })

  warnings("Off")
  kind("StaticLib")
end

table.insert(dependencies, xz)
