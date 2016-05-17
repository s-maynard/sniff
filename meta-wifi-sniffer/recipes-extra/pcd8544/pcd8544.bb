SUMMARY = "pcd8544 LCD lib"
HOMEPAGE = "http://binerry.de/post/25787954149/pcd8544-library-for-raspberry-pi"

LICENSE = "Proprietary"
LIC_FILES_CHKSUM = "file://COPYING;md5=d41d8cd98f00b204e9800998ecf8427e"

DEPENDS = "libwiringpi"

SRC_URI = "\
    git://git@github.com/s-maynard/libpcd8544.git;protocol=ssh \
    "
SRCREV = "${AUTOREV}"

S = "${WORKDIR}/git"

inherit autotools
