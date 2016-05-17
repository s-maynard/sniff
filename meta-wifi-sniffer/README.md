meta-wifi-sniffer
==============

This README file contains information on configuring and building the Wireless Information Tool

To build for either:<br>
1) $ <b><i>git apply meta-wifi-sniffer/yocto_patches/add_build_info.patch</i></b><br>


To build for the Raspberry Pi:<br>
1) $ <b><i>cd ~</i></b><br>
2) $ <b><i>git clone -b fido git://git.yoctoproject.org/poky.git</i></b><br>
3) $ <b><i>git clone https://github.com/s-maynard/wit.git</i></b><br>
4) $ <b><i>git clone https://github.com/s-maynard/sniffer.git</i></b><br>
5) $ <b><i>cd poky</i></b><br>
6) $ <b><i>git clone -b fido git://git.openembedded.org/meta-openembedded.git</i></b><br>
7) $ <b><i>git clone -b master git://git.yoctoproject.org/meta-raspberrypi.git</i></b><br>
8) $ <b><i>MACHINE=raspberrypi source oe-init-build-env</i></b><br>
9) edit conf/bblayers.conf, ensuring that the following layers are present in BBLAYERS:<br>
<b>NOTE</b> - You must replace "~" with the fully qualified path (/home/&lt;your_username&gt;)<br>
  ~/wit/meta-wit \\<br>
  ~/sniffer/meta-wifi-sniffer \\<br>
  ~/poky/meta-raspberrypi \\<br>
  ~/poky/meta-openembedded/meta-networking \\<br>
  ~/poky/meta-openembedded/meta-oe \\<br>
10) $ <b><i>MACHINE=raspberrypi bitbake sniffer-image</i></b><br><br>


To build for the Raspberry Pi 2:<br>
1) $ <b><i>cd ~</i></b><br>
2) $ <b><i>git clone -b fido git://git.yoctoproject.org/poky.git</i></b><br>
3) $ <b><i>git clone https://github.com/s-maynard/wit.git</i></b><br>
4) $ <b><i>git clone https://github.com/s-maynard/sniffer.git</i></b><br>
5) $ <b><i>cd poky</i></b><br>
6) $ <b><i>git clone -b fido git://git.openembedded.org/meta-openembedded.git</i></b><br>
7) $ <b><i>git clone -b master git://git.yoctoproject.org/meta-raspberrypi.git</i></b><br>
8) $ <b><i>MACHINE=raspberrypi2 source oe-init-build-env</i></b><br>
9) edit conf/bblayers.conf, ensuring that the following layers are present in BBLAYERS:<br>
<b>NOTE</b> - You must replace "~" with the fully qualified path (/home/&lt;your_username&gt;)<br>
  ~/wit/meta-wit \\<br>
  ~/sniffer/meta-wifi-sniffer \\<br>
  ~/poky/meta-raspberrypi \\<br>
  ~/poky/meta-openembedded/meta-networking \\<br>
  ~/poky/meta-openembedded/meta-oe \\<br>
  ~/poky/meta-openembedded/meta-python \\<br>
10) $ <b><i>MACHINE=raspberrypi2 bitbake sniffer-image</i></b><br><br>



Additional Information
==============

- The boot splashscreen is located in <b>recipes-core/psplash</b>.  To modify the image, replace <b>psplash-img.png</b><br>
- The IP address information printed above the login prompt is controlled by the scripts in <b>recipes-extra/info</b><br>
- The user creation on first boot is controlled by the scripts in <b>recipes-extra/setup</b><br>
- Build information will be stored on the image root filesystem in <b>/etc/build</b>.  The exporting of the build information requires the patches in <b>yocto_patches</b> because that functionality is not currently part of the <i>daisy</i> branch<br>

#### To dd a RaspberryPi image from mac shared folder to sdcard
- from the VM on the mac:<br>
sudo cp tmp/deploy/images/raspberrypi2/sniffer-raspberrypi.rpi-sdimg /media/sf_shared/<br>
- from the terminal on the mac (after the sdcard has been inserted):<br>
sudo diskutil unmountDisk /dev/disk2<br>
sudo dd if=/Users/smaynard/shared/sniffer-raspberrypi.rpi-sdimg of=/dev/rdisk2<br>
sudo diskutil unmountDisk /dev/disk2<br>
