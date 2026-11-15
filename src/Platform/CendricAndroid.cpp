#include "Platform/CendricAndroid.h"

#ifdef __ANDROID__

#include <SFML/System/NativeActivity.hpp>
#include <android/native_activity.h>
#include <android/asset_manager.h>
#include <android/log.h>
#include <sys/stat.h>
#include <errno.h>
#include <fstream>
#include <sstream>

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, "CendricAndroid", __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, "CendricAndroid", __VA_ARGS__)

namespace {
	std::string g_androidBasePath;

	void mkdirRecursive(const std::string& path) {
		if (path.empty()) return;
		std::string current;
		for (size_t i = 0; i < path.size(); ++i) {
			current += path[i];
			if (path[i] == '/' && i > 0) {
				mkdir(current.c_str(), 0755);
			}
		}
		mkdir(current.c_str(), 0755);
	}

	bool fileExists(const std::string& path) {
		struct stat st;
		return stat(path.c_str(), &st) == 0;
	}

	bool copyAssetFile(AAssetManager* mgr, const std::string& assetPath, const std::string& destPath) {
		size_t slash = destPath.find_last_of('/');
		if (slash != std::string::npos) {
			mkdirRecursive(destPath.substr(0, slash));
		}

		AAsset* asset = AAssetManager_open(mgr, assetPath.c_str(), AASSET_MODE_STREAMING);
		if (!asset) {
			LOGE("Failed to open asset: %s", assetPath.c_str());
			return false;
		}

		std::ofstream out(destPath, std::ios::binary);
		if (!out.is_open()) {
			LOGE("Failed to open destination file: %s", destPath.c_str());
			AAsset_close(asset);
			return false;
		}

		char buffer[16384];
		int bytesRead = 0;
		while ((bytesRead = AAsset_read(asset, buffer, sizeof(buffer))) > 0) {
			out.write(buffer, bytesRead);
		}

		out.close();
		AAsset_close(asset);
		return true;
	}
}

void initAndroidAssets(std::string& resourcePath, std::string& documentsPath) {
	ANativeActivity* activity = sf::getNativeActivity();
	if (!activity || !activity->internalDataPath) {
		LOGE("NativeActivity or internalDataPath is null!");
		return;
	}

	g_androidBasePath = activity->internalDataPath;
	if (!g_androidBasePath.empty() && g_androidBasePath.back() != '/') {
		g_androidBasePath += '/';
	}

	mkdirRecursive(g_androidBasePath + "saves");

	const std::string markerPath = g_androidBasePath + ".assets_extracted_v1";
	if (!fileExists(markerPath)) {
		LOGI("Extracting Cendric assets to %s", g_androidBasePath.c_str());
		AAsset* manifest = AAssetManager_open(activity->assetManager, "asset_manifest.txt", AASSET_MODE_BUFFER);
		if (manifest) {
			off_t length = AAsset_getLength(manifest);
			std::string content(static_cast<size_t>(length), '\0');
			AAsset_read(manifest, &content[0], static_cast<size_t>(length));
			AAsset_close(manifest);

			std::istringstream iss(content);
			std::string line;
			while (std::getline(iss, line)) {
				if (!line.empty() && line.back() == '\r') {
					line.pop_back();
				}
				if (!line.empty()) {
					copyAssetFile(activity->assetManager, line, g_androidBasePath + line);
				}
			}

			std::ofstream marker(markerPath);
			marker << "ok\n";
			marker.close();
			LOGI("Asset extraction completed.");
		} else {
			LOGE("asset_manifest.txt not found in APK assets!");
		}
	}

	resourcePath = g_androidBasePath;
	documentsPath = g_androidBasePath;
}

std::string getExternalDocumentsPath() {
	return g_androidBasePath;
}

std::string getSystemResourcePath() {
	return g_androidBasePath;
}

#endif
