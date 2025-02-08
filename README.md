# 🐚 42sh
42sh is a **POSIX**-compliant shell written in C language according to the specified [Shell Command Language][1].

## ⚠️ Disclaimer 
This is a school project that was realised in **4 weeks**.\
It does not guarantee the [SCL][1]'s full implementation.

[1]: https://pubs.opengroup.org/onlinepubs/009695399/utilities/xcu_chap02.html
## 🛠️ Installation
Required prior installation:
    [Autotools build system](https://askubuntu.com/questions/430706/installing-autotools-autoconf-on-ubuntu)
```
$ autoreconf --install
$ ./configure --prefix=/install/at/this/path
$ make install
```

```
$ /install/at/this/path/bin/42sh # runs the shell
```

## 👥 Contributors
* [Eliott Mercier-Del-Forno](github.com/thisisyakou)
* [Enzo Berry](github.com/thisisyakou)
* [Pierre-Antoine Rollet](github.com/thisisyakou)
* [Yako Lemeilleur](github.com/thisisyakou)
