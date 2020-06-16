Build instructions:

### Ubuntu 18.04
Install gcc7 boost ssl packages.
  ```
  sudo add-apt-repository ppa:ubuntu-toolchain-r/test
  sudo apt update
  sudo apt install g++-7 libboost-all-dev libssl-dev -y
  ```
Set it up so the symbolic links gcc, g++ point to the newer version:
```
  sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-7 60 \
                           --slave /usr/bin/g++ g++ /usr/bin/g++-7
  sudo update-alternatives --config gcc
  gcc --version
  g++ --version
```
Install latest CMake
```
  wget "https://cmake.org/files/v3.12/cmake-3.12.0-Linux-x86_64.sh"
  sudo sh cmake-3.12.0-Linux-x86_64.sh --skip-license --prefix=/usr
```

```
sudo apt install ocl-icd-* opencl-headers
sudo add-apt-repository ppa:beineri/opt-qt-5.11.0-bionic
sudo apt-get update && sudo apt-get upgrade
sudo apt-get install qt511base qt511declarative qt511svg qt511tools qt511graphicaleffects qt511quickcontrols qt511quickcontrols2
export PATH=/opt/qt511/bin:$PATH
```

Go to defis project folder and call:
cmake -DXGM_USE_GPU=On -DCMAKE_BUILD_TYPE=Release . && make -j4
Or without GUI: 
cmake -DXGM_NO_QT_UI_WALLET=On -DCMAKE_BUILD_TYPE=Release . && make -j4

You'll find binaries in grimm/..., wallet/..., ui/..., explorer/... subfolders.


## Mac
Install Brew Package Manager.
Installed necessary packages using brew install openssl boost cmake command.
Download and install QT5.11 from the official website https://download.qt.io/official_releases/qt/5.11/5.11.0/qt-opensource-mac-x64-5.11.0.dmg.mirrorlist
Add export OPENSSL_ROOT_DIR="/usr/local/opt/openssl" and export PATH=/Users/<username>/Qt5.11.0/5.11.0/clang_64/bin:$PATH to the Environment Variables.
Go to defis project folder and call cmake -DCMAKE_BUILD_TYPE=Release . && make -j4.
You'll find binaries in grimm/..., wallet/..., ui/..., explorer/... subfolders.
If you don't want to build UI don't install QT5 and add -DXGM_NO_QT_UI_WALLET=On command line parameter when you are calling cmake.
  

## Windows

Install Visual Studio >= 2017 with CMake support.

Download and install Boost prebuilt binaries https://sourceforge.net/projects/boost/files/boost-binaries/1.68.0/boost_1_68_0-msvc-14.1-64.exe, also add BOOST_ROOT to the Environment Variables.

Download and install OpenSSL prebuilt binaries https://slproweb.com/products/Win32OpenSSL.html (Win64 OpenSSL v1.1.0h for example) and add OPENSSL_ROOT_DIR to the Environment Variables.

```
Download file https://developer.nvidia.com/compute/cuda/10.0/Prod/local_installers/cuda_10.0.130_411.31_win10 -FileName cuda.exe    
cmd: cuda.exe -s nvcc_10.0 nvprune_10.0 cublas_dev_10.0 cufft_dev_10.0 curand_dev_10.0 cusolver_dev_10.0 cusparse_dev_10.0 nvgraph_dev_10.0 npp_dev_10.0 nvrtc_dev_10.0 nvml_dev_10.0
cmd: set PATH=%ProgramFiles%\NVIDIA GPU Computing Toolkit\CUDA\v10.0\bin;%ProgramFiles%\NVIDIA GPU Computing Toolkit\CUDA\v10.0\libnvvp;%PATH%

Download file "http://registrationcenter-download.intel.com/akdlm/irc_nas/vcp/13794/opencl_runtime_18.1_x64_setup.msi"
cmd: start /wait msiexec /i opencl_runtime_18.1_x64_setup.msi /qn  /l*v msiexec2.log
```

Download and install QT 5.11 http://download.qt.io/archive/qt/5.11/5.11.1/ and add QT5_ROOT_DIR to the Environment Variables (usually it looks like .../5.11.0/msvc2017_64), also add QML_IMPORT_PATH (it should look like %QT5_ROOT_DIR%\qml). BTW disabling system antivirus on Windows makes QT installing process much faster.

Add .../qt511/5.11.1/msvc2017_64/bin and .../boost_1_68_0/lib64-msvc-14.1 to the System Path.
Open project folder in Visual Studio, select your target (Release-x64 for example, add -DXGM_USE_GPU=On in cmake options, if you downloaded 64bit Boost and OpenSSL) and select CMake -> Build All.

Go to CMake -> Cache -> Open Cache Folder -> defis (you'll find binaries in grimm/..., wallet/..., ui/..., explorer/... subfolders).

