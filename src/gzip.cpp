#include "gzip.hpp"
#include "Utils.hpp"

#include <zlib.h>
#include <spdlog/spdlog.h>

#include <memory>


size_t
gzip::compress(std::istream &input_stream, std::ostream &output_stream, std::function<void()> update_callback) {
    z_stream deflate_stream{};
    std::unique_ptr<z_stream, decltype(&deflateEnd)> stream_guard{&deflate_stream, deflateEnd};
    constexpr int window_bits = 31;
    constexpr int memory_level = 9; // [1,9], the bigger -> the faster

    if (deflateInit2(&deflate_stream, Z_BEST_SPEED, Z_DEFLATED, window_bits, memory_level,
                     Z_DEFAULT_STRATEGY) != Z_OK) {
        throw GzipException("Deflate init failed");
    }
    std::array<char, BUFFER_SIZE> input_buffer{};
    std::array<unsigned char, BUFFER_SIZE> compression_buffer{};

    std::size_t bytes_written_to_stream{0};

    while (input_stream.read(input_buffer.data(), BUFFER_SIZE).gcount() > 0) {
        update_callback();
        bytes_written_to_stream += compressChunk(output_stream, input_stream, deflate_stream, input_buffer,
                                                 compression_buffer, Z_NO_FLUSH);
    }
    bytes_written_to_stream += compressChunk(output_stream, input_stream, deflate_stream, input_buffer,
                                             compression_buffer, Z_FINISH);

    return bytes_written_to_stream;
}

size_t gzip::compressChunk(std::ostream &output_stream, const std::istream &input_stream, z_stream &deflate_stream,
                           std::array<char, BUFFER_SIZE> &input_buffer,
                           std::array<unsigned char, BUFFER_SIZE> &compression_buffer, int flush) {
    std::size_t bytes_written_to_stream{0};

    deflate_stream.next_in = std::bit_cast<unsigned char *>(input_buffer.data());
    deflate_stream.avail_in = static_cast<unsigned int>(input_stream.gcount());

    do {

        deflate_stream.avail_out = static_cast<unsigned int>(BUFFER_SIZE);
        deflate_stream.next_out = compression_buffer.data();
        auto ret = deflate(&deflate_stream, flush);
        if (ret < 0) {
            throw GzipException(fmt::format("Deflate failed with ret: {}", ret));
        }
        auto bytes_compressed = BUFFER_SIZE - deflate_stream.avail_out;
        bytes_written_to_stream += bytes_compressed;
        output_stream.write((char *) compression_buffer.data(), static_cast<std::streamsize>(bytes_compressed));
    } while (deflate_stream.avail_out == 0);
    return bytes_written_to_stream;
}


std::size_t gzip::decompress(std::ostream &decompressed_out_stream, std::istream &compressed_in_stream,
                             std::size_t compressed_length) {
    z_stream inflate_s{};
    std::unique_ptr<z_stream, decltype(&inflateEnd)> stream_guard{&inflate_s, inflateEnd};

    constexpr int window_bits = 15 + 32;


    if (inflateInit2(&inflate_s, window_bits) != Z_OK) {
        throw GzipException("Inflate init failed");
    }
    std::array<char, BUFFER_SIZE> input_buffer{};
    std::array<char, BUFFER_SIZE> output_buffer{};
    std::size_t total_read{0};
    std::size_t bytes_written_to_stream = 0;
    std::size_t left_to_read{compressed_length};

    while (total_read < compressed_length) {
        std::size_t this_chunk_size{std::min(BUFFER_SIZE, left_to_read)};
        compressed_in_stream.read(input_buffer.data(), static_cast<std::streamsize>(this_chunk_size));
        auto bytes_read = compressed_in_stream.gcount();
        total_read += static_cast<std::size_t>(bytes_read);
        left_to_read = compressed_length - total_read;


        inflate_s.next_in = std::bit_cast<unsigned char *>(input_buffer.data());
        inflate_s.avail_in = static_cast<unsigned int>(bytes_read);
        do {
            inflate_s.avail_out = static_cast<unsigned int>(BUFFER_SIZE);
            inflate_s.next_out = std::bit_cast<unsigned char *>(output_buffer.data());
            int ret = inflate(&inflate_s, Z_FINISH);
            if (ret != Z_STREAM_END && ret != Z_OK && ret != Z_BUF_ERROR) {
                throw GzipException(inflate_s.msg);
            }
            auto decompressed_bytes = BUFFER_SIZE - inflate_s.avail_out;
            decompressed_out_stream.write(output_buffer.data(), static_cast<std::streamsize>(decompressed_bytes));
            bytes_written_to_stream += decompressed_bytes;
        } while (inflate_s.avail_out == 0);
    }
    return bytes_written_to_stream;
}
