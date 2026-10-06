#!/usr/bin/env python3
"""Generates src/client/component/loot_pool.hpp for Black Market Drops.

Inputs:
  mplootitems.csv       gamedata/loot/mplootitems.csv (e.g. from ate47/bo3-source)
  mpunreleasedloot.csv  gamedata/loot/mpunreleasedloot.csv
  localize_dump.txt     output of the `lootdumpstrings` console command
                        (Debug build, run from the Multiplayer menu)
  image_dump.txt        output of the `lootdumpimages` console command

Usage:
  python scripts/generate_loot_pool.py mplootitems.csv mpunreleasedloot.csv \\
      localize_dump.txt image_dump.txt > src/client/component/loot_pool.hpp
"""

import csv
import re
import sys

HEADER = """// Generated from gamedata/loot/mplootitems.csv (ate47/bo3-source) and the
// game's English localized strings. Rows without an item id and items in
// mpunreleasedloot.csv are excluded.
#pragma once

#include <cstdint>

namespace loot::pool {
enum rarity : uint8_t {
  common = 0,
  rare = 1,
  epic = 2,
  legendary = 3,
  limited = 4
};

struct entry {
  uint32_t id;
  const char *category;
  rarity tier;
  const char *name;    // internal name
  const char *title;   // e.g. "Common Camo - Razorback"
  const char *display; // e.g. "Royal"
  const char *icon;    // UI image, empty if none was found
};

// clang-format off
inline constexpr entry items[] = {"""

FOOTER = """};
// clang-format on
} // namespace loot::pool
"""

# Localization keys that don't follow a pattern.
NAME_KEY_OVERRIDES = {
    "pbt_loot_theme_outrider_body2_ashblood": "HEROES_OUTRIDER_ASH_AND_BLOOD",
    "pbt_loot_theme_outrider_head2_ashblood": "HEROES_OUTRIDER_ASH_AND_BLOOD",
    "pbt_loot_theme_spectre_body2_surgeon": "HEROES_SPECTER_SURGEON",
    "pbt_loot_theme_spectre_head4_surgeon": "HEROES_SPECTER_SURGEON",
    "pbt_loot_theme_enforcer_body2_war": "HEROES_ENFORCER_ARTOFWAR",
    "pbt_loot_theme_enforcer_head1_war": "HEROES_ENFORCER_ARTOFWAR",
}
DISPLAY_OVERRIDES = {"mtl_t7_camo_soviet_winter_blue": "Permafrost"}

# Event camos without a weapon icon: their camo swatch images.
CAMO_SWATCHES = {
    "camo_loot_nightmare": "menu_camo_nightmare",
    "mtl_t7_camo_loot_patricks_03": "menu_camo_patricks_03",
    "camo_dlc4_pap_04": "menu_camo_pap_dlc4_04",
    "camo_cherry_fizz": "menu_camo_summertime_cherry_fiz",
    "camo_vip_bubbles": "menu_camo_summertime_vip_bubbles",
    "camo_dlc3_pap_var_03": "menu_camo_dlc3_pap_04",
    "camo_dlc3_pap_var_02": "menu_camo_dlc3_pap_02",
    "mtl_t7_camo_soviet_winter_blue": "menu_camo_winter_soviet_blue",
    "mtl_t7_camo_honeycomb_amber": "menu_camo_honeycomb",
}

TAUNT_HEROES = {"ruin": "Ruin", "prophet": "Prophet", "seraph": "Seraph", "nomad": "Nomad", "specter": "Spectre"}
LABELS = {
    "taunt": "Taunt",
    "gesture": "Gesture",
    "attachment_variant": "Attachment",
    "calling_card": "Calling Card",
    "decal": "Decal",
    "emblem": "Emblem",
    "material": "Material",
    "specialist_outfit": "Outfit",
    "camo": "Camo",
    "reticle": "Reticle",
    "melee_weapon": "Melee Weapon",
    "weapon": "Weapon",
}
RARITIES = {"common": "Common", "rare": "Rare", "epic": "Epic", "legendary": "Legendary"}
ATTACHMENT_ICON_NAMES = {
    "extclip": "extmag",
    "fastreload": "fastmag",
    "suppressed": "suppressor",
    "rf": "rapidfire",
    "stalker": "extstock",
    "steadyaim": "laser",
    "damage": "highcaliber",
}


def load_inputs(items_csv, unreleased_csv, localize_txt, images_txt):
    loc = {}
    with open(localize_txt, encoding="utf-8", errors="replace") as f:
        for line in f:
            line = line.rstrip("\n")
            if "\t" in line:
                key, value = line.split("\t", 1)
                loc[key.upper()] = value

    with open(unreleased_csv) as f:
        unreleased = {line.strip().split(";")[0] for line in f if line.strip()}
    with open(items_csv) as f:
        rows = list(csv.reader(f))
    items = [
        r
        for r in rows
        if r[1].isdigit() and r[0].split(";")[0] not in unreleased and r[0] not in unreleased and r[2] != "c2"
    ]

    with open(images_txt) as f:
        images = {line.strip() for line in f}
    return loc, items, images


