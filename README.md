# Ezz BOIII ☄️: Call of Duty® Black Ops III Client

[![github](https://img.shields.io/badge/GitHub-Repository-blue)](https://github.com/Ezz-lol/boiii-free)

---

> [!NOTE]
> Feel free to open up Pull requests 😑

---

## Table of Contents

- [About BOIII](#about-boiii)
- [Client Download](#client-download)
- [Prerequisites](#prerequisites)
- [Install Instructions](#install-instructions)
    - [Quick Install](#quick-install)
    - [Manual Installation](#manual-installation)
- [Where Can I Get the Game?](#where-can-i-get-the-game)
    - [Downloading via Torrent](#downloading-via-torrent)
    - [What If I Have a Pirated Version?](#what-if-i-have-a-pirated-version)
- [Loading Mods & Custom Maps](#loading-mods--custom-maps)
- [Workshop Downloader](#workshop-downloader)
- [Plugin System](#plugin-system)
- [Command Line Arguments](#command-line-arguments)
- [Hosting a Dedicated Server](#hosting-a-dedicated-server)
    - [Requirements](#requirements)
    - [Server Setup](#server-setup)
    - [Connecting](#connecting)
    - [LAN & VPN Play](#lan--vpn-play)
    - [Port Forwarding Alternatives](#port-forwarding-alternatives)
- [Zombies Server Setup](#zombies-server-setup)
- [Compile from Source](#compile-from-source)
- [Credits](#credits)
- [Disclaimer](#disclaimer)

---

## About BOIII

BOIII is a free, community-driven modification for Call of Duty: Black Ops III that removes Steam ownership verification and enhances the multiplayer and zombies experience. Whether you own the game or not, BOIII lets you jump in and play!

**Key Features:**
- ✅ No Steam ownership required
- 🌐 Cross-platform server browser
- 🎮 Full multiplayer & zombies support
- 🗺️ Custom maps and mods support
- 🔧 Dedicated server hosting
- 🎨 Steam Workshop integration
- 🔌 Plugin system for extensibility

---

## Client Download

**Latest Release:** [Download BOIII Client](https://github.com/Ezz-lol/boiii-free/releases/latest)

**Available Downloads:**
- `boiii.exe` - Main BOIII client executable
- `BOIII-Full.zip` - Complete package with all files
- Source code available on GitHub

**Quick Links:**
- 📖 [Full Installation Guide](https://forum.ezz.lol/topic/5/bo3-guide)
- 💬 [Discord Community](https://dc.ezz.lol)
- 🐛 [Report Issues](https://github.com/Ezz-lol/boiii-free/issues)

---

## Prerequisites

- **Operating System:** Windows 10/11 (64-bit)
- **Game Files:** Call of Duty: Black Ops III installation
- **Storage:** ~60GB free space (for full game + DLC)
- **Optional:** Steam (if you own the game)

---

## Install Instructions

### Quick Install

1. **Download** the latest `BOIII.exe` from [Releases](https://github.com/ezz-boiii/boiii/releases/latest)
2. **Place** the executable in your Call of Duty: Black Ops III game directory
3. **Run** `BOIII.exe`
4. **Play!** 🎮

> [!TIP]
> The default Steam installation path is usually:
> `C:/Program Files (x86)/Steam/steamapps/common/Call of Duty Black Ops III`

### Manual Installation

1. **Download** `BOIII-Full.zip` from the releases page
2. **Extract** all contents to your Black Ops III game directory
3. **Launch** the game using `boiii.exe`
4. **Customize** your name in the settings or use `/name YOURNAME` in-game

> [!WARNING]
> Make sure to extract **all files** from the zip archive, not just the executable!

---

## Where Can I Get the Game?

### Option 1: Steam (Recommended)
Purchase and download from [Steam Store](https://store.steampowered.com/app/311210/Call_of_Duty_Black_Ops_III/) 💰

### Option 2: Free Download
If you can't afford the game, you can download the game files here:
- **Direct Download:** [Click here](https://gofile.io/d/7pvpEs)
- **Torrent:** [Download torrent](https://github.com/ezz-boiii/boiii/releases/download/game-files/bo3-full-game.torrent)

> [!NOTE]
> DLC files are included in the torrent download.

### Downloading via Torrent

> [!WARNING]
> **Use a VPN** to avoid copyright notices from your ISP!
> Check what's visible about your downloads: https://iknowwhatyoudownload.com

**Steps:**

1. **Download qBittorrent**
    - Get it from [qBittorrent.org](https://www.qbittorrent.org/download)
    - ✅ Free, open-source, and ad-free!

2. **Open the `.torrent` file** in qBittorrent

3. **Set download location** to your games folder
    - Example: `D:/Games/Call of Duty Black Ops III`

4. **Important:** Set "Content layout" to **"Don't create subfolder"**

5. **Start download** and wait until it shows "100% - Seeding"

6. **Keep seeding** to help others download faster (optional but appreciated! 😊)

### What If I Have a Pirated Version?

No worries! BOIII works perfectly with pirated game files. Just make sure you have:
- ✅ The latest game binaries (`BlackOps3.exe`)
- ✅ All required DLL files
- ✅ Complete zone files

The client will verify your game files on launch.

---

## Loading Mods & Custom Maps

> [!TIP]
> **Default Workshop Location (Steam):**
> `C:/Program Files (x86)/Steam/steamapps/workshop/content/311210/`
>
> **BOIII comes with a built-in Workshop Downloader** - see the [Workshop Downloader](#workshop-downloader) section!

**Installation Steps:**

1. **Download Mods/Maps**
    - Use the built-in BOIII Workshop Downloader
    - Or copy from your Steam workshop folder (if you own the game)
    - Or use external workshop downloaders

2. **Create Folders** (if they don't exist):
   ```
   Call of Duty Black Ops III/
     ├─ mods/
     └─ usermaps/
   ```

3. **Place Files:**
    - **Mods:** Extract to `mods/` folder
        - Example: `mods/zombie_mod_v2/zone/`
        - The folder structure should be: `mods/[MOD_NAME]/zone/`
    - **Custom Maps:** Extract to `usermaps/` folder
        - Example: `usermaps/zm_castle/zone/`
        - The folder structure should be: `usermaps/[MAP_NAME]/zone/`

4. **Launch Ezz BOIII** and select your mod/map from the menu! 🎮

> [!IMPORTANT]
> **For Workshop Downloads from Steam:**
> - Workshop items are in numbered folders (e.g., `311210/1234567890/`)
> - Copy the entire numbered folder
> - Rename it to match the mod/map name if needed
> - Make sure the `zone/` folder is inside

**Troubleshooting:**
- If a mod shows "unsafe Lua" warning, launch with `-unsafe-lua` argument
- If a map doesn't load, verify the folder structure matches `[type]/[name]/zone/`
- Check [forum.ezz.lol](https://forum.ezz.lol/topic/5/bo3-guide) for detailed guides

---

## Workshop Downloader

**BOIII has a built-in Steam Workshop Downloader!** 🎉

You can download Steam Workshop content directly through the BOIII client without needing Steam ownership.

**How to use:**
1. Launch `boiii.exe`
2. Navigate to the Workshop Downloader section
3. Enter the Workshop ID or URL
4. Download directly to your game folder

**External Tool (Alternative):**
If you prefer a standalone tool, check out [BOIIIWD by faroukbmiled](https://github.com/faroukbmiled/BOIIIWD)

**Finding Workshop IDs:**
- Go to any Steam Workshop item page
- Look at the URL: `steamcommunity.com/sharedfiles/filedetails/?id=XXXXXXXXX`
- The numbers after `id=` are the Workshop ID

---

## Plugin System

**plugins** 🔌

Extend Ezz BOIII's functionality with custom plugins. Plugins are DLL files that can hook into BOIII's component system.

### Installing Plugins

**Plugin Locations:**

Plugins can be placed in either of these folders:
- `%localappdata%/boiii/plugins/`
- `Call of Duty Black Ops III/boiii/plugins/`

All plugin activity is logged to `Call of Duty Black Ops III/boiii_players/plugins.log`

### Creating Plugins

Plugins must export these optional functions:

```cpp
// Plugin name (recommended)
extern "C" __declspec(dllexport) const char* p_name()
{
    return "My Plugin Name";
}

// Called after all components load
extern "C" __declspec(dllexport) void post_load() { }

// Called after game unpacking
extern "C" __declspec(dllexport) void post_unpack() { }

// Called before shutdown
extern "C" __declspec(dllexport) void pre_destroy() { }
```

> [!NOTE]
> All plugin functions are optional. Plugins only need to export the functions they use.

---

## Command Line Arguments

Launch BOIII with these arguments for extra features:

| Argument | Description |
|:---------|:------------|
| `-unsafe-lua` | Allow mods to use unsafe Lua functions (required for some mods like All-Around Enhancement) |
| `-dedicated` | Launch as dedicated server |
| `-nosteam` | Bypass Steam entirely |
| `-nointro` | Skip intro videos |
| `-windowed` | Launch in windowed mode |
| `-safe` | Launch in safe mode (disable mods) |
| `-console` | Enable developer console |
| `-port XXXX` | Set server port (default: 27017) |
| `-launch` | Force BOIII to start directly (used by launchers/shortcuts; skips some pre-checks) |
| `-noupdate` | Disable automatic updates (not recommanded) |
| `-update` | Force enable updates (including host binary in debug builds) |
| `-norelaunch` | Skip automatic relaunch after updates |
| `-headless` | Run in headless mode (no GUI for the console) |
| `-nopatch` | Disable some of the server's patches |


**Example:**
```bash
boiii.exe -nointro -console -unsafe-lua
```

> [!WARNING]
> The `-unsafe-lua` argument is **required** for certain mods that need to modify the UI, menus, or game scripts (like All-Around Enhancement Mod). Only use this with trusted mods!
>
> The `-headless` may not behave correctly on non-servers!

---

## Hosting a Dedicated Server

### Requirements

- ✅ [Visual C++ 2015-2022 Redistributable](https://aka.ms/vs/17/release/vc_redist.x64.exe)
- ✅ Text editor ([VS Code](https://code.visualstudio.com/), [Notepad++](https://notepad-plus-plus.org/), or [Sublime Text](https://www.sublimetext.com/))
- ✅ Computer or VPS with 24/7 uptime
- ✅ Decent internet connection (10+ Mbps upload recommended)
- ✅ Basic technical knowledge
- ⚠️ Port forwarding access (or see [alternatives](#port-forwarding-alternatives))

### Server Setup

**For detailed server setup instructions, check out:**
🔗 [BO3 Server Installer by framilano](https://github.com/framilano/BlackOps3ServerInstaller)

**Quick Steps:**

1. **Download** BO3 Unranked Dedicated Server from Steam (Tools section)

2. **Add BOIII files** to your server directory

3. **Configure server settings:**
    - Edit `zone/dedicated.cfg`
    - Set server name, map rotation, game mode
    - Configure player count and rules

4. **Setup firewall rules:**
    - Allow UDP port 27017 (or your custom port)
    - Allow TCP port 27017 (optional but recommended)

5. **Launch server:**
   ```bash
   boiii.exe -dedicated
   ```

6. **Monitor** the console for any errors

## Zombies Server Setup

Hosting Zombies requires additional files that don't come with the dedicated server package.

### Required Files

Copy these from your full game installation to your dedicated server:

**Common Zombies Files:**
```
zone/en_zm_patch.ff
zone/en_zm_common.ff
zone/zm_patch.ff
zone/zm_common.fd
zone/zm_common.ff
zone/zm_levelcommon.ff
```

**Map-Specific Files (Example: Shadows of Evil):**
```
zone/en_zm_zod.ff
zone/en_zm_zod_patch.ff
zone/zm_zod.ff
zone/zm_zod_patch.ff
```

### Installation

1. **Copy files** from `Call of Duty Black Ops III/zone/` to your server's `zone/` folder
2. **Repeat** for each map you want to host
3. **Skip** `.xpak` files (these are textures/sounds that servers don't need)

> [!NOTE]
> If the server crashes on startup, check `console_mp.log` for missing zone files

**Common Maps:**
- `zm_zod` - Shadows of Evil
- `zm_factory` - The Giant
- `zm_castle` - Der Eisendrache
- `zm_island` - Zetsubou No Shima
- `zm_stalingrad` - Gorod Krovi
- `zm_genesis` - Revelations

### Connecting

**Option 1: Server Browser**
- Open Ezz BOIII client
- Navigate to "Server Browser"
- Find your server and join!

**Option 2: Direct Connect**
- Open console (press `~`)
- Type: `/connect IP:PORT`

**Examples:**
- Local: `/connect 192.168.1.100:27017`
- WAN: `/connect 45.123.67.89:27017`

> [!TIP]
> Find your local IP: Press `Win + R`, type `cmd`, then type `ipconfig`
> Find your WAN IP: Visit [WhatIsMyIP.com](https://www.whatismyip.com/)

### LAN & VPN Play

Don't want to mess with port forwarding? Use these VPN tools to play with friends! 😎

#### How It Works
- Players on the **same local network** are automatically discovered and shown in the **LAN** tab.
- VPN users (ZeroTier, Radmin VPN, Hamachi, etc.) need to manually add the host's IP to see the server in the LAN list.

#### Setup Steps

**For the Host (Person hosting the game):**

1. Make sure you're connected to the **same LAN or VPN network** as your friends
2. Launch `boiii.exe`
3. Start a game:
4. Share your IP address with your friends:
    - **VPN IP:** Check your VPN client for your virtual IP address

**For Clients (People joining the game):**

1. Make sure you're connected to the **same LAN or VPN network** as the host
2. Launch `boiii.exe`
3. **Option A: Join via LAN Tab (Recommended)**
    - Open the console (press `~` key)
    - Add the host's IP address:
      ```
      /lan_add <VPN_IP>
      ```
    - Go to **Server Browser → LAN tab**
    - The server should appear in the list
    - Click to join!
    - Or you can use `/connect <VPN_IP>`

> [!TIP]
> If you're on the same local network, the server should automatically appear in the LAN tab without needing `/lan_add`

#### Recommended VPN Tools

- **[ZeroTier](https://www.zerotier.com/)** - Best overall, easy to set up, works great for gaming
- **[Radmin VPN](https://www.radmin-vpn.com/)** - Simple interface, fast, no configuration needed
- **[Hamachi](https://vpn.net/)** - Legacy option, widely known

---

## Compile from Source

Want to build Ezz BOIII yourself? Here's how! 🔨

### Prerequisites

- [Visual Studio 2022](https://visualstudio.microsoft.com/downloads/) (Community Edition is free)
- [Git](https://git-scm.com/downloads)
- Windows 10/11 SDK
- VS Build Tools ([vs config you can import to quickly get the right ones](https://app.filen.io/#/d/52faaefc-2331-4904-897f-97bc2b36e4f1%23373366412d6a5456715853426253475063347531343161793171796d726d7445))<img width="1128" height="399" alt="image" src="https://github.com/user-attachments/assets/8f3a7a4d-b933-4d47-a193-c67260b96f16" />


### Build Steps

1. **Clone the repository:**
   ```bash
   git clone https://github.com/Ezz-lol/boiii-free.git
   cd boiii-free
   ```

2. **Initialize submodules:**
   ```bash
   git submodule update --init --recursive
   ```

3. **Generate project files:**
   ```bash
   generate.bat
   ```

4. **Open in Visual Studio:**
    - Open `boiii.sln`
    - Set configuration to `Release` and platform to `x64`
    - Build the solution (Ctrl+Shift+B)

5. **Find your build:**
    - Output will be in `build/bin/x64/Release/`

> [!TIP]
> You can also use `build.bat` to compile directly from the command line!

---

## Credits

**BOIII Development Team** 💪
- Developers, contributors, and the entire BOIII community

**Special Thanks:**
- [Likeicareaboutit](https://github.com/Likeicareaboutit) - Steam Workshop Downloader
- [framilano](https://github.com/framilano) - BO3 Server Installer
- Everyone in the [BOIII Discord](https://dc.ezz.lol) community! 😎


---

## Disclaimer

This software has been created purely for the purposes of academic research and to preserve access to Call of Duty: Black Ops III multiplayer and zombies modes. It is not intended to be used to harm others or violate any terms of service.

**Project maintainers are not responsible or liable for misuse of the software. Use responsibly.**

This is a non-profit, community-driven project. We do not condone piracy. If you enjoy the game, please consider supporting the developers by purchasing it legally.

---

<p align="center">
  <strong>Join our community!</strong><br>
  <a href="https://dc.ezz.lol">Discord</a> • 
  <a href="https://github.com/Ezz-lol/boiii-free">GitHub</a> • 
  <a href="https://github.com/Ezz-lol/boiii-free/issues">Report Issues</a> •
  <a href="https://forum.ezz.lol/topic/5/bo3-guide">Installation Guide</a>
</p>

<p align="center">
  Made with ❤️ by the BOIII community ☄️
</p>
