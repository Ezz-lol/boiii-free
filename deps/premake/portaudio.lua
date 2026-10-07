portaudio = {}
portaudio.root = path.join(dependencies.basePath, "portaudio")
portaudio.source = path.join(portaudio.root, "src")
portaudio.include = path.join(portaudio.root, "include")

function portaudio.defines()
  defines({
    "WIN32=1",
    "_WIN32=1",
    "WIN64=1",
    "_WIN64=1",
    "PA_BUILD_SHARED_LIBS=OFF",
    "PA_BUILD_TESTS=OFF",
    "PA_BUILD_EXAMPLES=OFF",
    "PA_ENABLE_DEBUG_OUTPUT=OFF",
    "PA_USE_SKELETON=OFF",
    -- Windows-specific
    "PA_USE_ASIO=OFF",
    "PA_USE_DS=ON",
    "PA_USE_WMME=ON",
    "PA_USE_WASAPI=ON",
    "PA_USE_WDMKS=ON",
    "PA_USE_WDMKS_DEVICE_INFO=ON",
    -- Linux-specific
    "PA_ALSA_DYNAMIC=OFF",
  })
end

function portaudio.import()
  portaudio.defines()
  links({ "portaudio", "winmm", "ole32", "uuid" })
  portaudio.includes()
end

function portaudio.includes()
  portaudio.defines()
  includedirs({
    portaudio.include,
    path.join(portaudio.source, "os/win"),
    path.join(portaudio.source, "common"),
  })
end

function portaudio.project()
  project("portaudio")
  language("C")

  portaudio.includes()

  files({
    -- Platform independent
    path.join(portaudio.include, "portaudio.h"),
    path.join(portaudio.source, "common/pa_allocation.c"),
    path.join(portaudio.source, "common/pa_allocation.h"),
    path.join(portaudio.source, "common/pa_converters.c"),
    path.join(portaudio.source, "common/pa_converters.h"),
    path.join(portaudio.source, "common/pa_cpuload.c"),
    path.join(portaudio.source, "common/pa_cpuload.h"),
    path.join(portaudio.source, "common/pa_debugprint.c"),
    path.join(portaudio.source, "common/pa_debugprint.h"),
    path.join(portaudio.source, "common/pa_dither.c"),
    path.join(portaudio.source, "common/pa_dither.h"),
    path.join(portaudio.source, "common/pa_endianness.h"),
    path.join(portaudio.source, "common/pa_front.c"),
    path.join(portaudio.source, "common/pa_hostapi.h"),
    path.join(portaudio.source, "common/pa_process.c"),
    path.join(portaudio.source, "common/pa_process.h"),
    path.join(portaudio.source, "common/pa_ringbuffer.c"),
    path.join(portaudio.source, "common/pa_ringbuffer.h"),
    path.join(portaudio.source, "common/pa_stream.c"),
    path.join(portaudio.source, "common/pa_stream.h"),
    path.join(portaudio.source, "common/pa_trace.c"),
    path.join(portaudio.source, "common/pa_trace.h"),
    path.join(portaudio.source, "common/pa_types.h"),

    -- Windows-specific
    path.join(portaudio.source, "os/win/*.c"),
  })

  warnings("Off")
  kind("StaticLib")
end

table.insert(dependencies, portaudio)
