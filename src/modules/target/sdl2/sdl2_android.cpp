#include "sdl2.h"
#include <iostream>

#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <jni.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>

using namespace Module::Target;

void SDL2::prepareAssets()
{
    chdir(SDL_AndroidGetInternalStoragePath());

    JNIEnv *env = (JNIEnv *) SDL_AndroidGetJNIEnv();
    jobject activity = (jobject) SDL_AndroidGetActivity();

    jclass clazz = env->GetObjectClass(activity);
    jmethodID methodId = env->GetMethodID(clazz, "getAssets", "()Landroid/content/res/AssetManager;");

    jobject assetManagerJava = env->CallObjectMethod(activity, methodId);

    AAssetManager *assetManager = AAssetManager_fromJava(env, assetManagerJava);

    std::vector<std::string> dirsToCopy { "", "shaders", "shaders/vulkan", "shaders/gles" };
   
    constexpr size_t bufferSize = 1024;
    auto buffer = std::make_unique<char[]>(bufferSize);

    for (const auto& dirPath : dirsToCopy) {
        AAssetDir *assetDir = AAssetManager_openDir(assetManager, dirPath.c_str());

        mkdir(dirPath.c_str(), S_IRWXU);

        const char *filename = nullptr;
        while ((filename = AAssetDir_getNextFileName(assetDir)) != nullptr) {
            std::string filePath = dirPath.empty() ? filename : (dirPath + "/" + filename);

            AAsset *asset = AAssetManager_open(assetManager, filePath.c_str(), AASSET_MODE_STREAMING);

            int nbRead;
            FILE *out = fopen(filePath.c_str(), "w");
            while ((nbRead = AAsset_read(asset, buffer.get(), bufferSize)) > 0) {
                fwrite(buffer.get(), nbRead, 1, out);
            }

            fclose(out);
            AAsset_close(asset);
        }

        AAssetDir_close(assetDir);
    }
}


