# Lobby protocol

This document describes the packets BOIII clients, hosts, dedicated servers
and the master server exchange to find each other, join party and game
lobbies, and authenticate players. It describes the protocol, not a particular
implementation. The code in `src/client/component` (`nat.cpp`, `friends.cpp`,
`auth.cpp`, `profile_infos.cpp`, `getinfo.cpp`) is the reference
implementation if you need more detail.

The engine's own lobby protocol (the packets the game sends once two lobbies
can reach each other) is not covered here. This protocol only gets the two
sides talking and hands the engine what it needs to start its own lobby join.

## Roles

- **Host**: a player whose party or game lobby others can join.
- **Joiner**: a player joining a host, either a friend's party or a match.
- **Master**: the master server. Besides the server list, it acts as a
  rendezvous point so a joiner can learn a host's address without the host's
  ports being open.
- **Server**: a running match, either a dedicated server or a player-hosted
  listen server.

## Packet format

Every packet is a single UDP datagram sent from the game's socket:

```
FF FF FF FF <command> <separator> <payload>
```

- `<command>` is ASCII and matched case-insensitively.
- `<separator>` is one byte, a space unless stated otherwise.
- `<payload>` is everything after the separator, up to the end of the
  datagram.

Payloads come in two styles:

- **Text**: fields separated by whitespace. A trailing name field may contain
  spaces and runs to the end of the payload.
- **Binary**: little-endian integers and length-prefixed strings. A
  length-prefixed string is a `uint32` byte count followed by that many bytes.

The master server accepts at most 512 bytes of payload per packet.

### Common fields

| Field | Format |
|---|---|
| Token | 8 to 64 characters from `[A-Za-z0-9_-]`. Clients generate 16 lowercase hex digits. |
| Friend code | A player's XUID as an unsigned decimal integer. |
| Endpoint | `a.b.c.d:port` (IPv4). |
| XUID (in `lobbyHost`) | Hexadecimal, no prefix. |
| Hex blob | Bytes as lowercase hex, two digits per byte. |

### Fragmented packets

Binary payloads larger than one datagram (`connect`, `profileInfo`) are split
into fragments. Each fragment's payload is:

| Type | Field |
|---|---|
| `uint64` | Fragment id, the same for every fragment of one message |
| `uint32` | Fragment count, 1 to 100 |
| `uint32` | Index of this fragment, starting at 0 |
| bytes | Up to 1024 bytes of the message |

The receiver keeps fragments per sender address and fragment id, and passes
the message on once every index has arrived. Incomplete messages are dropped
after about 5 seconds.

## Overview

### Hosting

A host that has opened its party to friends picks a random token and keeps
sending `privRegister` and `friendPublish` to the master every 5 seconds. The
master forgets both 15 seconds after the last refresh, so a host that stops
sending disappears on its own. A host that is online but not accepting joins
still sends `friendPublish` with `-` instead of a token, so friends see it as
online.

### Joining a friend

1. The joiner asks the master about its friends with `friendLookup`. For a
   friend who is hosting, the master answers `friendPresence` with the host's
   token and public endpoint.
2. The joiner sends `privJoin` with that token. The master answers both sides
   with `privPeer`, telling each the other's public endpoint plus any LAN
   endpoints they reported.
3. Both sides send `punch` to every endpoint they were given, which opens a
   path through NAT. Whichever side receives a `punch` or `punchAck` first
   knows a working endpoint for the other.
4. The joiner sends `lobbyJoin` to that endpoint. The host answers with
   `lobbyHost`, carrying what the engine needs to join the host's lobby, or
   `lobbyGame` if the host is in a match without a joinable game lobby.
5. The joiner hands the `lobbyHost` details to the engine's lobby join. From
   then on the engine talks to the host with its own lobby packets.

If no endpoint answers within 12 seconds, the joiner asks the master again
once. If that also fails, it falls back to the friend's last known address, if
there is one.

### Joining a match from the server browser

The joiner queries the server with `getInfo`. If the reply has a non-empty
`lobby` key, the server is a player-hosted match, and the joiner sends
`lobbyJoin` with the token `match` straight to that address (no master, no
punching). Otherwise it connects to the server directly.

### Connecting to a server

When a player connects to a running match (directly or after a lobby join),
the joiner authenticates with `getchallenge`, `getChallengeResponse` and
`connect`. The server then tells every player the XUIDs of everyone else with
`playerXuid` and shares player profiles with `profileInfo`.

## Rendezvous packets (client and master)

The master only reacts to packets from the endpoint that sent them, so a host
can only refresh or publish its own entries. Clients ignore every packet in
this section unless it comes from a master server address they know.

### `privRegister` (host to master)

```
privRegister <token> [<lan endpoint>]
```

- **Sent**: every 5 seconds while the host's party is open to friends.
- **Effect**: creates or refreshes a session for `<token>` pointing at the
  sender's public endpoint. The optional LAN endpoint is stored as an extra
  address a joiner on the same network can try.
