#include "freetype.h"
#include <stdexcept>
#include <cmath>

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
    for (auto& [fnam, fontfile] : mFontFiles) {
        delete fontfile;
    }

    FT_Done_FreeType(mLibrary);
}

FontFile& FTInstance::font(const std::string& filename)
{
    auto it = mFontFiles.find(filename);
    if (it != mFontFiles.end()) {
        return * it->second;
    }

    mFontFiles.emplace(filename, new FontFile(mLibrary, filename));
    
    return * mFontFiles.find(filename)->second;
}
