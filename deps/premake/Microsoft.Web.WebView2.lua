MicrosoftWebWebView2 = {
  source = path.join(dependencies.basePath, "Microsoft.Web.WebView2"),
}

function MicrosoftWebWebView2.import()
  MicrosoftWebWebView2.includes()

  syslibdirs({ path.join(MicrosoftWebWebView2.source, "build/native/x64") })
  libdirs({ path.join(MicrosoftWebWebView2.source, "build/native/x64") })

  links({ "WebView2LoaderStatic" })
end

function MicrosoftWebWebView2.includes()
  includedirs({
    path.join(MicrosoftWebWebView2.source, "build/native/include"),
  })
end

function MicrosoftWebWebView2.project() end

table.insert(dependencies, MicrosoftWebWebView2)
