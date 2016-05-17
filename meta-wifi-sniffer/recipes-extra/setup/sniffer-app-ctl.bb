DESCRIPTION = "Scripts to support a sniffer app setup on the Raspberry Pi."

FILESEXTRAPATHS_prepend := "${THISDIR}/rpi-first-run-setup:"

LICENSE = "GPLv2"
LIC_FILES_CHKSUM = "file://${WORKDIR}/LICENSE;md5=b234ee4d69f5fce4486a80fdaf4a4263"

#COMPATIBLE_MACHINE = "raspberrypi raspberrypi2"

RDEPENDS_${PN} = "sudo"

SRC_URI = " \
    file://LICENSE \
    file://sniffer-app-ctl \
"

inherit update-rc.d

INITSCRIPT_PACKAGES = "${PN}"
INITSCRIPT_NAME = "${PN}"
INITSCRIPT_PARAMS = "start 99 2 3 4 5 . stop  80 0 6 1 ."

do_install() {
    install -d ${D}${sysconfdir}/init.d/
    install -m 0755 ${WORKDIR}/sniffer-app-ctl ${D}${sysconfdir}/init.d/
}

PACKAGE_ARCH = "${MACHINE_ARCH}"

FILES_${PN} += "\
    ${prefix} \
    ${sysconfdir}/init.d/sniffer-app-ctl \
"

