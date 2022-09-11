![license](https://img.shields.io/github/license/momo5502/boiii.svg)
[![open bugs](https://img.shields.io/github/issues/momo5502/boiii/bug?label=bugs)](https://github.com/momo5502/boiii/issues?q=is%3Aissue+is%3Aopen+label%3Abug)
[![Build](https://github.com/momo5502/boiii/workflows/Build/badge.svg)](https://github.com/momo5502/boiii/actions)
[![paypal](https://img.shields.io/badge/PayPal-support-blue.svg?logo=paypal)](https://paypal.me/momo5502)
[![patreon](https://img.shields.io/badge/Patreon-support-red.svg?logo=patreon)](https://www.patreon.com/xlabsproject)
[![discord](https://img.shields.io/endpoint?url=https://momo5502.com/iw4x/members-badge.php)](https://discord.gg/sKeVmR3)


# BOIII

Reverse Engineering and analysis of Call of Duty: Black Ops 3. Very experimental.

<img src="https://encrypted-tbn0.gstatic.com/images?q=tbn:ANd9GcQeSXYzQITJrcjiifN1nqX1fsVE7VwLZ3vl2g&usqp=CAU">

## Roadmap

- [x] Steam API Emulation
- [x] Steam Integrity Bypass
- [x] Offline Multiplayer/Zombies/Campaign Support
- [x] RE Tool Detection Bypass (IDA Pro, HxD, ...)
- [x] Disable Hardware Breakpoint Detection
- [ ] Disable Anti-Debugging mechanisms
- [ ] Disable Integrity Checks
- [ ] Demonware Emulation

## Download

Unzip <a href="https://nightly.link/momo5502/boiii/workflows/build/main/Release%20Binary.zip">this</a> into your BOIII folder and run the BlackOps3.exe

## Compile from source

- Clone the Git repo. Do NOT download it as ZIP, that won't work.
- Update the submodules and run `premake5 vs2022` or simply use the delivered `generate.bat`.
- Build via solution file in `build\boiii.sln`.

## Disclaimer

This software has been created purely for the purposes of
academic research. It is not intended to be used to harm
others. Project maintainers are not responsible or
liable for misuse of the software. Use responsibly.

