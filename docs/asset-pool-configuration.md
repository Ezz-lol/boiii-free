# Asset Pool Configuration

An asset pool size configuration can be loaded from a loading mod or usermap.
This will be loaded from the first discovered file with name "assetlimits.txt"
or "assetpools.txt" in the item's "zone" tree.

Each pool's configuration will be loaded and applied if the specified, new
allocation count exceeds what is currently allocated.

## Syntax

Individual pool configurations must be separated by a new line (`\n`).

Each line contains the following space-delimited parameters.

- `XAssetType` contained in the pool, encoded as an unsigned integer. Both
  decimal and hex integers are supported.
- New number of allocated elements in asset pool, encoded as an unsigned
  integer. Both decimal and hex integers are supported.

## T7Recharged Syntax

The T7Recharged pool configuration syntax is also supported.

Individual pool configurations must be separated by a new line (`\n`).

Each line contains the following space-delimited parameters.

- `XAssetType` contained in the pool, encoded as an unsigned integer. Both
  decimal and hex integers are supported.
- Asset pool name, serialized as a string. Unused.
- Default number of allocated elements in asset pool. Unused.
- New number of allocated elements in asset pool, encoded as an unsigned
  integer. Both decimal and hex integers are supported.

## XAssetType

The following is the complete list of supported `XAssetType` enumerations and
the corresponding hex representation of each.

- `PHYSPRESET`: 0x0
- `PHYSCONSTRAINTS`: 0x1
- `DESTRUCTIBLEDEF`: 0x2
- `XANIMPARTS`: 0x3
- `XMODEL`: 0x4
- `XMODELMESH`: 0x5
- `MATERIAL`: 0x6
- `COMPUTE_SHADER_SET`: 0x7
- `TECHNIQUE_SET`: 0x8
- `IMAGE`: 0x9
- `SOUND`: 0xA
- `SOUND_PATCH`: 0xB
- `CLIPMAP`: 0xC
- `COMWORLD`: 0xD
- `GAMEWORLD`: 0xE
- `MAP_ENTS`: 0xF
- `GFXWORLD`: 0x10
- `LIGHT_DEF`: 0x11
- `LENSFLARE_DEF`: 0x12
- `UI_MAP`: 0x13
- `FONT`: 0x14
- `FONTICON`: 0x15
- `LOCALIZE_ENTRY`: 0x16
- `WEAPON`: 0x17
- `WEAPONDEF`: 0x18
- `WEAPON_VARIANT`: 0x19
- `WEAPON_FULL`: 0x1A
- `CGMEDIA`: 0x1B
- `PLAYERSOUNDS`: 0x1C
- `PLAYERFX`: 0x1D
- `SHAREDWEAPONSOUNDS`: 0x1E
- `ATTACHMENT`: 0x1F
- `ATTACHMENT_UNIQUE`: 0x20
- `WEAPON_CAMO`: 0x21
- `CUSTOMIZATION_TABLE`: 0x22
- `CUSTOMIZATION_TABLE_FE_IMAGES`: 0x23
- `CUSTOMIZATION_TABLE_COLOR`: 0x24
- `SNDDRIVER_GLOBALS`: 0x25
- `FX`: 0x26
- `TAGFX`: 0x27
- `NEW_LENSFLARE_DEF`: 0x28
- `IMPACT_FX`: 0x29
- `IMPACT_SOUND`: 0x2A
- `PLAYER_CHARACTER`: 0x2B
- `AITYPE`: 0x2C
- `CHARACTER`: 0x2D
- `XMODELALIAS`: 0x2E
- `RAWFILE`: 0x2F
- `STRINGTABLE`: 0x30
- `STRUCTURED_TABLE`: 0x31
- `LEADERBOARD`: 0x32
- `DDL`: 0x33
- `GLASSES`: 0x34
- `TEXTURELIST`: 0x35
- `SCRIPTPARSETREE`: 0x36
- `KEYVALUEPAIRS`: 0x37
- `VEHICLEDEF`: 0x38
- `ADDON_MAP_ENTS`: 0x39
- `TRACER`: 0x3A
- `SLUG`: 0x3B
- `SURFACEFX_TABLE`: 0x3C
- `SURFACESOUNDDEF`: 0x3D
- `FOOTSTEP_TABLE`: 0x3E
- `ENTITYFXIMPACTS`: 0x3F
- `ENTITYSOUNDIMPACTS`: 0x40
- `ZBARRIER`: 0x41
- `VEHICLEFXDEF`: 0x42
- `VEHICLESOUNDDEF`: 0x43
- `TYPEINFO`: 0x44
- `SCRIPTBUNDLE`: 0x45
- `SCRIPTBUNDLELIST`: 0x46
- `RUMBLE`: 0x47
- `BULLETPENETRATION`: 0x48
- `LOCDMGTABLE`: 0x49
- `AIMTABLE`: 0x4A
- `ANIMSELECTORTABLESET`: 0x4B
- `ANIMMAPPINGTABLE`: 0x4C
- `ANIMSTATEMACHINE`: 0x4D
- `BEHAVIORTREE`: 0x4E
- `BEHAVIORSTATEMACHINE`: 0x4F
- `TTF`: 0x50
- `SANIM`: 0x51
- `LIGHT_DESCRIPTION`: 0x52
- `SHELLSHOCK`: 0x53
- `XCAM`: 0x54
- `BG_CACHE`: 0x55
- `TEXTURE_COMBO`: 0x56
- `FLAMETABLE`: 0x57
- `BITFIELD`: 0x58
- `ATTACHMENT_COSMETIC_VARIANT`: 0x59
- `MAPTABLE`: 0x5A
- `MAPTABLE_LOADING_IMAGES`: 0x5B
- `MEDAL`: 0x5C
- `MEDALTABLE`: 0x5D
- `OBJECTIVE`: 0x5E
- `OBJECTIVE_LIST`: 0x5F
- `UMBRA_TOME`: 0x60
- `NAVMESH`: 0x61
- `NAVVOLUME`: 0x62
- `BINARYHTML`: 0x63
- `LASER`: 0x64
- `BEAM`: 0x65
- `STREAMER_HINT`: 0x66

# Caveats

Due to internal engine handling quirks, not all asset pools can be cleanly,
safely, or even feasibly expanded.

The GFX World, COM World, and Game World asset pools will only ever use two
assets - the world in use, and a loading world, if any. The statically allocated
pools for these world assets are accessed directly in many places in the engine,
and as such, pool reallocation will cause incorrect world assets to be used or
modified in some functions in the engine. Re-allocation of these pools is
therefore both unsafe and non-beneficial.

The engine also accesses the statically allocated GFX Image pool directly in
some functions, and is known to use hard-coded asset pool size when handling the
XModel Mesh asset pool. Re-allocation of these pools will not be beneficial, and
may be unsafe.

Use with caution, on a trial-and-error basis.
