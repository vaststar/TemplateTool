# standalone Asio

- Version: 1.38.2
- Upstream: https://github.com/chriskohlhoff/asio
- Source tag: `asio-1-38-2`
- Download archive SHA-256: `9f2648fa483e58a6bf848d970ee0ea650ca19ed7769dfa520ed4f7b8d27af1db`
- License: Boost Software License 1.0 (see `asio-1.38.2/LICENSE_1_0.txt`)

Only the upstream public headers and license/readme files are vendored. The
local CMake target named `asio` exposes them in standalone, header-only mode.
Consumers should link the target privately and must not expose Asio types from
first-party public headers.
