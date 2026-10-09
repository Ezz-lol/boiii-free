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
    "PA_BUILD_SHARED_LIBS=0",
    "PA_BUILD_TESTS=0",
    "PA_BUILD_EXAMPLES=0",
    "PA_ENABLE_DEBUG_OUTPUT=0",
    "PA_USE_SKELET1=0",
    -- Windows-specific
    "PA_USE_ASIO=0",
    "PA_USE_DS=1",
    "PAWIN_USE_DIRECTSOUNDFULLDUPLEXCREATE=1",
    "PA_USE_WMME=1",
    "PA_USE_WASAPI=1",
    "PA_USE_WDMKS=1",
    "PAWIN_USE_WDMKS_DEVICE_INFO=1",
    "PA_USE_WDMKS_DEVICE_INFO=1",
    -- Linux-specific
    "PA_ALSA_DYNAMIC=0",
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

    -- DirectSound
    path.join(portaudio.source, "hostapi/dsound/**.c"),
    -- WASAPI
    path.join(portaudio.source, "hostapi/wasapi/**.c"),
    -- WDMKS
    path.join(portaudio.source, "hostapi/wdmks/**.c"),
    -- WMME
    path.join(portaudio.source, "hostapi/wmme/**.c"),
  })

  warnings("Off")
  kind("StaticLib")
end

table.insert(dependencies, portaudio)
