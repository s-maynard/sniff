SUMMARY = "sniffer app component"
HOMEPAGE = "http://github.com/s-maynard/sniff"

LICENSE = "Proprietary"
LIC_FILES_CHKSUM = "file://COPYING;md5=d41d8cd98f00b204e9800998ecf8427e"

DEPENDS = "pcd8544 witapp"

SRC_URI = "\
    git://git@github.com/s-maynard/sniffer.git;protocol=ssh;branch=master \
    "
SRCREV = "${AUTOREV}"

S = "${WORKDIR}/git"

inherit autotools

do_install_append () {
    install -d ${D}/usr/sbin
    install -m 755 ${WORKDIR}/git/wit_watchdog.sh ${D}/usr/sbin
}

FILES_${PN} = " \
    /usr/bin/sniff \
    /usr/sbin/wit_watchdog.sh \
"
