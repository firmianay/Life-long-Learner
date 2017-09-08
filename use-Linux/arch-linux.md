# Arch Linux

- [Install](#install)
  - [clean up useless software](#clean-up-useless-software-manjaro)
  - [software source settings](#software-source-settings)
  - [install MATLAB using an ISO image](#install-matlab-using-an-iso-image)
  - [Black Manjaro (install Blackarch tools)](#black-manjaro-install-blackarch-tools)
- [Error Handling](#error-handling)
  - [PGP signatures could not be verified](#pgp-signatures-could-not-be-verified)
  - [file owned by xxxx](#file-owned-by-xxxx)
  - [Installing pacman () breaks dependency 'pacman-init'](#installing-pacman-breaks-dependency-pacman-init)
  - [filesystem: /etc/os-release exists in filesystem](#filesystem-etc-os-release-exists-in-filesystem)
  - [XXXX exists in filesystem](#xxxx-exists-in-filesystem)
  - [shadowsocks undefined symbol: EVP_CIPHER_CTX_cleanup](#shadowsocks-undefined-symbol-evpcipherctxcleanup)


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

If you got an error:
```
$ sudo ./strap.sh
[+] installing blackarch keyring...
[-] ERROR: invalid keyring signature. please stop by irc.freenode.net/blackarch
```
Try to change `-gpg --keyserver http://pgp.mit.edu --recv-keys` to `-gpg --keyserver http://pgp.mit.edu --recv-keys`. Or do the reverse thing.


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

### shadowsocks undefined symbol: EVP_CIPHER_CTX_cleanup
When I updated `openssl`, the Python version of shadowsocks can't work, so now I try to fix it:
```
$ sslocal -c shadowsocks.json    
INFO: loading config from shadowsocks.json
2017-09-08 12:03:57 INFO     loading libcrypto from libcrypto.so.1.1
Traceback (most recent call last):
  File "/usr/bin/sslocal", line 11, in <module>
    load_entry_point('shadowsocks==2.8.2', 'console_scripts', 'sslocal')()
  File "/usr/lib/python3.6/site-packages/shadowsocks/local.py", line 39, in main
    config = shell.get_config(True)
  File "/usr/lib/python3.6/site-packages/shadowsocks/shell.py", line 262, in get_config
    check_config(config, is_local)
  File "/usr/lib/python3.6/site-packages/shadowsocks/shell.py", line 124, in check_config
    encrypt.try_cipher(config['password'], config['method'])
  File "/usr/lib/python3.6/site-packages/shadowsocks/encrypt.py", line 44, in try_cipher
    Encryptor(key, method)
  File "/usr/lib/python3.6/site-packages/shadowsocks/encrypt.py", line 83, in __init__
    random_string(self._method_info[1]))
  File "/usr/lib/python3.6/site-packages/shadowsocks/encrypt.py", line 109, in get_cipher
    return m[2](method, key, iv, op)
  File "/usr/lib/python3.6/site-packages/shadowsocks/crypto/openssl.py", line 76, in __init__
    load_openssl()
  File "/usr/lib/python3.6/site-packages/shadowsocks/crypto/openssl.py", line 52, in load_openssl
    libcrypto.EVP_CIPHER_CTX_cleanup.argtypes = (c_void_p,)
  File "/usr/lib/python3.6/ctypes/__init__.py", line 361, in __getattr__
    func = self.__getitem__(name)
  File "/usr/lib/python3.6/ctypes/__init__.py", line 366, in __getitem__
    func = self._FuncPtr((name_or_ordinal, self))
AttributeError: /usr/lib/libcrypto.so.1.1: undefined symbol: EVP_CIPHER_CTX_cleanup
```

Let's see the OpenSSL update history:
> EVP_CIPHER_CTX was made opaque in OpenSSL 1.1.0. As a result, EVP_CIPHER_CTX_reset() appeared and EVP_CIPHER_CTX_cleanup() disappeared. EVP_CIPHER_CTX_init() remains as an alias for EVP_CIPHER_CTX_reset().

Before update:
```
$ openssl version
OpenSSL 1.0.2g  1 May 2016
```
After update:
```
$ openssl version             
OpenSSL 1.1.0f  25 May 2017
```

So, we just replace `EVP_CIPHER_CTX_cleanup()` with `EVP_CIPHER_CTX_reset()` and then shadowsocks will work again.
```
$ sudo vim /usr/lib/python3.6/site-packages/shadowsocks/crypto/openssl.py
```
Then change the following two `EVP_CIPHER_CTX_cleanup` to `EVP_CIPHER_CTX_reset`, which on line 52 and line 111.
```
libcrypto.EVP_CIPHER_CTX_cleanup.argtypes = (c_void_p,)

libcrypto.EVP_CIPHER_CTX_cleanup(self._ctx)
```