- **Master handling**: ignores the packet if another endpoint owns the token
  and refreshed it in the last 15 seconds. A LAN endpoint is kept only if it
  has a port from 1024 to 65535 and is either a private address (10/8,
  172.16/12, 192.168/16, 100.64/10, 25/8, 26/8) or on the sender's public IP.
  Replies with `privRegisterAck`.

### `privRegisterAck` (master to host)

```
privRegisterAck <endpoint>
```

- **Effect**: confirms the session exists. `<endpoint>` is the host's public
  endpoint as the master sees it. The host only advertises its token to
  friends after receiving this.

### `friendPublish` (client to master)

```
friendPublish 1 <friend code> <token>
friendPublish 1 <friend code> -
```

- **Sent**: every 5 seconds. With a token while hosting, with `-` otherwise.
- **Effect**: tells the master this friend code is online at the sender's
  endpoint, and which session token (if any) friends can join with.
- **Master handling**: with a token, the token must be a live session owned by
  the same endpoint. A friend code already published from a different
  endpoint in the last 15 seconds can't be taken over, unless the new packet
  has a token and the existing one doesn't.

### `friendLookup` (client to master)

```
friendLookup 1 <request id> <friend code>
```

- **Sent**: when the friends list refreshes, once per friend. All lookups of
  one refresh share a request id, which is a token.
- **Master handling**: replies with exactly one of `friendPresence`,
  `friendOnline` or `friendOffline`.

### `friendPresence` (master to client)

```
friendPresence 1 <request id> <friend code> <token> <endpoint>
```

- **Effect**: the friend is hosting a joinable party. `<token>` is used for
  `privJoin` and `<endpoint>` is the host's public endpoint, used as a
  fallback if punching fails.

### `friendOnline` and `friendOffline` (master to client)

```
friendOnline 1 <request id> <friend code>
friendOffline 1 <request id> <friend code>
```

- **Effect**: the friend is online but not hosting, or not online at all. A
  client treats friends it got no answer for within 2 seconds as offline.

### `privJoin` (joiner to master)

```
privJoin <token> [<lan endpoint>]
```

- **Sent**: when the joiner starts joining a friend, then every second until
  it has endpoints to try.
- **Master handling**: if the token is a live session not owned by the
  sender, sends `privPeer` to both the joiner and the host. Otherwise replies
  `privReject`.

### `privPeer` (master to host and joiner)

```
privPeer <token> <public endpoint> [<lan endpoint> ...]
```

- **Effect**: tells each side where to reach the other: the other side's
  public endpoint, then up to 2 LAN endpoints.
- **Handling**: the joiner adds the endpoints to the list it punches. The
  host, if the token is its own, punches the joiner's endpoints for 10
  seconds. Each side punches at most 16 endpoints.

### `privReject` (master to joiner)

```
privReject <token> unavailable
```

- **Effect**: the host's session no longer exists. The joiner gives up
  punching straight away and moves to its fallback.

### `friendInvite` (client to master, master to client)

From the inviter to the master:

```
friendInvite 1 <invitee friend code> <inviter friend code> <inviter name>
```

From the master to the invitee:

```
friendInvite 1 <inviter friend code> <inviter name>
```

- **Sent**: when a player invites a friend. The inviter opens its party to
  friends first, so the invitee can join it.
- **Master handling**: both the inviter and the invitee must have published
  in the last 15 seconds, and the inviter's publication must come from the
  sender's endpoint. If so, it forwards the invite to the invitee's endpoint.
  Otherwise it replies `friendInviteFailed`.
- **Client handling**: adds the inviter to the friends list, refreshes
  presence and shows the invite. The invitee then joins through the normal
  friend join.

### `friendInviteFailed` (master to inviter)

```
friendInviteFailed <invitee friend code>
```

- **Effect**: the invitee isn't online, so the invite wasn't delivered.

## Peer packets (host and joiner)

### `punch` and `punchAck`

```
punch <token>
punchAck <token>
```

- **Sent**: the joiner sends `punch` to all its candidate endpoints every 250
  ms while joining. The host does the same for the joiner's endpoints it got
  from `privPeer`.
- **Handling**: a side that receives `punch` with a token it is joining or
  hosting replies `punchAck` to the sender. When the joiner receives either
  packet with its token, the sender's address is a working endpoint for the
  host. The joiner stops punching and sends `lobbyJoin` there.

### `lobbyJoin` (joiner to host)

```
lobbyJoin <token> <xnaddr>
```

- `<token>` is the host's session token, or `match` when joining a match
  found in the server browser.
- `<xnaddr>` is the joiner's 37-byte engine address (`XNADDR`), hex encoded.
- **Sent**: once a working endpoint is known, or straight away for `match`.
  The joiner must already have its own party lobby, so it sends this from the
  multiplayer or zombies menu.
