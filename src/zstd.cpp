#include "zstd.hpp"

#include <zstd.h>

#include <memory>


void assertOk(std::size_t error_code) {
    if (ZSTD_isError(error_code)) {
       throw ZSTDException{ZSTD_getErrorName(error_code)};
    }
}

size_t
zstd::compress(std::istream &input_stream, std::ostream &output_stream, std::function<void()> update_callback) {
    int const cLevel = 6;
    std::string read_buffer(ZSTD_DStreamInSize(), '\0');
    std::string write_buffer(ZSTD_DStreamOutSize(), '\0');

    ZSTD_CCtx * const cctx = ZSTD_createCCtx();
    std::unique_ptr<ZSTD_CCtx, decltype(&ZSTD_freeCCtx)> ctx_free_guard{cctx, ZSTD_freeCCtx};
    if (cctx == nullptr)  {
        throw ZSTDException{"ZSTD_createCCtx() failed!"};
    }
    assertOk(ZSTD_CCtx_setParameter(cctx, ZSTD_c_compressionLevel, cLevel));
    assertOk(ZSTD_CCtx_setParameter(cctx, ZSTD_c_checksumFlag, 1));
    assertOk( ZSTD_CCtx_reset(cctx, ZSTD_reset_session_only) );

    std::size_t bytes_written_to_stream{0};
    while (input_stream.read(read_buffer.data(), static_cast<std::streamsize>(read_buffer.size())).gcount() > 0) {
        auto read = static_cast<std::size_t>(input_stream.gcount());
        bool is_last_chunk = input_stream.gcount() < read_buffer.size();

        const ZSTD_EndDirective mode = is_last_chunk ? ZSTD_e_end : ZSTD_e_continue;
        ZSTD_inBuffer input = { read_buffer.data(), read, 0 };
        bool is_finished;
        do {
            ZSTD_outBuffer output = { write_buffer.data(), write_buffer.size(), 0 };
            const std::size_t remaining = ZSTD_compressStream2(cctx, &output, &input, mode);
            assertOk(remaining);
            bytes_written_to_stream += output.pos;
            output_stream.write(write_buffer.data(), static_cast<std::streamsize>(output.pos));
            is_finished = is_last_chunk ? (remaining == 0) : (input.pos == input.size);
        } while (!is_finished);
        if (input.pos != input.size) {
            throw ZSTDException("Impossible: zstd only returns 0 when the input is completely consumed!");
        }
        update_callback();
    }
    return bytes_written_to_stream;
}

std::size_t zstd::decompress(std::ostream &decompressed_out_stream, std::istream &compressed_in_stream,
                             std::size_t compressed_length) {
    std::string read_buffer(ZSTD_DStreamInSize(), '\0');
    std::string write_buffer(ZSTD_DStreamOutSize(), '\0');

    ZSTD_DCtx* const dctx = ZSTD_createDCtx();
    if (dctx == nullptr)  {
        throw ZSTDException{"ZSTD_createDCtx() failed!"};
    }
    std::unique_ptr<ZSTD_DCtx, decltype(&ZSTD_freeDCtx)> ctx_free_guard{dctx, ZSTD_freeDCtx};


    std::size_t total_read{0};
    std::size_t bytes_written_to_stream = 0;
    std::size_t left_to_read{compressed_length};
    std::size_t this_chunk_size{std::min(read_buffer.size(), left_to_read)};

    while (compressed_in_stream.read(read_buffer.data(), static_cast<std::streamsize>(this_chunk_size)).gcount() > 0) {
        std::size_t bytes_read = compressed_in_stream.gcount();
        total_read += static_cast<std::size_t>(bytes_read);
        left_to_read = compressed_length - total_read;
        this_chunk_size = std::min(read_buffer.size(), left_to_read);

        ZSTD_inBuffer input = { read_buffer.data(), bytes_read, 0 };
        while (input.pos < input.size) {
            ZSTD_outBuffer output = {write_buffer.data(), write_buffer.size(), 0 };
            size_t const ret = ZSTD_decompressStream(dctx, &output , &input);
            assertOk(ret);
            bytes_written_to_stream += output.pos;
            decompressed_out_stream.write(write_buffer.data(), static_cast<std::streamsize>(output.pos));
        }
    }
    return bytes_written_to_stream;
}
