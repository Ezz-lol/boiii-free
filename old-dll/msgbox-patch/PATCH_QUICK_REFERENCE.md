# Quick Patch Reference

## Changes Made to ext.dll

### Patch 1: Remove First MessageBoxA
| Property | Value |
|----------|--------|
| File Offset | 0x000AB141 |
| Virtual Address | 0x1800abd41 |
| Original Bytes | `FF 15 69 28 00 00` |
| Original Instruction | `call qword [rip+0x2869]` (MessageBoxA) |
| Patched Bytes | `90 90 90 90 90 90` |
| Patched Instruction | `nop` × 6 |

### Patch 2: Remove Second MessageBoxA
| Property | Value |
|----------|--------|
| File Offset | 0x000AB1A0 |
| Virtual Address | 0x1800abda0 |
| Original Bytes | `FF 15 0A 28 00 00` |
| Original Instruction | `call qword [rip+0x280a]` (MessageBoxA) |
| Patched Bytes | `90 90 90 90 90 90` |
| Patched Instruction | `nop` × 6 |

## Radiff2 Output Explained

```bash
radiff2 "old\ext.dll" "msgbox-patch\ext.dll"
INFO: Second cons!
0x000ab141 ff1569280000 => 909090909090 0x000ab141
             ^^^^^^^^^^^     ^^^^^^^^^^^^
             Original        Patched
             (call)         (6 nops)

0x000ab1a0 ff150a280000 => 909090909090 0x000ab1a0
             ^^^^^^^^^^^     ^^^^^^^^^^^^
             Original        Patched
             (call)         (6 nops)
```

## How to Verify

Using xxd:
```bash
xxd -s 0xAB141 -l 12 ext.dll
# Should show: 909090909090...
```

Using radare2:
```bash
r2 -qc "pd 10 @ 0x1800abd3f" ext.dll
# Should show nop instructions at 0x1800abd41
```

## Summary

- **Total locations patched**: 2
- **Total bytes modified**: 12 (6 at each location)
- **Patch type**: Inline instruction replacement
- **Effect**: Removes blocking MessageBoxA dialog while preserving execution flow
