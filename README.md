# InFormant™ - a real-time pitch and formant tracking software

# THIS BRANCH IS CURRENTLY NOT MAINTAINED - CURRENT WORK IS HAPPENING ON THE [WITH-QT](/../../tree/with-qt) BRANCH.

## How to compile from source

### Preparing the build environment

You must have Docker installed.

Build the Docker images for the platforms that you want to build for. (read `build-docker-images.sh` for an example of how to build them - but don't use the script as-is!)

### Building the project

Run the `build.sh` script from the project root.

The script takes two arguments:
- The first one is the target platform: must be one of `linux`, `win32`, `win64`, `macos`, `android`, `emscripten`.
- The second one is the build configuration: must be one of `Release`, `RelWithDebInfo`, `Debug`.
