set -e

sudo apt update
sudo apt install -y pip gcovr
pip install -U conan --break-system-packages || pip install -U conan
conan profile detect || true
conan install . -b missing
