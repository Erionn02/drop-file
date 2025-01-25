FROM rikorose/gcc-cmake:latest


WORKDIR /drop-file
RUN apt update
RUN apt install -y sudo
COPY . .
RUN ./setup.sh
RUN mkdir -p build && cd build && \
    cmake -DCMAKE_BUILD_TYPE=Release .. && cmake --build . -t drop-file-server -- -j $(nproc --all)

CMD ["/drop-file/build/apps/drop-file-server", "/drop-file/example_assets"]