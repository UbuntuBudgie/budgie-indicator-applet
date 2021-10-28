#!/bin/bash
set -e

git submodule init
git submodule update

VERSION="0.7.1"
NAME="budgie-indicator-applet"
git-archive-all --prefix ${NAME}-${VERSION}/ --verbose  ${NAME}-${VERSION}.tar
xz -9 "${NAME}-${VERSION}.tar"

gpg --armor --detach-sign "${NAME}-${VERSION}.tar.xz"
gpg --verify "${NAME}-${VERSION}.tar.xz.asc"

