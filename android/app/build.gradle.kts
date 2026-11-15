plugins {
    id("com.android.application")
}

android {
    namespace = "com.cendric.android"
    compileSdk = 35
    ndkVersion = "26.3.11579264"

    defaultConfig {
        applicationId = "com.cendric.android"
        minSdk = 26
        targetSdk = 35
        versionCode = 1
        versionName = "1.0-port"

        ndk {
            abiFilters += listOf("arm64-v8a")
        }

        externalNativeBuild {
            cmake {
                arguments += listOf(
                    "-DANDROID_STL=c++_shared",
                    "-DBUILD_SHARED_LIBS=ON",
                    "-DCENDRIC_BUILD_DIALOGUE_TOOL=OFF",
                    "-DCENDRIC_BUILD_SQLITE_SHELL=OFF"
                )
            }
        }
    }

    buildTypes {
        release {
            isMinifyEnabled = false
        }
    }

    externalNativeBuild {
        cmake {
            path = file("../../CMakeLists.txt")
            System.getenv("ANDROID_CMAKE_VERSION")?.let { version = it }
        }
    }
}
