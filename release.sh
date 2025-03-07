#!/bin/bash

set -eu -o pipefail
shopt -s failglob

die() {
  echo "$@"
  exit 1
}

dependencies() {
  which wget >/dev/null || die "Missing wget"
  which curl >/dev/null || die "Missing curl"
  which jq   >/dev/null || die "Missing jq"
  which gh   >/dev/null || die "Missing gh"

  test -f ~/.appveyor.token || die "Missing ~/.appveyor.token"
}

setup() {
  SRC_DIR=$(git rev-parse --show-toplevel)
  cd "$SRC_DIR"
  
  VERSION=$(cat CMakeLists.txt | grep -E '^\s+VERSION' | awk '{print $2}')
}

images() {
  docker build docker \
    -f docker/Dockerfile.ubuntu --platform linux/amd64 -t black:ubuntu24.04 \
    --build-arg VERSION=24.04 --build-arg GCC_VERSION=14
  docker build docker \
    -f docker/Dockerfile.fedora --platform linux/amd64 -t black:fedora41 \
    --build-arg VERSION=41
}

launch() {
  image=$1
  shift
  ver=$1
  shift
  if [ "$1" = "root" ]; then
    user=''
    shift
  else
    user="-u $(id -u)"
  fi

  docker run --platform linux/amd64 -it --rm \
             -v $SRC_DIR:/black -w /black/build $user $image$ver "$@"
}

ubuntu() {
  launch black:ubuntu "$@"
}

fedora() {
  launch black:fedora "$@"
}

build() {
  distro=$1
  ver=$2
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

  env="$distro $ver"

  rm -rf "$SRC_DIR/build"
  mkdir build
  $env cmake -DENABLE_CMSAT=NO -DENABLE_CVC5=NO .. || die
  $env make -j || die
  $env cpack -G $gen || die

  mkdir -p "$SRC_DIR/packages/$VERSION"
  mv "$SRC_DIR/build/black-sat-$VERSION-Linux.$ext" \
     "$SRC_DIR/packages/$VERSION/black-sat-$VERSION.$distro$ver.x86_64.$ext" 
}

test_pkg() {
  distro=$1
  ver=$2

  case $distro in
    ubuntu)
      cmd="apt update && apt install /black/packages/$VERSION/black-sat-$VERSION.$distro$ver.x86_64.deb && black --help"
    ;;
    fedora)
      cmd="yum -y install /black/packages/$VERSION/black-sat-$VERSION.$distro$ver.x86_64.rpm && black --help"
    ;;
  esac

  $distro $ver root bash -c "$cmd" || die
}

appveyor() {
  AV_API=https://ci.appveyor.com/api
  AV_TOKEN=$(cat ~/.appveyor.token)
  AV_USER=nicola-gigante
  AV_REPO=black
  AV_PROJECTS_URL=$AV_API/projects/$AV_USER/$AV_REPO

  api() {
    curl -sS -L --header "Content-type: application/json" --header "Authorization: Bearer $AV_TOKEN" "$@"
  }

  CURRENT_COMMIT=$(git rev-parse HEAD)
  BUILD_COMMIT=$(api "$AV_PROJECTS_URL" | jq -r .build.commitId)
  if [ "$CURRENT_COMMIT" != "$BUILD_COMMIT" ]; then
    echo Latest Appveyor build is not for the current commit. Aborting...
    die
  fi
  JOB_ID=$(api "$AV_PROJECTS_URL" | jq -r '.build.jobs[0].jobId')
  
  AV_ARTIFACTS_URL=$AV_API/buildjobs/$JOB_ID/artifacts
  N_ARTIFACTS=$(api "$AV_ARTIFACTS_URL" | jq '. | length')
  if [ "$N_ARTIFACTS" -eq 0 ]; then
    echo No artifact available in Appveyor. Aborting...
    die
  fi
  api "$AV_ARTIFACTS_URL"/black-win-x64.zip \
    -o packages/$VERSION/black-$VERSION-win-x64.zip
}

release() {
  gh auth login -p ssh --web
  temp=$(mktemp)
  vim $temp
  if [ -z "$(cat $temp)" ]; then 
    echo "Empty release notes. Quitting..."
    exit 0
  fi
  gh release create --notes-file $temp -t v$VERSION --target master v$VERSION

  for file in packages/$VERSION/*.{rpm,deb,zip}; do
    gh release upload v$VERSION $file
  done
}

homebrew() {
  temp=$(mktemp)
  wget -O "$temp" "https://github.com/black-sat/black/archive/v$VERSION.tar.gz"
  sum=$(shasum -a 256 "$temp" | awk '{print $1}')
  
  cat <<END > $SRC_DIR/packages/$VERSION/black-sat.rb
class BlackSat < Formula
  desc "BLACK (Bounded Lᴛʟ sAtisfiability ChecKer)"
  homepage "https://www.black-sat.org"
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
  dependencies
  setup
  images

  arg=$1

  case "$1" in
    build-only)
      build ubuntu 24.04
      test_pkg ubuntu 24.04
      build fedora 41
      test_pkg fedora 41
      appveyor
    ;;
    upload-only)
      release
      homebrew
    ;;
    all)
      build ubuntu 24.04
      test_pkg ubuntu 24.04
      build fedora 41
      test_pkg fedora 41
      appveyor
      release
      homebrew
  esac
}

main "$@"