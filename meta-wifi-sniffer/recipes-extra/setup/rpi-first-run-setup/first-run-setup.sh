#!/bin/sh

# Initially based on the scripts by JohnX/Mer Project - http://wiki.maemo.org/Mer/
# Reworked for the OpenPandora - John Willis/Michael Mrozek
# Quickly 'hacked' for the Raspberry Pi to provide a simple 1st boot wizard.

# Modified by - Carey Sonsino

# Ensure there is a wheel group for sudoers to be put into.
# TODO: Do this somewhere better.
groupadd wheel

# Set the root password
rootusername="root"
rootpassword="raspiroot"
passwd "$rootusername" <<EOF
$rootpassword
$rootpassword
EOF

# Set up the user account
fullname="raspberry pi"
username="pi"
password="raspberry"

useradd -c "$fullname,,," -G wheel,users "$username"

passwd "$username" <<EOF
$password
$password
EOF

# Add the new user to the sudoers list
echo "$username ALL=(ALL:ALL) ALL" >> /etc/sudoers

# Update the environment PATH
# Note - /sbin and /usr/sbin are only added to the path for the root user.  The following
# export adds those directories for the newly added user
echo "export PATH=$PATH" >> /home/$username/.bashrc
echo "alias l='ls -CF'" >> /home/$username/.bashrc
echo "alias la='ls -A'" >> /home/$username/.bashrc
echo "alias ll='ls -alF'" >> /home/$username/.bashrc
#echo "export PATH=$PATH:/usr/sbin:/sbin" >> /home/$username/.bashrc
#if [[ "$PATH" != "*sbin*" ]] ; then
#    export PATH="/sbin:/usr/sbin:$PATH" >> /home/$username/.bash_profile
#fi

echo "ctrl_interface=/var/run/wpa_supplicant" > /etc/wpa_supplicant.conf
echo "ctrl_interface_group=0" >> /etc/wpa_supplicant.conf
echo "update_config=1" >> /etc/wpa_supplicant.conf
echo "network={" >> /etc/wpa_supplicant.conf
echo "  ssid=\"TenerVista-5G\"" >> /etc/wpa_supplicant.conf
echo "  proto=WPA RSN" >> /etc/wpa_supplicant.conf
echo "  scan_ssid=1" >> /etc/wpa_supplicant.conf
echo "  key_mgmt=WPA-PSK" >> /etc/wpa_supplicant.conf
echo "  pairwise=CCMP TKIP" >> /etc/wpa_supplicant.conf
echo "  group=CCMP TKIP" >> /etc/wpa_supplicant.conf
echo "  psk=2af52545151de2d6c57e99a765499768a4435de78de82f13cb7493f98cbe68c2" >> /etc/wpa_supplicant.conf
echo "}" >> /etc/wpa_supplicant.conf
echo "network={" >> /etc/wpa_supplicant.conf
echo "  ssid=\"TenerVista-2.4G\"" >> /etc/wpa_supplicant.conf
echo "  proto=WPA RSN" >> /etc/wpa_supplicant.conf
echo "  scan_ssid=1" >> /etc/wpa_supplicant.conf
echo "  key_mgmt=WPA-PSK" >> /etc/wpa_supplicant.conf
echo "  pairwise=CCMP TKIP" >> /etc/wpa_supplicant.conf
echo "  group=CCMP TKIP" >> /etc/wpa_supplicant.conf
echo "  psk=d58eaa7c52383b9ddc595e97e9bd207e048a2a625c411eff6f0920a185e36ce8" >> /etc/wpa_supplicant.conf
echo "}" >> /etc/wpa_supplicant.conf

# Lock the root account
usermod -L "$rootusername"

# Pick a name for the machine.

hostname="sniffer"
hostname_file="/etc/hostname"
hosts_file="/etc/hosts"

echo $hostname > $hostname_file
hostname =$(sed 's/ /_/g' $hostname_file)
echo $hostname > $hostname_file
echo "127.0.0.1 localhost.localdomain localhost $hostname" > $hosts_file
hostname -F $hosts_file

control_file="/etc/rpi/first-boot"

# Write the control file so this script is not run on next boot 
# (hackish I know but I want the flexability to drop a new script in later esp. in the early firmwares).

touch $control_file
# Prevent the control file from being erased, which would rerun the initial setup on next boot
chmod 0444 $control_file

# Remove THIS script (especially since it contains sensitive info)
rm -f $0

reboot

