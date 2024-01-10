#! /bin/bash
set -e
trap 'previous_command=$this_command; this_command=$BASH_COMMAND' DEBUG
trap 'echo FAILED COMMAND: $previous_command' EXIT

#-------------------------------------------------------------------------------------------
# This WIP script will download packages for, configure, 
# build and install a Windows on ARM64 GCC cross-compiler.
# See: http://preshing.com/20141119/how-to-build-a-gcc-cross-compiler
#-------------------------------------------------------------------------------------------

# arch: [aarch64, x86_64]
# platform: [w64-mingw32, linux-gnu]
BUILD_ARCH=$HOSTTYPE
TARGET_ARCH=x86_64
TARGET_PLATFORM=w64-mingw32
RUN_DOWNLOAD=1
RUN_CONFIG=1
TARGET=$TARGET_ARCH-$TARGET_PLATFORM
BUILD_DIR=build-$TARGET
INSTALL_PATH=$(pwd)/bin-$TARGET
PARALLEL_MAKE=-j6
CONFIGURATION_OPTIONS="--disable-multilib --disable-threads --disable-shared --disable-gcov"
MINGW_REPO=https://git.code.sf.net/p/mingw-w64/mingw-w64 


if [[ $TARGET == *mingw* ]]; then
    BUILD_MINGW=1
else
    BUILD_MINGW=0
fi



for var in "$@"
do
    if [ "$var"${BLUE} = "q" ] || [ "$var"${BLUE} = "quick" ] ; then 
      RUN_CONFIG=0
      RUN_DOWNLOAD=0
      echo_heading "quick build "
    fi
done

export PATH=$INSTALL_PATH/bin:$PATH

echo_heading()
{
    BLUE='\033[1;34m'
    NC='\033[0m' # No Color

    echo ""
    echo -e " ${BLUE}==== $1 ====${NC}"
    echo ""
}

if [ $RUN_DOWNLOAD = 1 ] ; then
    echo_heading "download sources"
    # Download packages
    mkdir -p downloads

    wget -N -P downloads https://ftp.gnu.org/gnu/binutils/binutils-2.41.tar.gz
    wget -N -P downloads https://ftp.gnu.org/gnu/gcc/gcc-13.1.0/gcc-13.1.0.tar.gz
    wget -N -P downloads https://gcc.gnu.org/pub/gcc/infrastructure/mpfr-4.1.0.tar.bz2
    wget -N -P downloads https://gcc.gnu.org/pub/gcc/infrastructure/gmp-6.2.1.tar.bz2
    wget -N -P downloads https://gcc.gnu.org/pub/gcc/infrastructure/mpc-1.2.1.tar.gz
    wget -N -P downloads https://gcc.gnu.org/pub/gcc/infrastructure/isl-0.24.tar.bz2

    # Extract everything
    mkdir -p code
    cd code
    for f in ../downloads/*.tar*; do tar xf $f --skip-old-files; done

    # git clone "$BINUTILS_REPO" "binutils"  || git -C "binutils" pull
    # git clone "$GCC_REPO" "gcc" || git -C "gcc" pull
    git clone "$MINGW_REPO" mingw || git -C mingw pull

    # Symbolic links for deps
    ln -sf `ls -1d gcc-*/` gcc
    ln -sf `ls -1d binutils-*/` binutils

    cd gcc
    ln -sf `ls -1d ../mpfr-*/` mpfr
    ln -sf `ls -1d ../gmp-*/` gmp
    ln -sf `ls -1d ../mpc-*/` mpc
    ln -sf `ls -1d ../isl-*/` isl
    cd ../..

fi

mkdir -p $BUILD_DIR

# Build Binutils
echo_heading "build binutils"
mkdir -p $BUILD_DIR/binutils
cd $BUILD_DIR/binutils
if [ $RUN_CONFIG = 1 ] ; then ../../code/binutils/configure \
    --prefix=$INSTALL_PATH --target=$TARGET
fi
make $PARALLEL_MAKE
make install
cd ../..

# Build C/C++ Compilers
echo_heading "build gcc"
mkdir -p $BUILD_DIR/gcc
cd $BUILD_DIR/gcc
if [ $RUN_CONFIG = 1 ] ; then ../../code/gcc/configure \
    --prefix=$INSTALL_PATH --target=$TARGET \
    --enable-languages=c,lto,c++,fortran \
    --disable-multilib --disable-shared --disable-gcov
fi
make $PARALLEL_MAKE all-gcc
make install-gcc
cd ../..

# mingw headers
echo_heading "build mingw-headers"

if [[ $TARGET == *x86_64* ]]; then
    MINGW_CONF="--disable-lib32 --enable-lib64 --disable-libarm32 --disable-libarm64"
else
    MINGW_CONF="--disable-lib32 --disable-lib64 --disable-libarm32 --enable-libarm64"
fi

mkdir -p $BUILD_DIR/mingw-headers
cd $BUILD_DIR/mingw-headers
if [ $RUN_CONFIG = 1 ] ; then ../../code/mingw/mingw-w64-headers/configure \
    --prefix=$INSTALL_PATH/$TARGET --host=$TARGET --with-default-msvcrt=msvcrt \
    $MINGW_CONF
fi
make
make install
cd ../..

# Symlink for gcc
ln -sf $INSTALL_PATH/$TARGET $INSTALL_PATH/mingw

# Build mingw
echo_heading "build mingw"
mkdir -p $BUILD_DIR/mingw
cd $BUILD_DIR/mingw
if [ $RUN_CONFIG = 1 ] ; then ../../code/mingw/mingw-w64-crt/configure \
    --build=$BUILD_ARCH-linux-gnu \
    --with-sysroot=$INSTALL_PATH \
    --prefix=$INSTALL_PATH/$TARGET \
    --host=$TARGET \
    --disable-shared \
    $MINGW_CONF \
    --with-default-msvcrt=msvcrt
fi
make $PARALLEL_MAKE
make install
cd ../..

# Build Libgcc
echo_heading "build libgcc"
cd $BUILD_DIR/gcc
make $PARALLEL_MAKE all-target-libgcc
make install-target-libgcc
cd ../..

# Build libstdc++
echo_heading "build libstdcpp"
cd $BUILD_DIR/gcc
make $PARALLEL_MAKE all-target-libstdc++-v3
make install-target-libstdc++-v3
cd ../..

# Build libgfortran++
echo_heading "build libgfortran"
cd $BUILD_DIR/gcc
make $PARALLEL_MAKE all-target-libgfortran
make install-target-libgfortran
cd ../..

# Build the rest of GCC
echo_heading "build remaining gcc"
cd $BUILD_DIR/gcc
make $PARALLEL_MAKE all
make install
cd ../..


trap - EXIT
echo 'Success!'