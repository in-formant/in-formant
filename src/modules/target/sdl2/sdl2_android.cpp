#include "sdl2.h"
#include <iostream>

#include <fcntl.h>
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

    std::cout << "test: " << SDL_AndroidGetInternalStoragePath() << std::endl;

    JNIEnv *env = (JNIEnv *) SDL_AndroidGetJNIEnv();
    jobject activity = (jobject) SDL_AndroidGetActivity();

    jclass clazz = env->GetObjectClass(activity);
    jmethodID methodId = env->GetMethodID(clazz, "getAssets", "()Landroid/content/res/AssetManager;");

    jobject assetManagerJava = env->CallObjectMethod(activity, methodId);

    AAssetManager *assetManager = AAssetManager_fromJava(env, assetManagerJava);

    std::vector<std::string> dirsToCopy { "" };
   
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
            int outfd = ::open(filePath.c_str(), O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU);
            while ((nbRead = AAsset_read(asset, buffer.get(), bufferSize)) > 0) {
                int nbToWrite = nbRead;
                int off = 0;
                while (nbToWrite > 0) {
                    int nbWritten = ::write(outfd, buffer.get() + off, nbToWrite);
                    nbToWrite -= nbWritten;
                    off += nbWritten;
                }
            }

            ::close(outfd);
            AAsset_close(asset);
        }

        AAssetDir_close(assetDir);
    }
}


