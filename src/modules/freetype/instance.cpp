#include "freetype.h"
#include <fstream>
#include <memory>

using namespace Module::Freetype;

void Module::Freetype::checkError(FT_Error error)
{
    if (error) {
        throw std::runtime_error(std::string("Font::Freetype] ") + FT_Error_String(error));
    }
}

FTInstance::FTInstance()
{
    FT_Init_FreeType(&mLibrary);
}

FTInstance::~FTInstance()
{
    FT_Done_FreeType(mLibrary);
}

FTInstance::operator FT_Library()
{
    return mLibrary;
}

const std::vector<FT_Byte>& FTInstance::getFileData(const std::string& filename)
{
    auto it = mFilesData.find(filename);
    if (it != mFilesData.end()) {
        return it->second;
    }

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

    mFilesData.emplace(filename, std::move(data));

    return mFilesData.find(filename)->second;
}
