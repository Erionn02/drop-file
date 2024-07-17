FROM rikorose/gcc-cmake:latest

RUN apt update
RUN apt install -y pip
RUN pip install -U conan==1.60.0
RUN conan profile new drop-file-profile --detect --force
RUN conan profile update settings.compiler.libcxx=libstdc++11 drop-file-profile
RUN conan profile update conf.tools.system.package_manager:mode=install drop-file-profile
RUN conan profile update conf.tools.system.package_manager:sudo=True drop-file-profile

WORKDIR /drop-file
COPY ./conanfile.txt .
RUN conan install --build missing conanfile.txt
COPY . .
RUN mkdir -p build && cd build && \
    cmake -DCMAKE_BUILD_TYPE=Debug .. && cmake --build . -t drop-file-server -- -j $(nproc --all)

CMD ["./build/bin/drop-file-server"]