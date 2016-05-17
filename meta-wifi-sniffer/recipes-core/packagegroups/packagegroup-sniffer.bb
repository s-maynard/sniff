SUMMARY = "Custom package group for Wireless Information Tool"

LICENSE = "MIT"

inherit packagegroup

PACKAGES = "\
    packagegroup-sniffer \
    packagegroup-sniffer-debug \
"

# Components used in sniffer
RDEPENDS_packagegroup-sniffer = "\
    systemd-compat-units \
    linux-firmware \
    psplash \
    telnetd \
    sshd \
    iperf3 \
    cronie \
    parted \
    binutils \
    binutils-symlinks \
    wlan-ctl \
    packagegroup-base-wifi \
    bridge-utils \
    iw \
    less \
    sqlite3 \
    networkmanager \
    ntp \
    ntpdate \
    perl \
    python-dev \
    python-imaging \
    witapp \
    sniff \
    sniffer-app-ctl \
"

# Components used in debug sniffer
RDEPENDS_packagegroup-sniffer-debug = "\
    git \
    autoconf \
    automake \
    cpp \
    cpp-symlinks \
    gcc \
    gcc-symlinks \
    g++ \
    g++-symlinks \
    make \
    libstdc++ \
    libstdc++-dev \
    gdb \
    gdbserver \
    vim \
"

#Note: psplash is not required, but it adds a nice "yocto project" splash screen and loading bar

