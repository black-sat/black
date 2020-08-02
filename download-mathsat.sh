#!/bin/bash

MATHSAT_VERSION=5.5.4

die() {
  echo $1 >&2
  exit 1
}

detect_downloader() {
  if command -v wget >/dev/null 2>&1; then
    DOWNLOADER=wget
    download() {
      wget -q -O - "$1"
    }
  else
    die 'This script needs the `wget` command installed.'
  fi
}

detect_system() {
  [ "$(uname -m)" == "x86_64" ] || \
    die "MathSAT 5 is available for download only for 64bit systems."

  SYSTEM="$(uname -s)-$(uname -m)"
  if [ "$(uname -s)" == "Linux" ]; then
    MATHSAT_DIR=mathsat-$MATHSAT_VERSION-linux-x86_64
  elif [ "$(uname -s)" == "Darwin" ]; then
    MATHSAT_DIR=mathsat-$MATHSAT_VERSION-darwin-libcxx-x86_64
  else
    die "There is no MathSAT 5 pre-compiled distribution for your platform."
  fi
}

main()
{
  detect_downloader
  detect_system

  ORIGINAL_PWD=$(pwd)

  cd "$(git rev-parse --show-toplevel)/external" || \
    die "Please run the script from inside BLACK's sources"

  MATHSAT_URL=http://mathsat.fbk.eu/download.php?file=$MATHSAT_DIR.tar.gz
  MATHSAT_DEST=mathsat5-$SYSTEM

  [ -d "$MATHSAT_DEST" ] && \
    die "A MathSAT directory seems to exist already: $MATHSAT_DEST"

  echo "Using $DOWNLOADER to download MathSAT $MATHSAT_VERSION for $SYSTEM..."

  download $MATHSAT_URL | tar xz && [ -d "$MATHSAT_DIR" ] || \
    die "Unable to download and/or unpack MathSAT"

  mv "$MATHSAT_DIR" "$MATHSAT_DEST"

  cd $ORIGINAL_PWD

  echo "MathSAT downloaded."

  exit 0
}

main
