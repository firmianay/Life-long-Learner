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

### Black Manjaro (install Blackarch tools)
BlackArch Linux is an Arch Linux-based penetration testing distribution for penetration testers and security researchers. The repository contains 1805 tools. We now install those tools on top of Manjaro.
```sh
# Run https://blackarch.org/strap.sh as root and follow the instructions.
$ curl -O https://blackarch.org/strap.sh

# The SHA1 sum should match: 34b1a3698a4c971807fb1fe41463b9d25e1a4a09
$ sha1sum strap.sh

# Set execute bit
$ chmod +x strap.sh

# Run strap.sh
$ sudo ./strap.sh
You may now install tools from the blackarch repository.

# To list all of the available tools, run
$ sudo pacman -Sgg | grep blackarch | cut -d' ' -f2 | sort -u

# To install all of the tools, run
$ sudo pacman -S blackarch

# To install a category of tools, run
$ sudo pacman -S blackarch-<category>

# To see the blackarch categories, run
$ sudo pacman -Sg | grep blackarch
```
As part of an alternative method of installation, you can build the blackarch packages from source. You can find the PKGBUILDs on github. To build the entire repo, you can use the blackman tool.
```sh
# First, you must install blackman. If the BlackArch package repository is setup on your machine,
# you can install blackman like:
$ sudo pacman -S blackman

# Download, compile and install package:
$ sudo blackman -i <package>

# Download, compile and install whole category
$ sudo blackman -g <group>

# Download, compile and install all BlackArch tools
$ sudo blackman -a

# To list blackarch categories
$ blackman -l

# To list category tools
$ blackman -p <category>
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
$ yaourt -Syu
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
$ sudo pacman -Syu
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

### Installing pacman () breaks dependency 'pacman-init'
```sh
$ sudo pacman -Syu
[sudo] password for firmy:
:: Synchronizing package databases...
 core is up to date         0.0   B  0.00B/s 00:00 [----------------------]   0%
 extra is up to date        0.0   B  0.00B/s 00:00 [----------------------]   0%
 community is up to date    0.0   B  0.00B/s 00:00 [----------------------]   0%
 multilib is up to date     0.0   B  0.00B/s 00:00 [----------------------]   0%
 archlinuxcn is up to date  0.0   B  0.00B/s 00:00 [----------------------]   0%
:: Starting full system upgrade...
resolving dependencies...
looking for conflicting packages...
error: failed to prepare transaction (could not satisfy dependencies)
:: manjaro-system: installing pacman (5.0.2-1) breaks dependency 'pacman-init'
```
A manjaro install does not have a default mirrorlist. The mirrorlist is created by pacman-mirrors so pacman-mirrors is a serious and very vital of a manjaro installation.

The error will occur if you are on a Manjaro installation but change to Arch mirrors.

Solution:
```sh
# -g, --generate        Generate mirrorlist
# -c, --country COUNTRY
#                     Comma separated list of countries, from which mirrors will be used
sudo pacman-mirrors -g -c China

# -y, --refresh        download fresh package databases from the server
#                      (-yy to force a refresh even if up to date)
# -u, --sysupgrade     upgrade installed packages (-uu enables downgrades)
sudo pacman -Syy
sudo pacman -Syu
```

### filesystem: /etc/os-release exists in filesystem
```sh
$ sudo pacman -Syu

error: failed to commit transaction (conflicting files)
filesystem: /etc/os-release exists in filesystem
Errors occurred, no packages were upgraded.
```

After updating mirrorlist, the error occurred. It seems as something is understood as new but has the same filename with what it is expected to replace.
```sh
$ pacman -Qo /etc/os-release
error: No package owns /etc/os-release
$ ls -al /etc/os-release
lrwxrwxrwx 1 root root 21 Apr 11 18:34 /etc/os-release -> /usr/lib/os-release
$ ls -al /usr/lib/os-release
-rw-r--r-- 1 root root 212 Mar 27 05:57 /usr/lib/os-release
$ pacman -Qo /usr/lib/os-release
/usr/lib/os-release is owned by filesystem 2017.03-2
```
So `/etc/os-release` is a symlink to `/usr/lib/or-release`. You can remove it and do the update/upgrade.

Solution:
```sh
$ sudo mv /etc/os-release /etc/os-release.old
# or sudo rm /etc/os-release
$ sudo pacman -Syu
```

### XXXX exists in filesystem
When I install pwntools, the error occurred as:
```
error: failed to commit transaction (conflicting files)
python2-markupsafe: /usr/lib/python2.7/site-packages/MarkupSafe-1.0-py2.7.egg-info/PKG-INFO exists in filesystem
python2-markupsafe: /usr/lib/python2.7/site-packages/MarkupSafe-1.0-py2.7.egg-info/SOURCES.txt exists in filesystem
python2-markupsafe: /usr/lib/python2.7/site-packages/MarkupSafe-1.0-py2.7.egg-info/dependency_links.txt exists in filesystem
```
We can install the package as follows:
```
pacman -S pwntools --force
```
>--force          force install, overwrite conflicting files

But it is strongly advised to avoid the `--force` or `-f` switch as it is not safe.
