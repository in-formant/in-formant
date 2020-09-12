#include "freetype.h"
#include "../target/base/base.h"
#include <stdexcept>
#include <cmath>

using namespace Module::Freetype;

void Module::Freetype::checkError(FT_Error error)
{
    if (error) {
        throw std::runtime_error(std::string("Font::Freetype] ") + FT_Error_String(error));
    }
}

FTInstance::FTInstance(Module::Target::AbstractBase *target)
{
    FT_Init_FreeType(&mLibrary);
   
    float hdpi, vdpi; 
    target->getDisplayDPI(&hdpi, &vdpi, nullptr);

    mHorizontalDPI = std::round(hdpi);
    mVerticalDPI = std::round(vdpi);
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

    mFontFiles.emplace(filename, new FontFile(mLibrary, filename, mHorizontalDPI, mVerticalDPI));
    
    return * mFontFiles.find(filename)->second;
}
