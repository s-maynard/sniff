DESCRIPTION = "Scripts to support a WLAN setup on the Raspberry Pi."

FILESEXTRAPATHS_prepend := "${THISDIR}/rpi-first-run-setup:"

LICENSE = "GPLv2"
LIC_FILES_CHKSUM = "file://${WORKDIR}/LICENSE;md5=b234ee4d69f5fce4486a80fdaf4a4263"

COMPATIBLE_MACHINE = "raspberrypi"

RDEPENDS_${PN} = "sudo"

SRC_URI = " \
    file://LICENSE \
    file://wlan-ctl \
    file://wpa_supplicant.conf \
"

inherit update-rc.d

INITSCRIPT_PACKAGES = "${PN}"
INITSCRIPT_NAME = "${PN}"
INITSCRIPT_PARAMS = "start 98 2 3 4 5 . stop 80 0 6 1 ."

do_install() {
    install -d ${D}${sysconfdir}/init.d/
    install -m 0755 ${WORKDIR}/wlan-ctl ${D}${sysconfdir}/init.d/
    install -m 0755 ${WORKDIR}/wpa_supplicant.conf ${D}${sysconfdir}/
}

PACKAGE_ARCH = "${MACHINE_ARCH}"

FILES_${PN} += "\
    ${prefix} \
    ${sysconfdir}/wpa_supplicant.conf \
    ${sysconfdir}/init.d/wlan-ctl \
"

