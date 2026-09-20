# Maintainer: sadaudio <sadaudio@localhost>
# NOTE: AUR publishing is deferred; this PKGBUILD is for local install.

pkgname=sadaudio
pkgver=0.1.0
pkgrel=7
pkgdesc="Pixel-art audio player with equalizer and bilingual UI (Qt6 + libmpv + TagLib)"
arch=('x86_64')
license=('MIT')
depends=('qt6-base' 'mpv' 'taglib')
makedepends=('cmake' 'ninja' 'pkgconf')
source=("sadaudio-$pkgver.tar.gz")
b2sums=('21d8286563e80c99bb685f1a8521ce5ac251186a0659bfc07d6192f1234870e0ca2d47a639b1c0e03721c5a01313a6281f154578126cddb9483a519445bd3f06')

build() {
    cd "$srcdir/$pkgname"
    cmake -B build -G Ninja \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_PREFIX=/usr
    cmake --build build
}

package() {
    cd "$srcdir/$pkgname"
    DESTDIR="$pkgdir" cmake --install build
}