#include <iostream>
#include <fstream>
#include <vector>
#include <cstring>
#include "hef_format.hpp"
#include "hef_decode.hpp"

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <input.nef> <output.tiff>\n";
        return 1;
    }
    
    const char* input_path = argv[1];
    const char* output_path = argv[2];
    
    // Read file
    std::ifstream file(input_path, std::ios::binary);
    if (!file) {
        std::cerr << "Cannot open " << input_path << "\n";
        return 1;
    }
    
    file.seekg(0, std::ios::end);
    size_t file_size = file.tellg();
    file.seekg(0, std::ios::beg);
    
    std::vector<uint8_t> file_data(file_size);
    file.read(reinterpret_cast<char*>(file_data.data()), file_size);
    file.close();
    
    // Parse headers
    hefraw::ImageHeader header;
    if (!hefraw::parse_hef_headers(file_data.data(), file_size, header)) {
        std::cerr << "Failed to parse HE* headers\n";
        std::cerr << "File size: " << file_size << "\n";
        std::cerr << "First 8 bytes: ";
        for (int i = 0; i < 8 && i < (int)file_size; i++) {
            std::cerr << std::hex << (int)file_data[i] << " ";
        }
        std::cerr << std::dec << "\n";
        
        // Debug: try to find SubIFDs manually
        if (file_size >= 8) {
            uint32_t ifd0 = file_data[4] | (file_data[5] << 8) | (file_data[6] << 16) | (file_data[7] << 24);
            std::cerr << "IFD0 offset: " << ifd0 << "\n";
            if (ifd0 + 2 < file_size) {
                uint16_t num_entries = file_data[ifd0] | (file_data[ifd0+1] << 8);
                std::cerr << "IFD0 entries: " << num_entries << "\n";
            }
        }
        return 1;
    }
    
    std::cout << "Image: " << header.width << "x" << header.height << "\n";
    std::cout << "Tiles: " << header.tiles.size() << "\n";
    
    // Decode CFA
    std::vector<uint16_t> cfa;
    if (!hefraw::assemble_image_cfa16(header, file_data.data(), file_size, cfa)) {
        std::cerr << "Failed to decode CFA\n";
        return 1;
    }
    
    std::cout << "Decoded " << cfa.size() << " pixels\n";
    
    // Simple 16-bit TIFF writer (minimal)
    std::ofstream out(output_path, std::ios::binary);
    if (!out) {
        std::cerr << "Cannot create " << output_path << "\n";
        return 1;
    }
    
    // Write minimal TIFF header
    uint8_t tiff_header[] = {
        'I', 'I', 0x2A, 0x00,  // Little-endian TIFF magic
        0x08, 0x00, 0x00, 0x00, // IFD offset
    };
    out.write(reinterpret_cast<char*>(tiff_header), sizeof(tiff_header));
    
    // Write IFD
    uint16_t num_entries = 8;
    out.write(reinterpret_cast<char*>(&num_entries), 2);
    
    // ImageWidth (0x0100)
    uint8_t width_entry[] = {0x00, 0x01, 0x03, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    out.write(reinterpret_cast<char*>(width_entry), 12);
    
    // ImageLength (0x0101) 
    uint8_t height_entry[] = {0x01, 0x01, 0x03, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    out.write(reinterpret_cast<char*>(height_entry), 12);
    
    // BitsPerSample (0x0102)
    uint8_t bps_entry[] = {0x02, 0x01, 0x03, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    out.write(reinterpret_cast<char*>(bps_entry), 12);
    
    // Compression (0x0103) - uncompressed
    uint8_t comp_entry[] = {0x03, 0x01, 0x03, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00};
    out.write(reinterpret_cast<char*>(comp_entry), 12);
    
    // PhotometricInterpretation (0x0106) - CFA
    uint8_t photo_entry[] = {0x06, 0x01, 0x03, 0x00, 0x01, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00};
    out.write(reinterpret_cast<char*>(photo_entry), 12);
    
    // StripOffsets (0x0111)
    uint8_t so_entry[] = {0x11, 0x01, 0x04, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    out.write(reinterpret_cast<char*>(so_entry), 12);
    
    // SamplesPerPixel (0x0115)
    uint8_t spp_entry[] = {0x15, 0x01, 0x03, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00};
    out.write(reinterpret_cast<char*>(spp_entry), 12);
    
    // StripByteCounts (0x0117)
    uint8_t sbc_entry[] = {0x17, 0x01, 0x04, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    out.write(reinterpret_cast<char*>(sbc_entry), 12);
    
    // Next IFD offset (0)
    uint32_t next_ifd = 0;
    out.write(reinterpret_cast<char*>(&next_ifd), 4);
    
    // Write image data
    out.write(reinterpret_cast<char*>(cfa.data()), cfa.size() * 2);
    
    out.close();
    std::cout << "Exported to " << output_path << "\n";
    
    return 0;
}
