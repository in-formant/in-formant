#include "freetype.h"
#include <fstream>
#include <memory>

using namespace Module::Freetype;

FontFile::FontFile(FT_Library library, const std::string& filename, int hdpi, int vdpi)
    : mLibrary(library),
      mHorizontalDPI(hdpi),
      mVerticalDPI(vdpi)
{
    std::ifstream file(filename, std::ios::binary);

    if (!file.is_open()) {
        throw std::runtime_error(std::string("Font::Freetype] Error reading font file \"") + filename + "\"");
    }

    constexpr size_t dataVectorIncrSize = 4096;
    constexpr size_t readBufferSize = 1024;
    
    auto buffer = std::make_unique<char[]>(readBufferSize);

    std::vector<FT_Byte> data;

    file.seekg(0);
    do {
        file.read(buffer.get(), readBufferSize);
        size_t bytesRead = file.gcount();
        if (data.size() + bytesRead > data.capacity()) {
            data.reserve(data.capacity() + dataVectorIncrSize);
        }
        data.insert(data.end(),
                reinterpret_cast<FT_Byte *>(buffer.get()),
                reinterpret_cast<FT_Byte *>(std::next(buffer.get(), bytesRead)));
    } while (file);
    
    data.shrink_to_fit();
 
    mDataSize = data.size();
    mData = new FT_Byte[mDataSize];
    memcpy(mData, data.data(), mDataSize * sizeof(FT_Byte));
}

FontFile::~FontFile()
{
    for (auto& [ptsz, font] : mFonts) {
        delete font;
    }

    delete[] mData;
}

Font& FontFile::with(int pointSize)
{
    auto it = mFonts.find(pointSize);
    if (it != mFonts.end()) {
        return * it->second;
    }

    mFonts.emplace(pointSize, new Font(mLibrary, mData, mDataSize, pointSize, mHorizontalDPI, mVerticalDPI));

    return * mFonts.find(pointSize)->second;
}
