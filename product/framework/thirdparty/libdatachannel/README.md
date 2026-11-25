# libdatachannel

TemplateTool modules link the `libdatachannel` interface target for WebRTC data
channels. It uses the pinned upstream static library and the repository's
`openssl` target. Media, WebSockets, examples, tests, and libjuice server support
are disabled.

The source files came from these upstream revisions. This snapshot includes the
build files, library sources, headers, and licenses needed for the enabled
configuration.

| Project | Revision | License |
| --- | --- | --- |
| [libdatachannel](https://github.com/paullouisageneau/libdatachannel/releases/tag/v0.24.8) | `6c50b6d88fd691957f1d7df0d80875e9cd57606b` | MPL-2.0 |
| [libjuice](https://github.com/paullouisageneau/libjuice) | `4fa015bb029c254443e01f1ead24360df40ceb2e` | MPL-2.0 |
| [plog](https://github.com/SergiusTheBest/plog) | `94899e0b926ac1b0f4750bfbd495167b4a6ae9ef` | MIT |
| [usrsctp](https://github.com/sctplab/usrsctp) | `6a768c4618ca2442cfda21064075f84ff94c1863` | BSD-3-Clause |

The local upstream CMake change selects TemplateTool's `openssl` target when
building in this repository. The standalone OpenSSL lookup remains available
when that target is absent.
