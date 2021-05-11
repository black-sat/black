#!/bin/bash

set -eu -o pipefail
shopt -s failglob

if [ $# -ne 1 ]; then
  echo Please provide the version number, e.g.
  echo \$ pack.sh 0.4.0
  exit 1
fi

VERSION=$1
SRC_DIR=$(pwd)

die() {
  exit 1
}

prepare() {
  cd "$SRC_DIR"
  docker build docker \
    -f docker/Dockerfile.ubuntu -t black:ubuntu --build-arg GCC_VERSION=10
  docker build docker \
    -f docker/Dockerfile.fedora -t black:fedora --build-arg VERSION=34
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
  mv "$SRC_DIR/build/black-sat-$VERSION-Linux.$ext" \
     "$SRC_DIR/packages/black-sat-$VERSION-1.$(uname -m).$ext" 
}

homebrew() {
  temp=$(mktemp)
  wget -O "$temp" "https://github.com/black-sat/black/archive/v$VERSION.tar.gz"
  sum=$(shasum -a 256 "$temp" | awk '{print $1}')
  
  cat <<END > $SRC_DIR/packages/black-sat.rb
class BlackSat < Formula
  desc "BLACK (Bounded Lᴛʟ sAtisfiability ChecKer)"
  homepage ""
  url "https://github.com/black-sat/black/archive/v$VERSION.tar.gz"
  sha256 "$sum"

  depends_on "cmake" => :build
  depends_on "hopscotch-map" => :build
  depends_on "catch2" => :build
  depends_on "nlohmann-json" => :build
  depends_on "fmt"
  depends_on "z3"
  depends_on "cryptominisat" => :recommended

  def install
    # ENV.deparallelize  # if your formula fails when building in parallel
    system "cmake", ".", *std_cmake_args
    system "make"
    system "make", "install"
  end

  test do
    system "make test"
  end
end
END
}

main () {
  prepare
  build ubuntu 
  build fedora
  homebrew
}

main "$@"