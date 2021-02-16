#!/bin/bash

set -e

cd /dist

/android/bin/androiddeployqt --input /build/android_deployment_settings.json --output /build/android-build
