# BLACK [![Build Status](https://api.cirrus-ci.com/github/black-sat/black.svg)](https://cirrus-ci.com/github/black-sat/black) ![appveyor](https://ci.appveyor.com/api/projects/status/github/black-sat/black?branch=master&svg=true) ![MIT](https://img.shields.io/badge/license-MIT-brightgreen) [![Latest release](https://badgen.net/github/release/black-sat/black)](https://github.com/black-sat/black/releases/tag/v0.10.8) [![codecov](https://codecov.io/gh/black-sat/black/branch/master/graph/badge.svg?token=ZETQF5NZ6X)](https://codecov.io/gh/black-sat/black)

BLACK (short for Bounded Lᴛʟ sAtisfiability ChecKer) is a tool for testing the
satisfiability of formulas in Linear Temporal Logic and related logics.

BLACK is:
* **Fast**: based on a state-of-the-art SAT-based encoding 
* **Lightweight**: low memory consuption even for large formulas
* **Flexible**: supports LTL and LTL+Past, LTLf both on infinite and finite models, and LTLf Modulo Theories
* **Robust**: rock-solid stability with almost 100% test coverage
* **Multiplatform**: works on Linux, macOS, Windows and FreeBSD
* **Easy to use**: easy to install binary packages provided for all major platforms
* **Embeddable**: use BLACK's library API to integrate BLACK's solver into your code

## How to use BLACK

See the [Documentation site][Doc] to learn how to use BLACK.

## Downloads

See the [Documentation page][Doc] for further information on BLACK's installation.

### Linux

| Ubuntu ≥ 22.04             | Fedora 36 |
|----------------------------|------------------------------|
| [![Download](https://badgen.net/badge/Download%20v0.10.8/.deb/green)][pkg.deb] | [![Download](https://badgen.net/badge/Download%20v0.10.8/.rpm/green)][pkg.rpm]   |
| `sudo apt install ⟨file⟩` | `sudo dnf install ⟨file⟩` |


### macOS

Install via [Homebrew][brew]:

```
$ brew install black-sat/black/black-sat
```

### Windows

Download the self-contained ZIP archive.

[![Download](https://badgen.net/badge/Download%20v0.10.8/.zip/green)][pkg.win]

[Doc]: https://www.black-sat.org
[brew]: https://brew.sh
[pkg.deb]: https://github.com/black-sat/black/releases/download/v0.10.8/black-sat-0.10.8-1.x86_64.deb
[pkg.rpm]: https://github.com/black-sat/black/releases/download/v0.10.8/black-sat-0.10.8-1.x86_64.rpm
[pkg.win]: https://github.com/black-sat/black/releases/download/v0.10.8/black-0.10.8-win-x64.zip