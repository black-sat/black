#!/bin/bash

set -eu -o pipefail
shopt -s failglob

die() {
  exit 1
}

setup() {
  SRC_DIR=$(git rev-parse --show-toplevel)
  VERSION=$(cat CMakeLists.txt | grep -E '^\s+VERSION' | awk '{print $2}')

  cd "$SRC_DIR"
}

images() {
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
  $env make -j || die
  $env cpack -G $gen || die

  mkdir -p "$SRC_DIR/packages"
  mv "$SRC_DIR/build/black-sat-$VERSION-Linux.$ext" \
     "$SRC_DIR/packages/black-sat-$VERSION-1.$(uname -m).$ext" 
}

release() {
  cat gh.token | gh auth login -p ssh --with-token
  temp=$(mktemp)
  vim $temp
  if [ -z "$(cat $temp)" ]; then 
    echo "Empty release notes. Quitting..."
    exit 0
  fi
  gh release create --notes-file $temp -p -t v$VERSION --target master v$VERSION
  gh release upload v$VERSION packages/black-sat-$VERSION-1.x86_64.deb
  gh release upload v$VERSION packages/black-sat-$VERSION-1.x86_64.rpm
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
  setup
  images
  build ubuntu 
  build fedora
  release
  # homebrew
}

main "$@"