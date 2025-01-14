set -e

sudo apt update
sudo apt install -y pip gcovr
pip install -U conan --break-system-packages
conan install . -b missing --output-folder build
