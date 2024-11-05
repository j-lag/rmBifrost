# rmBifrost
rmBifrost is a development framework that enables homebrew applications on the reMarkable Paper Pro (RMPP). Supports 3.14.4.0.

This project is under active development, and we welcome contributions. Please feel free to open an issue if you'd like to implement any features.

## Building
1. Set up the official RMPP toolchain and source the environment file
2. `git clone --recursive https://github.com/shg8/rmBifrost`
3. `cd rmBifrost && mkdir build`
4. `cmake ../ -DCMAKE_BUILD_TYPE=Debug`
5. `cmake --build ./ -jN`
6. Load the library with xochitl `LD_PRELOAD=./libbifrost.so /usr/bin/xochitl --system`

## Disclaimer
By viewing, downloading, or using any code or derived binaries of this repository, you agree to the following terms.
<details>
  <summary>Please ensure that you are familarized with <a href="https://support.remarkable.com/s/article/Limited-Warranty-Policy">reMarkable policies</a> before using this software in any capacity.</summary>
  
  This software is provided "as is," without any express or implied warranties of any kind, including, but not limited to, implied warranties of merchantability, fitness for a particular purpose, or non-infringement. In no event shall the authors or copyright holders be liable for any claim, damages, or other liability, whether in an action of contract, tort, or otherwise, arising from, out of, or in connection with the software or the use or other dealings in the software.

  By using this software, you acknowledge that you assume all risks associated with its use and accept that no warranty, guarantee, or representation is made regarding the accuracy, reliability, or performance of the software.

  "reMarkable" and "reMarkable Paper Pro" are registered trademarks of Remarkable AS. All rights in the trademarks are the property of Remarkable AS. Any use is for nominative purposes only, and no endorsement, sponsorship, or affiliation is implied or intended.
</details>

# reminder

## install toolchain

./meta-toolchain-remarkable-4.0.813-ferrari-public-x86_64-toolchain.sh 

## run toolchain

osboxes@osboxes:/mnt/shared$  . /opt/codex/ferrari/4.0.813/environment-setup-cortexa53-crypto-remarkable-linux

## install turbojpeg / libpng:

scp /opt/codex/ferrari/4.0.813/sysroots/cortexa53-crypto-remarkable-linux/usr/lib/libturbojpeg.so* root@10.11.99.1:/home/root
scp /opt/codex/ferrari/4.0.813/sysroots/cortexa53-crypto-remarkable-linux/usr/lib/libpng*.so* root@10.11.99.1:/home/root

## run:
export LD_LIBRARY_PATH=/home/root:$LD_LIBRARY_PATH
LD_PRELOAD=./libbifrost.so /usr/bin/xochitl --system
