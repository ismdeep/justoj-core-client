#!/usr/bin/env bash

work_dir=`pwd`

/usr/sbin/useradd -m -u 1536 judge

apt-get update
apt-get install -y \
    htop wget curl vim flex \
    gcc g++ clang \
    make cmake \
    zlibc zlib1g-dev \
    iptables \
    openjdk-11-jdk \
    fp-compiler \
    sbcl clisp guile-2.2 \
    golang

PYTHON2_VERSION=2.7.18
PYTHON3_VERSION=3.7.8


cd ${work_dir} && cp docs/Python-${PYTHON2_VERSION}.tar.xz /opt
cd ${work_dir} && cp docs/Python-${PYTHON3_VERSION}.tar.xz /opt

cd /opt && \
    xz -d   Python-${PYTHON2_VERSION}.tar.xz && \
    tar -xf Python-${PYTHON2_VERSION}.tar && \
    rm      Python-${PYTHON2_VERSION}.tar

cd /opt && \
    xz -d   Python-${PYTHON3_VERSION}.tar.xz && \
    tar -xf Python-${PYTHON3_VERSION}.tar && \
    rm      Python-${PYTHON3_VERSION}.tar

cd /opt/Python-${PYTHON2_VERSION}; \
    ./configure \
        --prefix=/usr/local/python2 \
        --enable-optimizations \
        --enable-loadable-sqlite-extensions && \
    make -j8 build_all && \
    make -j8 altinstall

cd /opt/Python-${PYTHON3_VERSION}; \
    ./configure \
        --prefix=/usr/local/python3 \
        --enable-optimizations \
        --enable-loadable-sqlite-extensions; \
    make -j8 build_all; \
    make -j8 altinstall


mkdir -p /data/build
cd /data/build && cmake ${work_dir} && make && cp justoj* /usr/bin/


mkdir -p /data/honix/etc
cd ${work_dir} && cp -v config-example/java0.policy /data/honix/etc
cd ${work_dir} && cp -v config-example/judge.conf   /data/honix/etc


echo "To complete the installation, follow below instructions:"
echo ""
echo "- Edit /data/honix/etc/judge.conf"
echo "- Link justoj-data/data to /data/honix/data"
echo ""
echo "Start command: justoj-core /data/honix"
