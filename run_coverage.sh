set -e

if [ $# -ne 1 ]; then
    echo "Usage: $0 <build_dir>"
    exit 1
fi

build_dir=$1

mkdir -p $build_dir
cd $build_dir
cmake .. -DCMAKE_BUILD_TYPE=Debug -Dcoverage=ON
cmake --build . -- -j$(nproc)
ctest --output-on-failure

cd ..
gcovr -r . $build_dir --exclude  apps --fail-under-function 85 --sonarqube $build_dir/code-coverage.xml