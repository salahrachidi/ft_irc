
# 💬 ft_irc — Minimal IRC Server in C++

A standards‑lean, educational **IRC server** implemented in **C++98**, using **non‑blocking sockets** and `poll()` for multiplexing.  
Tested with popular IRC clients (✅ **LimeChat**) and basic CLI tools (`nc`, `telnet`).

Built by **Salaheddine Rachidi (@salahrachidi)** and **Usama Hrb (@usama-hrb)** as part of the 42/1337 curriculum.

---

## ✨ Highlights

- **Multi‑client** handling via non‑blocking sockets + `poll()`
- **Registration flow** (`PASS` → `NICK` → `USER`) with authentication
- **RFC‑style commands** (subset): `NICK`, `USER`, `PASS`, `PING/PONG`, `PRIVMSG`, `NOTICE`, `JOIN`, `PART`, `TOPIC`, `KICK`, `INVITE`, `MODE`, `WHO`, `QUIT`
- **Channels** with modes: `+i` (invite-only), `+t` (topic‑op‑only), `+k` (key), `+o` (op), `+l` (user limit)
- **Operator tools**: kick, invite, topic lock, set key, set limit
- **Error codes & replies** modeled after RFC 1459/2812 (minimal set)
- **Bot hook (optional)**: simple channel game / auto-reply (can be disabled)

---

## 🧱 Project Structure (example)

```
ft_irc/
├── Makefile
├── include/
│   ├── Server.hpp
│   ├── Client.hpp
│   ├── Channel.hpp
│   ├── Command.hpp
│   └── Utils.hpp
├── src/
│   ├── main.cpp
│   ├── Server.cpp
│   ├── Client.cpp
│   ├── Channel.cpp
│   ├── Command.cpp
│   └── Utils.cpp
└── docs/
    └── README.md  (this file)
```

> Your actual layout may differ; update paths accordingly.

---

## ⚙️ Build & Run

### Prereqs
- C++ compiler supporting **-std=c++98** (clang++/g++)
- Unix-like OS (macOS/Linux)

### Build
```bash
make        # or: c++ -Wall -Wextra -Werror -std=c++98 -o ircserv src/*.cpp
```

### Run
```bash
./ircserv <port> <password>
# example:
./ircserv 6667  supersecret
```
- `port`: TCP port to listen on (e.g., 6667)
- `password`: required in `PASS` during client registration

> Logs will print connections, joins, parts, errors, and command traces (if enabled).

---

## 🧪 Quick Tests (without a GUI client)

### Netcat (single user)
```bash
nc 127.0.0.1 6667
PASS supersecret
NICK alice
USER alice 0 * :Alice Doe
JOIN #devs
PRIVMSG #devs :hello from netcat
```

### Two shells (two users)
Open two terminals, connect as `alice` and `bob`, then `JOIN #devs` and exchange `PRIVMSG`.

### Health check
```bash
printf "PING :12345\r\n" | nc 127.0.0.1 6667
# expect: PONG :12345 (after registration)
```

---

## 🧪 Testing with LimeChat (macOS)

1. Install LimeChat (App Store or the official site).
2. **Server** → *Connect* → *Add Server...*
3. **Host**: `127.0.0.1` • **Port**: `6667` • **Use SSL**: Off
4. **Nick**: choose one (`salah` / `usama`)  
5. **Server Password**: set to your `<password>` (e.g., `supersecret`)
6. Connect → `/join #general` → chat!
7. Useful commands in LimeChat input:
   - `/join #room`
   - `/part #room`
   - `/topic #room New topic`
   - `/mode #room +i` (invite‑only), `/mode #room +k key`, `/mode #room +l 42`
   - `/invite nick #room`
   - `/kick #room nick :reason`
   - `/msg nick hello`

> If you get “bad password”, ensure LimeChat’s **Server Password** matches the server’s run‑time password.

---

## 🗺️ Supported Commands (subset)

| Command | Summary |
|--------:|---------|
| `PASS <password>` | Must be sent **before** registration completes |
| `NICK <nickname>` | Set or change nickname (must be unique) |
| `USER <user> <mode> * :<realname>` | Complete registration metadata |
| `PING :token` | Keepalive; server replies `PONG :token` |
| `PRIVMSG <target> :<text>` | Send message to user or channel |
| `NOTICE <target> :<text>` | Like `PRIVMSG` (no auto-replies/errors) |
| `JOIN <#chan>[ key]` | Join or create channel (with optional key) |
| `PART <#chan>` | Leave a channel |
| `TOPIC <#chan> [:text]` | Get/set channel topic (honors `+t`) |
| `MODE <#chan> +/-[itkol] [arg]` | Set channel modes (`i,t,k,o,l`) |
| `KICK <#chan> <nick> [:reason]` | Op removes user from channel |
| `INVITE <nick> <#chan>` | Op invites user (needed for `+i`) |
| `WHO [#chan|nick]` | List users / channel members |
| `QUIT [:msg]` | Close the client session |

**Notes**
- Operator is granted via `MODE #chan +o <nick>` by an existing op.
- Channel keys and limits are enforced server-side.
- Replies and numeric errors follow an RFC‑inspired minimal subset.

---

## 🧬 Design Overview

- **Server loop**: non‑blocking listen socket + `poll()` on all fds
- **Per‑client buffers**: accumulate until CRLF; parse into commands
- **Command dispatcher**: routes to handlers with validation
- **Channel registry**: membership, ops, bans/keys/limits (subset)
- **Reply builder**: formats server and numeric response messages
- **Error handling**: invalid syntax, no such nick/channel, not on channel, need more params, etc.

> The focus is correctness, clarity, and robustness for teaching purposes.

---

## 🧑‍🤝‍🧑 Collaboration & Workflow

- Pair programming between **[@salahrachidi](https://github.com/salahrachidi)** and **[@usama-hrb](https://github.com/usama-hrb)**
- **Git** flow with feature branches + PR reviews
- **Issues/Boards** (Jira/Notion) for command specs, modes, and test cases
- **Client test matrix**: LimeChat, `nc`, and scripted fuzz cases
- CI-style local scripts to compile, boot server, and run smoke tests

---

## 🔧 Configuration Tips

- Change the listen port and password at runtime: `./ircserv 7000 mypass`
- Default max clients and message size can be tuned in `Server.hpp`
- Enable verbose logs with `#define DEBUG` (if provided in sources)

---

## 🧭 Roadmap

- User/channel **modes extensions** (bans, quiets, etc.)
- **TLS** support
- Services integration (NickServ‑like flows)
- More conformance to RFC 2812 replies

---

## 🧪 Troubleshooting

- **“No such nick/channel”** → verify spelling and membership
- **“Bad password”** → LimeChat server password must match the server’s password argument
- **Silent disconnects** → check for CRLF endings (`\r\n`) in raw commands
- **Stuck registration** → order must be: `PASS` → `NICK` → `USER`

---

## 🙌 Authors

- **Salaheddine Rachidi** — [@salahrachidi](https://github.com/salahrachidi)  

> Feel free to open issues or PRs for improvements or RFC compliance fixes.
