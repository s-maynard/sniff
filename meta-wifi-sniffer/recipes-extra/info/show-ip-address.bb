DESCRIPTION = "Scripts to print the device IP address."
LICENSE = "CLOSED"

#source: http://offbytwo.com/2008/05/09/show-ip-address-of-vm-as-console-pre-login-message.html

SRC_URI = " \
    file://get-ip-address \
    file://show-ip-address \
"

do_install() {
    install -d ${D}${bindir}
    install -m 0755 ${WORKDIR}/get-ip-address ${D}${bindir}/

    install -d ${D}${sysconfdir}/network/if-up.d/
    install -m 0755 ${WORKDIR}/show-ip-address ${D}${sysconfdir}/network/if-up.d/
}

FILES_${PN} += "\
    ${bindir} \
    ${sysconfdir} \
"

