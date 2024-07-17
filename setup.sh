set -e

sudo apt update
sudo apt install -y pip
pip install -U conan==1.60.0
conan profile new drop-file-profile --detect --force
conan profile update settings.compiler.libcxx=libstdc++11 drop-file-profile
conan profile update conf.tools.system.package_manager:mode=install drop-file-profile
conan profile update conf.tools.system.package_manager:sudo=True drop-file-profile
