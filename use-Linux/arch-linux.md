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

