pkgname=fantasti-gameshell
pkgver=0.4.k
pkgrel=1
pkgdesc='fantasti launcher'
arch=(armv7h)
depends=()
source=(
./fantasti
./font.ttf
./fallback.ttf
./fantasti-wl-scan
)
sha1sums=(SKIP
          'bbf4ffb55ff0245a88e185353c010a4edaca80ea'
          'bbf4ffb55ff0245a88e185353c010a4edaca80ea'
          SKIP)
package() {
  cd $srcdir
  install -Dm755 fantasti $pkgdir/usr/local/bin/fantasti
  install -Dm755 fantasti-wl-scan $pkgdir/usr/local/bin/fantasti-wl-scan
  install -Dm644 font.ttf $pkgdir/usr/local/share/fantasti/font.ttf
  install -Dm644 fallback.ttf $pkgdir/usr/local/share/fantasti/fallback.ttf
}
