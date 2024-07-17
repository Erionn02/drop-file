FROM rikorose/gcc-cmake:latest


WORKDIR /drop-file
RUN apt update
RUN apt install -y sudo
COPY ./setup.sh .
RUN ./setup.sh
COPY ./conanfile.txt .
RUN conan install --build missing -pr drop-file-profile conanfile.txt
COPY . .
RUN mkdir -p build && cd build && \
    cmake -DCMAKE_BUILD_TYPE=Debug .. && cmake --build . -t drop-file-server -- -j $(nproc --all)

CMD ["./build/bin/drop-file-server", "/drop-file/example_assets"]