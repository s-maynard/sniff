SUMMARY = "wiringPi gpio lib"
HOMEPAGE = "https://projects.drogon.net/raspberry-pi/wiringpi"

LICENSE = "Proprietary"
LIC_FILES_CHKSUM = "file://COPYING;md5=d41d8cd98f00b204e9800998ecf8427e"

DEPENDS = ""

SRC_URI = "\
    git://git@github.com/s-maynard/libwiringpi.git;protocol=ssh \
    "
SRCREV = "${AUTOREV}"

S = "${WORKDIR}/git"

inherit autotools