- **Host handling**:
  - Ignored unless the token is the host's current token (or `match` and the
    host has a joinable lobby), and the sender's address is usable: IPv4,
    port 1024 or higher, and not 0.x.x.x or multicast. Loopback is allowed.
  - If the host is in a match with no game lobby to join, replies
    `lobbyGame`.
  - Otherwise it remembers that the joiner's `XNADDR` maps to the sender's
    endpoint, so the engine's later lobby packets reach the joiner, and
    replies `lobbyHost` describing its game lobby if it has one, or its party
    lobby otherwise.
- If the joiner gets no reply within 4 seconds, the join fails. For a
  `match` join to a server that advertised `lobby` as `game`, the joiner
  connects to the server directly instead.

### `lobbyHost` (host to joiner)

```
lobbyHost <token> <xuid> <xnaddr> <security id> <security key> <lobby> c:<mod>:<map>:<usermap id> <name>
```

| Field | Meaning |
|---|---|
| `<token>` | Echo of the `lobbyJoin` token. |
| `<xuid>` | Host's XUID, hex. |
| `<xnaddr>` | Host's 37-byte `XNADDR`, hex. |
| `<security id>` | 8-byte lobby security id, hex. |
| `<security key>` | 16-byte lobby security key, hex. |
| `<lobby>` | `party` or `game`. |
| `c:...` | Content the joiner needs: mod publisher id, map name, usermap publisher id. Empty fields are left empty, for example `c:::`. |
| `<name>` | Host's name, to the end of the packet. |

- **Joiner handling**:
  - Ignored unless it answers the joiner's pending `lobbyJoin` (same token,
    from the address it was sent to).
  - When joining a game lobby, if the host runs a different mod, the joiner
    skips the lobby and connects to the match directly.
  - The joiner checks it has the mod and usermap. If not, it offers to
    download them and rejoins once they are installed. If the loaded mod is
    different, it switches mods and rejoins.
  - Otherwise it maps the host's `XNADDR` to the host's endpoint and starts
    the engine's lobby join with the host's XUID, name, security id, key and
    address, into the host's party or game lobby.

### `lobbyGame` (host to joiner)

```
lobbyGame <token>
```

- **Effect**: the host is in a match but has no game lobby the joiner can
  enter. The joiner connects to the match directly instead.

## Server packets

### `getInfo` and `infoResponse`

The standard server query. Two keys matter for lobbies:

- `lobby`: `party` or `game` when a player-hosted match can be joined through
  its lobby (send `lobbyJoin` with token `match`), empty otherwise. Dedicated
  servers always send it empty.
- `modId`: the server's mod, so a joiner can get the same mod before joining.

### `getchallenge` (client to server)

```
getchallenge
```

- **Sent**: while the client is connecting to a server, every 3 seconds
  until it gets a challenge. A client with two local players sends it once
  and uses the challenge for both, because the server keeps one challenge per
  address.
- **Server handling**: replies `getChallengeResponse` with the challenge it
  issued to that address. A challenge stays valid for 10 minutes; after that,
  the next request gets a new one.

### `getChallengeResponse` (server to client)

```
getChallengeResponse <32 bytes>
```

The payload is 32 raw random bytes, not text.

- **Client handling**: stores the challenge and moves every connecting local
  client from the challenging state to the connecting state, which sends
  `connect`.

### `connect` (client to server, fragmented)

The reassembled message:

| Type | Field |
|---|---|
| string | The player's ECC public key |
| string | Signature of the 32-byte challenge, made with the matching private key |
| `int32` | Profile version |
| string | Profile data |
| string | The engine's connect info string |

- **Server handling**:
  - Verifies the signature against the challenge it issued to the sender. On
    failure it replies `error Bad signature`.
  - Reads `xuid` from the info string. It must equal the hash of the public
    key, so a player can't claim someone else's XUID. Otherwise
    `error Bad XUID`.
  - Reads `name`. It must not be empty or contain `%`, `~` or characters
    outside printable ASCII. Otherwise `error Bad name`.
  - Stores the profile and sends it to the other players (`profileInfo`),
    lets the engine connect the player, then sends out XUIDs (`playerXuid`).

### `playerXuid` (server to clients)

| Type | Field |
|---|---|
| `int32` | Client number |
| `uint64` | XUID |

- **Sent**: when a player connects. The server sends the new player's entry
  to every connected client, and sends the new player one packet per client
  already connected. Bots and the server's own local players are skipped.
- **Client handling**: only accepted from the server the client is connected
  to, and only when the client isn't running a server itself. Stores the XUID
  for that client number, which is how clients know the XUIDs of other
  players.

### `profileInfo` (server to clients, fragmented)

| Type | Field |
|---|---|
| `uint64` | XUID |
| `int32` | Profile version |
| string | Profile data |

- **Sent**: when a player connects, to everyone, and to the new player for
  every profile the server already has.
- **Client handling**: only accepted from the connected server. Stores the
  profile for that XUID.
