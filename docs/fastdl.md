# FastDL

FastDL lets players download your server's mod or custom map straight from your own web server, so it doesn't need to be on the Steam Workshop.

## Server config

```cfg
set sv_wwwBaseURL "http://your-ip-or-domain:8080/"
set sv_wwwDownload "1"
```

Use an address players can reach. `127.0.0.1` only works for players on the same PC as the server.

## Make the manifest

Start the server with your mod or map loaded, then type this in the server console:

```
fastdl_manifest
```

It writes a `manifest.json` into the mod folder and the custom map folder. Run it again every time you update the files.

## Upload

Put the folders on your web server like this:

```
<sv_wwwBaseURL>/mods/<mod folder>/manifest.json
<sv_wwwBaseURL>/mods/<mod folder>/zone/...
<sv_wwwBaseURL>/usermaps/<map name>/manifest.json
<sv_wwwBaseURL>/usermaps/<map name>/zone/...
```

The folder names must match the mod and map names the server is running.

## What players see

- Missing mod or map: they're asked to download it, then asked to reconnect.
- Outdated copy: they're asked to update it from your server.
- No manifest on your web server: they get the Workshop download instead, like before.
