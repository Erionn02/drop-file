#include "DirectoryCompressor.hpp"

#include <iostream>
#include <filesystem>




int main() {
    fs::path source_folder = "/home/kuba/Documents";
    fs::path compressed_file = "/tmp/compressed_documents.zip";
    fs::path decompressed_folder = "/tmp/Documents";

    DirectoryCompressor dc1{source_folder};
    dc1.compress(compressed_file);
    std::cout << "Folder skompresowany do: " << compressed_file << std::endl;

    DirectoryCompressor dc2{decompressed_folder};
    dc1.decompress(compressed_file);
    std::cout << "Folder zdekompresowany do: " << decompressed_folder << std::endl;

    return 0;
}

