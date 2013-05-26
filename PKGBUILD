# Maintainer: mimas <mimasgpc@free.fr>

_pkgname=obsession
pkgname=$_pkgname-hg
pkgver=17.be73c8ac1bd1
pkgrel=1
pkgdesc="Openbox session tools"
arch=('i686' 'x86_64')
url=http://code.google.com/p/mimarchlinux/
license=('GPL')
depends=('glib2' 'gtk2' 'dbus' 'systemd')
optdepends=('xlock' 'lxdm' 'gdm')
source=(hg+https://code.google.com/p/mimarchlinux.obsession/)
md5sums=('SKIP')

pkgver() {
  cd mimarchlinux.$_pkgname
  echo $(hg identify -n).$(hg identify -i)
}

build() {
  cd $srcdir/mimarchlinux.$_pkgname
  make configure PREFIX=/usr/
  make
}

package() {
  cd $srcdir/mimarchlinux.$_pkgname
  make install PREFIX=$pkgdir/usr
}
