# cd /local/graehl/src/gcc-build;sudo mv /local/gcc{,-4.9}; sudo make install
#cd /local;sudo mv gcc{,-4.9}; sudo tar xzf /home/graehl/gcc.tar.gz
gccver=5.1.0
srcdir=/local/graehl/src
gccwithv=gcc-$gccver
gccsrc=$srcdir/$gccwithv
gccbuild=$srcdir/gcc-build
gccprefix=/local/gcc
gccget() {
    mkdir -p $srcdir
    cd $srcdir
    [[ -f $gccwithv.tar.bz2 ]] || wget http://ftp.gnu.org/gnu/gcc/gcc-$gccver/$gccwithv.tar.bz2
    gccclean
}
gccclean() {
    cd $srcdir
    rm -rf $gccwithv
    bzcat $gccwithv.tar.bz2 | tar xf -
    gccprereq
    rm -rf $gccbuild
}
gccprereq() {
    cd $gccsrc
    contrib/download_prerequisites
}
gccbuild() {
    (
    cd $gccsrc
    set -e
    mkdir $gccbuild
        cd $gccbuild
        ../gcc-$gccver/configure \
            --prefix=$gccprefix \
            --libdir=$gccprefix/lib \
            --enable-static \
            --enable-shared \
            --enable-threads=posix \
            --enable-__cxa_atexit \
            --enable-clocale=gnu \
            --disable-multilib \
            --with-system-zlib \
            --disable-checking \
            --enable-languages=c,c++
        make "$@"
    )
}
gccinstall() {
    cd $gccbuild
    sudo make install
}
gccall() {
    (set -e
    gccget
    gccbuild "$@"
    gccinstall
    )
}
uselocalgcc() {
    export PATH=/local/gcc/bin:$PATH
    export LD_LIBRARY_PATH=/local/gcc/lib64:$LD_LIBRARY_PATH
}
vgall() {
    (set -e
     cd /local/graehl/src
     tarxzf ~/src/valgrind-3.10.1.tar.bz2 2>&1 >/dev/null
     cd valgrind-3.10.1/
     ./configure --prefix=/usr/local && make -j8 && sudo make install
    )
}
gdbinstall() {
mkdir -pv /usr/share/gdb/auto-load/usr/lib              &&
mv -v /usr/lib/*gdb.py /usr/share/gdb/auto-load/usr/lib &&
chown -v -R root:root \
      /usr/lib/gcc/*linux-gnu/$gccver/include{,-fixed}
}
