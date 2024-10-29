# All this because of a packaging bug in libboost-log-dev
# 'Multi-Arch: same' is not present in the control file of Debian packages, all the way up to Sid
# Rather than recompiling the parent libbost suite, download the 6 packages, extract, edit, and repackage.

# https://bugs.debian.org/cgi-bin/pkgreport.cgi?package=libboost-log-dev

apt-get -y install --download-only libboost-log-dev:amd64 libboost-log1.67.0:amd64 libboost-log1.67-dev:amd64
apt-get -y install --download-only libboost-log-dev:armhf libboost-log1.67.0:armhf libboost-log1.67-dev:armhf

mkdir -p /tmp/armhf/libboost-log-dev/
dpkg-deb -x /var/cache/apt/archives/libboost-log-dev_1.67.0.1_armhf.deb /tmp/armhf/libboost-log-dev/
dpkg-deb -e /var/cache/apt/archives/libboost-log-dev_1.67.0.1_armhf.deb /tmp/armhf/libboost-log-dev/DEBIAN/
sed -i '/^Priority: optional/a Multi-Arch: same' /tmp/armhf/libboost-log-dev/DEBIAN/control
dpkg-deb -b /tmp/armhf/libboost-log-dev/ /tmp/libboost-log-dev_1.67.0.1_armhf.deb

mkdir -p /tmp/amd64/libboost-log-dev/
dpkg-deb -x /var/cache/apt/archives/libboost-log-dev_1.67.0.1_amd64.deb /tmp/amd64/libboost-log-dev/
dpkg-deb -e /var/cache/apt/archives/libboost-log-dev_1.67.0.1_amd64.deb /tmp/amd64/libboost-log-dev/DEBIAN/
sed -i '/^Priority: optional/a Multi-Arch: same' /tmp/amd64/libboost-log-dev/DEBIAN/control
dpkg-deb -b /tmp/amd64/libboost-log-dev/ /tmp/libboost-log-dev_1.67.0.1_amd64.deb

mkdir -p /tmp/armhf/libboost-log1.67-dev/
dpkg-deb -x /var/cache/apt/archives/libboost-log1.67-dev_1.67.0-13+deb10u1_armhf.deb /tmp/armhf/libboost-log1.67-dev/
dpkg-deb -e /var/cache/apt/archives/libboost-log1.67-dev_1.67.0-13+deb10u1_armhf.deb /tmp/armhf/libboost-log1.67-dev/DEBIAN/
sed -i '/^Priority: optional/a Multi-Arch: same' /tmp/armhf/libboost-log1.67-dev/DEBIAN/control
dpkg-deb -b /tmp/armhf/libboost-log1.67-dev/ /tmp/libboost-log1.67-dev_1.67.0-13+deb10u1_armhf.deb

mkdir -p /tmp/amd64/libboost-log1.67-dev/
dpkg-deb -x /var/cache/apt/archives/libboost-log1.67-dev_1.67.0-13+deb10u1_amd64.deb /tmp/amd64/libboost-log1.67-dev/
dpkg-deb -e /var/cache/apt/archives/libboost-log1.67-dev_1.67.0-13+deb10u1_amd64.deb /tmp/amd64/libboost-log1.67-dev/DEBIAN/
sed -i '/^Priority: optional/a Multi-Arch: same' /tmp/amd64/libboost-log1.67-dev/DEBIAN/control
dpkg-deb -b /tmp/amd64/libboost-log1.67-dev/ /tmp/libboost-log1.67-dev_1.67.0-13+deb10u1_amd64.deb

mkdir -p /tmp/armhf/libboost-log1.67.0/
dpkg-deb -x /var/cache/apt/archives/libboost-log1.67.0_1.67.0-13+deb10u1_armhf.deb /tmp/armhf/libboost-log1.67.0/
dpkg-deb -e /var/cache/apt/archives/libboost-log1.67.0_1.67.0-13+deb10u1_armhf.deb /tmp/armhf/libboost-log1.67.0/DEBIAN/
sed -i '/^Priority: optional/a Multi-Arch: same' /tmp/armhf/libboost-log1.67.0/DEBIAN/control
dpkg-deb -b /tmp/armhf/libboost-log1.67.0/ /tmp/libboost-log1.67.0_1.67.0-13+deb10u1_armhf.deb

mkdir -p /tmp/amd64/libboost-log1.67.0/
dpkg-deb -x /var/cache/apt/archives/libboost-log1.67.0_1.67.0-13+deb10u1_amd64.deb /tmp/amd64/libboost-log1.67.0/
dpkg-deb -e /var/cache/apt/archives/libboost-log1.67.0_1.67.0-13+deb10u1_amd64.deb /tmp/amd64/libboost-log1.67.0/DEBIAN/
sed -i '/^Priority: optional/a Multi-Arch: same' /tmp/amd64/libboost-log1.67.0/DEBIAN/control
dpkg-deb -b /tmp/amd64/libboost-log1.67.0/ /tmp/libboost-log1.67.0_1.67.0-13+deb10u1_amd64.deb

dpkg -i /tmp/libboost*.deb