# Arch Linux

## Install
### clean up useless software (manjaro)
```sh
sudo pacman -Rscn manjaro-hello hexchat libcdr evolution parted brasero hplip imagewriter lollypop
```

### software source settings
```
# /etc/pacman.conf:
[archlinuxcn]
SigLevel = Optional TrustedOnly
Server = http://mirrors.xdlinux.info/archlinuxcn/$arch

# /etc/pacman.d/mirrorlist:
Server = https://mirrors.xdlinux.info/archlinux/$repo/os/$arch
```

### install MATLAB using an ISO image
Note that you must be a license administrator to download an ISO archive of the MATLAB installer.

1.Create a folder to use as a mount point.
```
# mkdir /run/media/mathworks
```

2.Mount the ISO to that folder. In this example, the ISO is called “matlab.iso” and is located in the user’s home folder under Downloads:
```
# mount -t iso9660 -o loop ~/downloads/matlab.iso /run/media/mathworks
```

3.Navigate to the mount point.
```
# cd /run/media/mathworks
```

4.Run the installer script.
```
# ./install
```

5.Umount and remove
```
# umount /run/media/mathworks
# rm -R /run/media/mathworks
```

## Error Handling
### PGP signatures could not be verified
```sh
==> Building and installing package
==> Making package: lib32-libmng 2.0.3-1 (Wed May  3 20:52:59 CST 2017)
==> Checking runtime dependencies...
==> Checking buildtime dependencies...
==> Retrieving sources...
  -> Found libmng-2.0.3.tar.xz
  -> Found libmng-2.0.3.tar.xz.asc
==> Validating source files with sha512sums...
    libmng-2.0.3.tar.xz ... Passed
    libmng-2.0.3.tar.xz.asc ... Skipped
==> Verifying source file signatures with gpg...
    libmng-2.0.3.tar.xz ... FAILED (unknown public key F54984BFA16C640F)
==> ERROR: One or more PGP signatures could not be verified!
==> ERROR: Makepkg was unable to build lib32-libmng.
==> Restart building lib32-libmng ? [y/N]
==> -------------------------------------
==>
```

Solution:
```sh
# Add a single key, and yaourt can update your packages.
gpg --recv-keys <keyid>

# Or trust all keys always. Only use this after all other attempts fail.
vim ~/.gnupg/gpg.conf
# Add line "truct-model always" to the end of the file.
```

### file owned by xxxx
```sh
[firmy@MiWiFi-R1CL-srv ~]$ yaourt -Syu
:: Synchronizing package databases...
 core is up to date         0.0   B  0.00B/s 00:00 [----------------------]   0%
 extra is up to date        0.0   B  0.00B/s 00:00 [----------------------]   0%
 community is up to date    0.0   B  0.00B/s 00:00 [----------------------]   0%
 multilib is up to date     0.0   B  0.00B/s 00:00 [----------------------]   0%
 archlinuxcn is up to date  0.0   B  0.00B/s 00:00 [----------------------]   0%
error: file owned by 'lsb-release' and 'manjaro-release': 'etc/lsb-release'
error: file owned by 'firefox' and 'manjaro-browser-settings': 'usr/lib/firefox/distribution/distribution.ini'
```

Solution:
```sh
# The issue is related tp packages first installed by pacman and later trying to install same packages with yaourt
# If update with pacman, there is no error
[firmy@MiWiFi-R1CL-srv ~]$ sudo pacman -Syu
[sudo] password for firmy:
:: Synchronizing package databases...
 core is up to date         0.0   B  0.00B/s 00:00 [----------------------]   0%
 extra is up to date        0.0   B  0.00B/s 00:00 [----------------------]   0%
 community is up to date    0.0   B  0.00B/s 00:00 [----------------------]   0%
 multilib is up to date     0.0   B  0.00B/s 00:00 [----------------------]   0%
 archlinuxcn is up to date  0.0   B  0.00B/s 00:00 [----------------------]   0%
:: Starting full system upgrade...
 there is nothing to do

# So, first uninstall with pacman the pakages pacman -R and install with yaourt -S
```
