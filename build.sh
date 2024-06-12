#!/usr/bin/env bash
#./configure --prefix=[your install path] 
#./configure --prefix=[your install path]  --enable-static=yes --enable-shared=false 
#make -j4 && make install

if [ $# -gt 0 ]; then
  mkdir bld-release
  cd bld-release
#  ../configure --prefix=/usr/local/xapian CXXFLAGS='-fpic -O2' --enable-64bit-termpos=yes --enable-64bit-docid=yes --enable-64bit-termcount=yes --enable-static=yes --enable-shared=false
  ../configure --prefix=/usr/local/xapian2 CXXFLAGS='-fpic -O2 -pthread' --enable-64bit-termpos=yes --enable-64bit-docid=yes --enable-64bit-termcount=yes
else
  mkdir bld-debug
  cd bld-debug
  ../configure --prefix=/usr/local/xapian2 CXXFLAGS='-fpic -g -O0 -pthread' --enable-64bit-termpos=yes --enable-64bit-docid=yes --enable-64bit-termcount=yes
fi

#../configure --prefix=/usr/local/xapian  --enable-static=yes --enable-shared=false
#make -j4 && make install

cd ..