def strip_prefix(text, *prefixes):
    for prefix in prefixes:
        if text.startswith(prefix):
            return text[len(prefix):]
    return text


class Names:
    def __init__(self, loc):
        self.loc = loc
        self.normalized = {}
        for key in loc:
            self.normalized.setdefault(key.replace("_", ""), key)

    def lookup(self, *keys):
        for key in keys:
            if key and key.upper() in self.loc and self.loc[key.upper()].strip():
                return self.loc[key.upper()]
        for key in keys:
            if not key:
                continue
            match = self.normalized.get(key.upper().replace("_", ""))
            if match and self.loc[match].strip():
                return self.loc[match]
        return None

    def hero(self, code):
        return self.lookup("HEROES_" + code.replace("pbt_mp_", ""))

    def weapon(self, code):
        return self.lookup("WEAPON_" + code, "WEAPON_" + code.rstrip("0123456789"), "WEAPON_SPECIAL_" + code)

    def name(self, row):
        full, category = row[0], row[2]
        base = full.partition(";")[0]
        b = base.upper()
        if category == "taunt":
            return self.lookup("HEROES_" + b)
        if category == "gesture":
            return self.lookup("HEROES_" + b, "HEROES_" + (b[:-4] if b.endswith("_ALT") else b))
        if category == "calling_card":
            s = strip_prefix(b, "T7_LOOT_CALLINGCARD_")
            return self.lookup("EM_BACK_LOOT_" + s, "EM_BACK_LOOT_SINGLE_" + s, "EM_BACK_" + s, "EM_" + b, "EM_BACK_" + b)
        if category in ("decal", "emblem"):
            s = strip_prefix(b, "T7_LOOT_EMBLEM_", "T7_")
            e = strip_prefix(b, "EM_")
            e2 = re.sub(r"(_?0?1|_FULL)$", "", e)
            return self.lookup(
                "EM_" + b,
                "EM_" + strip_prefix(b, "T7_"),
                "EM_" + s,
                "EM_LOOT_" + s,
                "EM_T7_LOOT_EMBLEM_" + s,
                "EM_T7_LOOT_DECAL_ICON_" + s,
                "EM_" + e,
                "EM_" + e2,
            )
        if category == "specialist_outfit":
            m = re.match(r"PBT_LOOT_THEME_([A-Z]+)_(?:BODY|HEAD)\d+_(.+)", b)
            if not m:
                return None
            hero, theme = m.groups()
            return self.lookup(f"HEROES_{hero}_{theme}", "HEROES_" + theme)
        if category == "camo":
            s = strip_prefix(b, "CAMO_LOOT_", "CAMO_", "MTL_T7_CAMO_LOOT_", "MTL_T7_CAMO_")
            return self.lookup("MPUI_CAMO_" + s, "MPUI_CAMO_LOOT_" + s, "MPUI_" + b)
        if category == "material":
            if b.startswith("MENU_EM_MATERIAL_"):
                return self.lookup("MPUI_MATERIAL_" + strip_prefix(b, "MENU_EM_MATERIAL_"))
            s = strip_prefix(b, "MENU_CAMO_")
            s = s[:-8] if s.endswith("_PATTERN") else s
            return self.lookup("MPUI_CAMO_" + s, "MPUI_CAMO_LOOT_" + s)
        if category == "reticle":
            return self.lookup("MPUI_RETICLE_" + b)
        if category in ("weapon", "melee_weapon"):
            return self.lookup("WEAPON_" + b, "WEAPON_" + b.rstrip("0123456789"), "WEAPON_SPECIAL_" + b)
        if category == "attachment_variant":
            m = re.match(r"ACV_([A-Z0-9]+)_(.+)", b)
            if not m:
                return None
            attachment, weapon = m.groups()
            attachment_name = self.lookup("MPUI_" + attachment, "WEAPON_" + attachment)
            weapon_name = self.weapon(weapon.lower())
            return f"{attachment_name} ({weapon_name})" if attachment_name and weapon_name else None
        return None

    def context(self, row):
        base, _, suffix = row[0].partition(";")
        category = row[2]
        if category in ("gesture", "specialist_outfit") and suffix:
            return self.hero(suffix)
        if category == "taunt":
            m = re.match(r"t7_loot_taunt_(?:e_)?([a-z]+)_", base)
            return self.lookup("HEROES_" + m.group(1)) if m else None
        if category == "camo" and suffix:
            return self.weapon(suffix)
        return None


def readable(base):
    for prefix in (
        "t7_loot_callingcard_",
        "t7_loot_emblem_",
        "t7_loot_decal_icon_",
        "t7_loot_decal_",
        "t7_icon_emblem_",
        "t7_loot_",
        "camo_loot_",
        "mtl_t7_camo_",
        "camo_",
        "em_",
    ):
        if base.startswith(prefix):
            base = base[len(prefix):]
            break
    return " ".join(word.capitalize() for word in base.split("_") if word)


