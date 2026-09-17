SteamworkSDK = {
  source = path.join(dependencies.basePath, "SteamworkSDK"),
}

function SteamworkSDK.import()
  SteamworkSDK.includes()
end

function SteamworkSDK.includes()
  includedirs({
    path.join(SteamworkSDK.source, "public"),
  })
end

function SteamworkSDK.project() end

table.insert(dependencies, SteamworkSDK)
