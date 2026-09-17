curl = {
  source = path.join(dependencies.basePath, "curl"),
}

function curl.import()
  zstd.import()
  zlib.import()
  brotli.import()

  links({ "curl" })

  links({ "Crypt32.lib" })
  filter({})

  curl.includes()
end

function curl.includes()
  zstd.includes()
  zlib.includes()
  brotli.includes()

  includedirs({
    path.join(curl.source, "include"),
  })

  defines({
    "CURL_DISABLE_ALTSVC",
    "CURL_STRICTER",
    "CURL_STATICLIB",
    "CURL_DISABLE_DICT",
    "CURL_DISABLE_FILE",
    "CURL_DISABLE_LDAP",
    "CURL_DISABLE_LDAPS",
    "CURL_DISABLE_FTP",
    "CURL_DISABLE_GOPHER",
    "CURL_DISABLE_IMAP",
    "CURL_DISABLE_MQTT",
    "CURL_DISABLE_POP3",
    "CURL_DISABLE_RTSP",
    "CURL_DISABLE_SMTP",
    "CURL_DISABLE_SMB",
    "CURL_DISABLE_TELNET",
    "CURL_DISABLE_TFTP",
    "HAVE_LIBZ",
    "HAVE_BROTLI",
    "HAVE_ZSTD",
  })
  filter({})
end

function curl.project()
  project("curl")
  language("C")

  zstd.import()
  zlib.import()
  brotli.import()

  curl.includes()

  includedirs({
    path.join(curl.source, "lib"),
  })

  files({
    path.join(curl.source, "lib/**.c"),
    path.join(curl.source, "lib/**.h"),
  })

  defines({
    "BUILDING_LIBCURL",
  })

  defines({
    "USE_SCHANNEL",
    "USE_WINDOWS_SSPI",
    "USE_THREADS_WIN32",
    "CURL_ZSTD=ON",
    "CURL_BROTLI=ON",
  })

  filter({})

  warnings("Off")
  kind("StaticLib")
end

table.insert(dependencies, curl)