def title_and_display(names, row):
    base = row[0].partition(";")[0]
    category = row[2]
    name = names.loc.get(NAME_KEY_OVERRIDES[base]) if base in NAME_KEY_OVERRIDES else names.name(row)
    if not name:
        name = readable(base)

    context = names.context(row)
    if category == "taunt" and not context:
        m = re.match(r"t7_loot_taunt_(?:e_)?([a-z]+)_", base)
        context = TAUNT_HEROES.get(m.group(1)) if m else None
    if category == "specialist_outfit":
        part = "Head" if re.search(r"_head\d+_", base) else "Body"
        context = f"{context} {part}" if context else part

    body = f"{name} ({context})" if context else name
    label = f"{RARITIES.get(row[3], 'Limited')} {LABELS[category]}"

    display, context = body, None
    m = re.match(r"^(.*) \(([^()]*)\)$", body)
    if m:
        display, context = m.groups()
    if category == "attachment_variant":
        display += " Variant"
    display = DISPLAY_OVERRIDES.get(base, display)
    title = f"{label} - {context}" if context else label
    return title, display


class Icons:
    def __init__(self, images):
        self.images = images
        self.camo_icons = sorted(x for x in images if x.startswith("t7_icon_weapon_") and "_camo_" in x)

    def first(self, *candidates):
        for candidate in candidates:
            if candidate and candidate in self.images:
                return candidate
        return None

    def find(self, row):
        base, _, suffix = row[0].partition(";")
        category = row[2]
        if category in ("weapon", "melee_weapon"):
            return self.first("t7_icon_weapon_" + base, "img_t7_hud_icon_" + base)
        if category == "camo":
            s = strip_prefix(base, "camo_loot_", "mtl_t7_camo_loot_", "mtl_t7_camo_", "camo_")
            if suffix:
                hit = self.first(
                    f"t7_icon_weapon_{suffix}_camo_loot_{s}",
                    f"t7_icon_weapon_{suffix}_camo_{s}",
                    f"t7_icon_weapon_{suffix}_{base}",
                    f"t7_icon_weapon_{suffix}_camo_{s.lower()}",
                )
                if hit:
                    return hit
            for ending in (f"_camo_loot_{s}", f"_camo_{s}", f"_camo_{s.lower()}"):
                for icon in self.camo_icons:
                    if icon.endswith(ending):
                        return icon
            return CAMO_SWATCHES.get(base, "t7_icon_menu_simple_paintjobs")
        if category == "specialist_outfit":
            hit = self.first("t7_" + base)
            if hit:
                return hit
            m = re.match(r"pbt_loot_theme_([a-z]+)_(body|head)\d+_(.+)", base)
            if m:
                same_theme = re.compile(rf"t7_pbt_loot_theme_{m.group(1)}_(?:body|head)\d+_{re.escape(m.group(3))}$")
                for icon in sorted(self.images):
                    if same_theme.match(icon):
                        return icon
                return self.first(f"t7_icon_pbt_mp_{m.group(1)}_{m.group(2)}1_skin1_rwd")
            return None
        if category in ("calling_card", "decal", "material", "reticle"):
            return self.first(base)
        if category == "gesture":
            m = re.match(r"t7_loot_gesture_(boast|goodgame|threaten)_", base)
            return self.first("t7_icon_blackmarket_" + m.group(1)) if m else None
        if category == "taunt":
            return self.first("t7_icon_blackmarket_taunt_epic" if "_taunt_e_" in base else "t7_icon_blackmarket_taunt")
        if category == "emblem":
            s = base.replace("t7_loot_emblem_", "")
            return self.first(
                f"t7_loot_decal_icon_{s}", f"t7_loot_decal_{s}", f"t7_icon_emblem_{s}", f"em_{s}"
            ) or self.first("t7_icon_menu_simple_emblems")
        if category == "attachment_variant":
            m = re.match(r"acv_([a-z0-9]+)_(.+)", base)
            if not m:
                return None
            attachment, weapon = m.groups()
            attachment = ATTACHMENT_ICON_NAMES.get(attachment, attachment)
            return self.first(
                f"t7_icon_attach_{weapon}_{attachment}_02",
                f"t7_icon_attach_{weapon}_{attachment}_01_rwd",
                f"t7_icon_attach_{weapon}_{attachment}_01",
            ) or "t7_icon_menu_simple_variants"
        return None


def escape(text):
    return text.replace("\\", "\\\\").replace('"', '\\"')


def main():
    if len(sys.argv) != 5:
        sys.exit(__doc__)
    loc, items, images = load_inputs(*sys.argv[1:5])
    names = Names(loc)
    icons = Icons(images)
    tiers = {"common": "common", "rare": "rare", "epic": "epic", "legendary": "legendary"}

    lines = [HEADER]
    for row in items:
        title, display = title_and_display(names, row)
        icon = icons.find(row) or ""
        lines.append(
            f'    {{{int(row[1])}u, "{row[2]}", {tiers.get(row[3], "limited")}, "{escape(row[0])}", '
            f'"{escape(title)}", "{escape(display)}", "{escape(icon)}"}},'
        )
    sys.stdout.write("\n".join(lines) + "\n" + FOOTER)


if __name__ == "__main__":
    main()
