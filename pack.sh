#!/bin/bash

set -eu -o pipefail
shopt -s failglob

SRC_DIR=$(pwd)

die() {
  exit 1
}

prepare() {
  cd "$SRC_DIR"
  docker build docker \
    -f docker/Dockerfile.ubuntu -t black:ubuntu --build-arg GCC_VERSION=10
  docker build docker \
    -f docker/Dockerfile.fedora -t black:fedora --build-arg VERSION=32
}

launch() {
  docker run -it --rm -v $SRC_DIR:/black -w /black/build -u $(id -u) "$@"
}

ubuntu() {
  launch black:ubuntu "$@"
}

fedora() {
  launch black:fedora "$@"
}

build() {
  env=$1
  case $1 in 
    ubuntu)
    gen=DEB
    ext=deb
    ;;
    fedora)
    gen=RPM
    ext=rpm
    ;;
  esac

  rm -rf "$SRC_DIR/build"
  mkdir build
  $env cmake -DENABLE_CMSAT=NO .. || die
  $env make -j18 || die
  $env cpack -G $gen || die

  mkdir -p "$SRC_DIR/packages"
  mv "$SRC_DIR"/build/black-sat-*.$ext "$SRC_DIR/packages" 
}

main () {
  prepare
  build ubuntu 
  build fedora
}

main "$@"