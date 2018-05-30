# Maintainer: Arthur Williams <taaparthur@gmail.com>


pkgname='mpx-patch'
pkgver='1.1.2'
_language='en-US'
pkgrel=1
pkgdesc='Control mouse from keyboard'

arch=('any')
license=('MIT')
depends=('xorg-server' 'libx11')
md5sums=('SKIP')

source=("git://github.com/TAAPArthur/MPX-Patch.git")
_srcDir="MPX-Patch"
build(){
  cd "$_srcDir"
  gcc -o mpx-patch -std=c99 -Os  MPXPatch.c -s -lX11 -lXtst -lXi
}
package() {

  cd "$_srcDir"
  mkdir -p "$pkgdir/usr/bin/"
  mkdir -p "$pkgdir/usr/lib/$pkgname/"
  install -D -m 0755 "mpx-patch" "$pkgdir/usr/bin/"
}
