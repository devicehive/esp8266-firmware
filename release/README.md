Binary fimrware release in repository is depricated. To download firmware binaries follow this link: https://github.com/devicehive/esp8266-firmware/releases  

This directory contains script for generating releases. There are some tools which should be installed.

# win32 build tools
```
sudo apt install binutils-mingw-w64-i686 gcc-mingw-w64-i686-dev gcc-mingw-w64-i686 g++-mingw-w64-i686
```

# osx build tools
```
sudo apt install ccache
wget http://security.ubuntu.com/ubuntu/pool/universe/o/openssl098/libssl0.9.8_0.9.8o-7ubuntu3.2_amd64.deb
wget https://launchpad.net/~flosoft/+archive/ubuntu/cross-apple/+files/apple-x86-odcctools_758.159-0flosoft11_amd64.deb
wget https://launchpad.net/~flosoft/+archive/ubuntu/cross-apple/+files/ccache-lipo_1.0-0flosoft3_amd64.deb
wget https://launchpad.net/~flosoft/+archive/ubuntu/cross-apple/+files/apple-x86-gcc_4.2.1~5646.1flosoft2_amd64.deb
wget https://launchpad.net/~flosoft/+archive/ubuntu/cross-apple/+files/apple-uni-sdk-10.5_20110407-0.flosoft1_amd64.deb
sudo dpkg -i libssl0.9.8_0.9.8o-7ubuntu3.2_amd64.deb
sudo dpkg -i apple-x86-odcctools_758.159-0flosoft11_amd64.deb
sudo dpkg -i ccache-lipo_1.0-0flosoft3_amd64.deb 
sudo dpkg -i apple-uni-sdk-10.5_20110407-0.flosoft1_amd64.deb
sudo dpkg -i apple-x86-gcc_4.2.1~5646.1flosoft2_amd64.deb
```

#converting md to pdf
sudo apt install npm nodejs-legacy
sudo npm install -g markdown-pdf