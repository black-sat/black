# BLACK [![Build Status](https://api.cirrus-ci.com/github/black-sat/black.svg)](https://cirrus-ci.com/github/black-sat/black) ![appveyor](https://ci.appveyor.com/api/projects/status/github/black-sat/black?branch=master&svg=true) ![MIT](https://img.shields.io/badge/license-MIT-brightgreen) [![Latest release](https://badgen.net/github/release/black-sat/black)](https://github.com/black-sat/black/releases/tag/v25.09.0) [![codecov](https://codecov.io/gh/black-sat/black/branch/master/graph/badge.svg?token=ZETQF5NZ6X)](https://codecov.io/gh/black-sat/black)

BLACK (short for Bounded Lᴛʟ sAtisfiability ChecKer) is a tool for testing the
satisfiability of formulas in Linear Temporal Logic and related logics.

BLACK is:
* **Fast**: based on a state-of-the-art SAT-based encoding 
* **Flexible**: supports LTL and LTL+Past, LTLf both on infinite and finite traces, and LTLf Modulo Theories
* **Robust**: stability and correct results via extensive testing
* **Multiplatform**: works on Linux, macOS and Windows
* **Easy to use**: easy to install binary packages provided for all major platforms

## How to use BLACK

See the [Documentation site][Doc] to learn how to use BLACK.

## Downloads

See the [Documentation page][Doc] for further information on BLACK's installation.

### Linux

| Ubuntu ≥ 24.04             | Fedora 42 |
|----------------------------|------------------------------|
| [![Download](https://badgen.net/badge/Download%20v25.09.0/.deb/green)][pkg.deb] | [![Download](https://badgen.net/badge/Download%20v25.09.0/.rpm/green)][pkg.rpm]   |
| `sudo apt install ⟨file⟩` | `sudo dnf install ⟨file⟩` |


### macOS

Install via [Homebrew][brew]:

```
$ brew install black-sat/black/black-sat
```

### Windows

Download the self-contained ZIP archive.

[![Download](https://badgen.net/badge/Download%20v25.09.0/.zip/green)][pkg.win]

[Doc]: https://www.black-sat.org
[brew]: https://brew.sh
[pkg.deb]: https://github.com/black-sat/black/releases/download/v25.09.0/black-sat-25.09.0-1.x86_64.deb
[pkg.rpm]: https://github.com/black-sat/black/releases/download/v25.09.0/black-sat-25.09.0-1.x86_64.rpm
[pkg.win]: https://github.com/black-sat/black/releases/download/v25.09.0/black-25.09.0-win-x64.zip