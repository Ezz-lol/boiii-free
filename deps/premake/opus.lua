opus = {
  root = path.join(dependencies.basePath, "opus"),
}

opus.source = path.join(opus.root, "src")

function opus.defines()
  defines({
    "OPUS_BUILD_SHARED_LIBRARY=OFF",
    "OPUS_BUILD_TESTING=OFF",
    "OPUS_BUILD_PROGRAMS=OFF",
    "OPUS_CUSTOM_MODES=OFF",
    "OPUS_STATIC_RUNTIME=ON",
    "OPUS_FORTIFY_SOURCE=ON",
    "OPUS_STACK_PROTECTOR=ON",
    "OPUS_BUILD",

    "WIN32=1",
    "_WIN32=1",
    "WIN64=1",
    "_WIN64=1",

    -- Opus features
    "OPUS_BUILD=1",
    "FLOATING_POINT=1", -- Use floating-point API
    -- MSVC supports variable length arrays via standard workarounds in opus_types.h
    "VAR_ARRAYS=1",

    -- Runtime CPU feature detection
    "CPU_INFO_BY_C=1",
    "OPUS_HAVE_RTCD=1",

    -- Default in `x86-64` march
    "OPUS_X86_PRESUME_SSE=1",
    "OPUS_X86_PRESUME_SSE2=1",

    'PACKAGE_NAME="opus"',
    'PACKAGE_VERSION="1.5.2"',
  })
end

function opus.import()
  opus.defines()
  links({ "opus" })
  opus.includes()
end

function opus.includes()
  opus.defines()
  includedirs({
    path.join(opus.root, "include"),
    path.join(opus.root, "silk"),
    path.join(opus.root, "silk/x86"),
    path.join(opus.root, "silk/float"),
    path.join(opus.root, "silk/float/x86"),
    path.join(opus.root, "celt"),
    path.join(opus.root, "celt/x86"),

    path.join(opus.root),
  })
end

function opus.project()
  project("opus")
  language("C")

  opus.includes()

  files({
    path.join(opus.source, "*.c"),
    path.join(opus.root, "silk/*.c"),
    path.join(opus.root, "silk/float/*.c"),
    path.join(opus.root, "celt/*.c"),
    path.join(opus.root, "celt/x86/*.c"),
  })

  warnings("Off")
  kind("StaticLib")
end

table.insert(dependencies, opus)
