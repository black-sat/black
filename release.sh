#!/bin/bash

set -eu -o pipefail
shopt -s failglob

die() {
  echo "$@"
  exit 1
}

dependencies() {
  which wget || die "Missing wget"
  which curl || die "Missing curl"
  which jq   || die "Missing jq"
  which gh   || die "Missing gh"
}

setup() {
  SRC_DIR=$(git rev-parse --show-toplevel)
  cd "$SRC_DIR"
  
  VERSION=$(cat CMakeLists.txt | grep -E '^\s+VERSION' | awk '{print $2}')
}

images() {
  docker build docker \
    -f docker/Dockerfile.ubuntu -t black:ubuntu
  docker build docker \
    -f docker/Dockerfile.fedora -t black:fedora
}

launch() {
  image=$1
  shift
  if [ "$1" = "root" ]; then
    user=''
    shift
  else
    user="-u $(id -u)"
  fi

  docker run -it --rm -v $SRC_DIR:/black -w /black/build $user $image "$@"
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
  $env cmake -DENABLE_CMSAT=NO -DENABLE_CVC5=NO .. || die
  $env make -j || die
  $env cpack -G $gen || die

  mkdir -p "$SRC_DIR/packages"
  mv "$SRC_DIR/build/black-sat-$VERSION-Linux.$ext" \
     "$SRC_DIR/packages/black-sat-$VERSION-1.$(uname -m).$ext" 
}

test_pkg() {
  env=$1
  case $1 in
    ubuntu)
      cmd="apt update && apt install /black/packages/black-sat-$VERSION-1.$(uname -m).deb && black --help"
    ;;
    fedora)
      cmd="yum -y install /black/packages/black-sat-$VERSION-1.$(uname -m).rpm && black --help"
    ;;
  esac

  $env root bash -c "$cmd" || die
}

appveyor() {
  AV_API=https://ci.appveyor.com/api
  AV_TOKEN=$(cat appveyor.token)
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
  api "$AV_ARTIFACTS_URL"/black-win-x64.zip -o packages/black-$VERSION-win-x64.zip
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
  gh release upload v$VERSION packages/black-$VERSION-win-x64.zip
}

homebrew() {
  temp=$(mktemp)
  wget -O "$temp" "https://github.com/black-sat/black/archive/v$VERSION.tar.gz"
  sum=$(shasum -a 256 "$temp" | awk '{print $1}')
  
  cat <<END > $SRC_DIR/packages/black-sat.rb
class BlackSat < Formula
  desc "BLACK (Bounded Lᴛʟ sAtisfiability ChecKer)"
  homepage "https://www.black-sat.org"
  url "https://github.com/black-sat/black/archive/v$VERSION.tar.gz"
  sha256 "$sum"

  depends_on "llvm" => :build
  depends_on "cmake" => :build
  depends_on "hopscotch-map" => :build
  depends_on "catch2" => :build
  depends_on "cereal"
  depends_on "fmt"
  depends_on "z3"
  depends_on "cryptominisat" => :recommended

  def install
    ENV["CC"]=Formula["llvm"].opt_bin/"clang"
    ENV["CXX"]=Formula["llvm"].opt_bin/"clang++"
    ENV["LDFLAGS"]="-L#{Formula["llvm"].opt_lib} -Wl,-rpath,#{Formula["llvm"].opt_lib}"
    ENV["CXXFLAGS"]="-I#{Formula["llvm"].opt_include}"
    system "cmake", ".", "-DENABLE_MINISAT=NO", *std_cmake_args
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
  build ubuntu 
  test_pkg ubuntu
  build fedora
  test_pkg fedora
  appveyor
  release
  homebrew
}

main "$@"