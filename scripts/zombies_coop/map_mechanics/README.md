# Expanded Zombies map mechanics

This experimental server-side adapter supports configured capacities of five
through eight. Configured one-through-four sessions keep their stock paths. It
uses the current real player roster, including bots; it does not invent player
entities or change the network schema to fit map geometry.

The native loader checks nine specific stock script objects before redirecting
22 call or function-reference imports to eight authored helpers. A different
stock object or conflicting producer fails the expanded readiness gate. Stock
code, exports and source CRC stay unchanged; only checked import/include metadata
is rebuilt. Helpers have fixed names, immutable pre-link templates and separate
backing for each server VM generation. The shared asset retention budget remains
bounded. `contracts.json` and `payloads.json` describe the packaged objects.

The helpers cover:

- Shangri-La Pack-a-Punch plates and Der Eisendrache boss readiness, with the
  requirement bounded by the live roster and four physical positions.
- The Giant and Kino teleports, with validated destination groups, paired
  position/angle selection, and token-owned cleanup on departure or shutdown.
- Gorod dragon passenger tags, return landings, wings and gauntlet respawns.
  Eight passengers share the existing four tags. Landing waits are bounded;
  collision grace requires the owned core callback and more than four live
  players. Separation runs after arrival, never while attached to the dragon.
- Gorod challenge allocation uses independently owned per-player descriptors.
  The initial catalog is bounded by the configured session capacity, with a
  minimum of the six stock descriptors per tier. Departing players return only
  their validated assignments once; consumed assignments are not replenished.
  Current-roster counts, catalog conservation and callback ownership must remain
  valid for expanded readiness. This does not validate every challenge worker
  or reuse of the same player script object on reconnect.
- Shared Anywhere But Here validation/activation and destination selection.
  Selection retains the chosen enabled room and its existing children, supports
  explicit custom labels above four, and otherwise shares stock labels. An
  undefined destination is rejected before teleport mutation. An already
  consumed gum is not refunded by this adapter.

`coopMapMechanicsStatus` prints a read-only `COOP_MAP_MECHANICS|` JSON record.
It reports the current map, capacity, startup epoch, VM generation, readiness
and a reason. `linked_body_authenticated` is always false: retained source
receipts do not prove the identity of every live engine-linked function body.
Official maps require the shared BGB binding. A custom map which never loads
that module may omit it; requesting an unsupported variant blocks expanded
readiness. This is not a claim of compatibility with every Workshop script.

Run `python tests/zombies_coop_map_payloads.py` for source/object packaging and
helper-import checks. The four `zombies_coop_map_*.cpp` suites exercise synthetic
import bindings, cache/lifecycle ownership, concurrent lookups and allocation
failure. They are host tests, not gameplay validation. Rebuild authored payloads
using the pinned ACTS compiler recorded in `payloads.json`, `gscc --game bo3
--platform pc --name <logical_name> -o <output> <source>`, retaining the listed
logical names and verifying export arities and hashes before replacing the
embedded arrays. Do not install private test fixtures as shipping data.

Real clients, client HUD/CSC, occupied map quests, shared dragon scenes,
physics at blocked fallback origins, reconnects and LAN/custom lobby behavior
still require in-game validation. Keep this branch experimental until those
checks pass.
