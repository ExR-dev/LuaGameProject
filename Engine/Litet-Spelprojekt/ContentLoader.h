#pragma once

#include "MeshD3D11.h"

//#define REPORT_UNRECOGNIZED

[[nodiscard]] bool LoadMeshFromFile(const char *path, MeshData &meshData);
[[nodiscard]] bool WriteMeshToFile(const char *path, const MeshData &meshData);

[[nodiscard]] bool LoadTextureFromFile(const char *path, UINT &width, UINT &height, std::vector<unsigned char> &data);
[[nodiscard]] bool LoadTextureFromFile(const char *path, UINT &width, UINT &height, std::vector<unsigned short> &data);
[[nodiscard]] bool LoadTextureFromFile(const char *path, UINT &width, UINT &height, std::vector<float> &data, int nChannels, bool highPrecision);

[[nodiscard]] bool WriteTextureToFile(const char *path, const UINT &width, const UINT &height, const std::vector<float> &data, int nChannels, bool highPrecision);
