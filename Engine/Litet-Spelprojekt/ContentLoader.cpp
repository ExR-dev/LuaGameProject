#include "stdafx.h"
#include "ContentLoader.h"

#include <vector>
#include <fstream>
#include <sstream>
#include <string>

#ifdef LEAK_DETECTION
#define new			DEBUG_NEW
#endif

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#pragma warning(disable: 6262)
#pragma warning(disable: 26819)
#include "stb_image.h"
#include "stb_image_write.h"
#pragma warning(default: 26819)
#pragma warning(default: 6262)


struct RawPosition	{ float	x, y, z; };
struct RawNormal	{ float	x, y, z; };
struct RawTexCoord	{ float	u, v;	 };
struct RawIndex		{ int	v, t, n; };


struct SubMaterial
{
	std::string
		mtlName,
		ambientPath,
		diffusePath,
		specularPath;
	float specularExponent;
};

struct MaterialGroup
{
	std::string mtlName;
	std::vector<SubMaterial> subMaterials;
};

struct FormattedVertex {
	float 
		px, py, pz,
		nx, ny, nz,
		tx, ty, tz,
		u, v;

	FormattedVertex() :
		px(0.0f), py(0.0f), pz(0.0f),
		nx(0.0f), ny(0.0f), nz(0.0f),
		tx(0.0f), ty(0.0f), tz(0.0f),
		u(0.0f), v(0.0f)
	{ }

	FormattedVertex(
		const float px, const float py, const float pz, 
		const float nx, const float ny, const float nz, 
		const float tx, const float ty, const float tz, 
		const float u, const float v) :
		px(px), py(py), pz(pz),
		nx(nx), ny(ny), nz(nz),
		tx(tx), ty(ty), tz(tz),
		u(u), v(v)
	{ }

	bool operator==(const FormattedVertex &other) const
	{
		if (px != other.px) return false;
		if (py != other.py) return false;
		if (pz != other.pz) return false;

		if (nx != other.nx) return false;
		if (ny != other.ny) return false;
		if (nz != other.nz) return false;

		if (tx != other.tx) return false;
		if (ty != other.ty) return false;
		if (tz != other.tz) return false;

		if (u != other.u) return false;
		if (v != other.v) return false;

		return true;
	}
};


static bool ReadWavefront(const char *path, 
	std::vector<RawPosition> &vertexPositions, 
	std::vector<RawTexCoord> &vertexTexCoords,
	std::vector<RawNormal> &vertexNormals,
	std::vector<std::vector<RawIndex>> &indexGroups,
	std::vector<std::string> &mtlGroups,
	std::string &mtlFile)
{
	std::ifstream fileStream(path);
	std::string line;

	bool begunReadingSubMesh = false;

	while (std::getline(fileStream, line))
	{
		size_t commentStart = line.find_first_of('#');
		if (commentStart != std::string::npos)
			line = line.substr(0, commentStart); // Exclude comments

		if (line.empty())
			continue; // Skip filler line

		if (line.length() > 1)
			if (line.at(0) == 'f' && line.at(1) == ' ')
			{
				char sep = '/';
				if (line.find('\\', 0) != std::string::npos)
					sep = '\\';

				// Replace all instances of separator with whitespace.
				std::string::size_type n = 0;
				while ((n = line.find(sep, n)) != std::string::npos)
				{
					line.replace(n, 1, " ");
					n++;
				}
			}

		std::istringstream segments(line);

		std::string dataType;
		if (!(segments >> dataType))
		{
			ErrMsg(std::format("Failed to get data type from line \"{}\", file \"{}\"!", line, path));
			return false;
		}

		if (dataType == "mtllib")
		{ // Define where to find materials
			if (!(segments >> mtlFile))
			{
				ErrMsg(std::format("Failed to get mtl name from line \"{}\", file \"{}\"!", line, path));
				return false;
			}
		}
		else if (dataType == "g")
		{ // Mesh Group
			begunReadingSubMesh = true;
			indexGroups.emplace_back();
			mtlGroups.emplace_back("");
		}
		else if (dataType == "o")
		{ // Mesh Object
			begunReadingSubMesh = true;
			indexGroups.emplace_back();
			mtlGroups.emplace_back("");
		}
		else if (dataType == "v")
		{ // Vertex Position
			float x, y, z;
			if (!(segments >> x >> y >> z))
			{
				ErrMsg(std::format(R"(Failed to get vertex position from line "{}", file "{}"!)", line, path));
				return false;
			}

			vertexPositions.push_back({ x, y, z });
		}
		else if (dataType == "vt")
		{ // Vertex Texture Coordinate
			float u, v;
			if (!(segments >> u >> v))
			{
				ErrMsg(std::format(R"(Failed to get texture coordinate from line "{}", file "{}"!)", line, path));
				return false;
			}

			vertexTexCoords.push_back({ u, v });
		}
		else if (dataType == "vn")
		{ // Vertex Normal
			float x, y, z;
			if (!(segments >> x >> y >> z))
			{
				ErrMsg(std::format(R"(Failed to get normal from line "{}", file "{}"!)", line, path));
				return false;
			}

			vertexNormals.push_back({ x, y, z });
		}
		else if (dataType == "f")
		{ // Index Group
			if (!begunReadingSubMesh)
			{
				ErrMsg(std::format(R"(Reached index group before creating submesh, file "{}"!)", path));
				return false;
			}

			std::vector<RawIndex> indicesInGroup;
			int vi, ti, ni;

			while (segments >> vi >> ti >> ni)
				indicesInGroup.push_back({ --vi, --ti, --ni });

			size_t groupSize = indicesInGroup.size();
			if (groupSize < 3 || groupSize > 4)
			{
				ErrMsg(std::format(R"(Unparseable group size '{}' at line "{}", file "{}"!)", groupSize, line, path));
				return false;
			}

			indexGroups.back().push_back(indicesInGroup.at(0));
			indexGroups.back().push_back(indicesInGroup.at(1));
			indexGroups.back().push_back(indicesInGroup.at(2));

			if (groupSize == 4)
			{ // Group is a quad
				indexGroups.back().push_back(indicesInGroup.at(0));
				indexGroups.back().push_back(indicesInGroup.at(2));
				indexGroups.back().push_back(indicesInGroup.at(3));
			}
		}
		else if (dataType == "usemtl")
		{ // Start of submesh with material
			if (!begunReadingSubMesh)
			{ // First submesh
				begunReadingSubMesh = true;
				indexGroups.emplace_back();
				mtlGroups.emplace_back("");
			}

			if (!(segments >> mtlGroups.back()))
			{
				ErrMsg(std::format("Failed to get sub-material name from line \"{}\", file \"{}\"!", line, path));
				return false;
			}
		}
#ifdef REPORT_UNRECOGNIZED
		else
		{
			ErrMsg(std::format(R"(Unimplemented object flag '{}' on line "{}", file "{}"!)", dataType, line, path));

		}
#endif
	}

	if (vertexPositions.empty())
		vertexPositions.push_back({ 0.0f, 0.0f, 0.0f });

	if (vertexTexCoords.empty())
		vertexTexCoords.push_back({ 0.0f, 0.0f });

	if (vertexNormals.empty())
		vertexNormals.push_back({ 0.0f, 1.0f, 0.0f });

	size_t groupCount = indexGroups.size();
	for (size_t i = 0; i < groupCount; i++)
	{
		if (mtlGroups.size() <= i)
			mtlGroups.emplace_back("default");
		else if (mtlGroups.at(i) == "")
			mtlGroups.at(i) = "default";
	}

	return true;
}


static bool ReadMaterial(const char *path, MaterialGroup &material)
{
	material.mtlName = static_cast<std::string>(path).substr(
		static_cast<std::string>(path).find_last_of('\\') + 1, 
		static_cast<std::string>(path).find_last_of('.')
	);

	const std::string folderPath = static_cast<std::string>(path).substr(
		0,
		static_cast<std::string>(path).find_last_of('\\') + 1
	);

	std::ifstream fileStream(path);
	std::string line;

	while (std::getline(fileStream, line))
	{
		size_t commentStart = line.find_first_of('#');
		if (commentStart != std::string::npos)
			line = line.substr(0, commentStart); // Exclude comments

		if (line.empty())
			continue; // Skip filler line

		std::istringstream segments(line);

		std::string dataType;
		if (!(segments >> dataType))
		{
			ErrMsg(std::format("Failed to get data type from line \"{}\", file \"{}\"!", line, path));
			return false;
		}

		if (dataType == "newmtl")
		{ // New Sub-material
			std::string mtlName;
			if (!(segments >> mtlName))
			{
				ErrMsg(std::format("Failed to get sub-material name from line \"{}\", file \"{}\"!", line, path));
				return false;
			}

			material.subMaterials.push_back({ mtlName, "", "", "", 0.0f });
		}
		else if (dataType == "map_Ka")
		{ 
			std::string mapPath;
			if (!(segments >> mapPath))
			{
				ErrMsg(std::format("Failed to get map_Ka path from line \"{}\", file \"{}\"!", line, path));
				return false;
			}

			if (material.subMaterials.empty())
				material.subMaterials.push_back({ "", "", "", "", 0.0f });
			material.subMaterials.back().ambientPath = folderPath + mapPath;
		}
		else if (dataType == "map_Kd")
		{ 
			std::string mapPath;
			if (!(segments >> mapPath))
			{
				ErrMsg(std::format("Failed to get map_Kd path from line \"{}\", file \"{}\"!", line, path));
				return false;
			}

			if (material.subMaterials.empty())
				material.subMaterials.push_back({ "", "", "", "", 0.0f });
			material.subMaterials.back().diffusePath = folderPath + mapPath;
		}
		else if (dataType == "map_Ks")
		{ 
			std::string mapPath;
			if (!(segments >> mapPath))
			{
				ErrMsg(std::format("Failed to get map_Ks path from line \"{}\", file \"{}\"!", line, path));
				return false;
			}

			if (material.subMaterials.empty())
				material.subMaterials.push_back({ "", "", "", "", 0.0f });
			material.subMaterials.back().specularPath = folderPath + mapPath;
		}
		else if (dataType == "Ns")
		{ 
			float exponent;
			if (!(segments >> exponent))
			{
				ErrMsg(std::format("Failed to get Ns value from line \"{}\", file \"{}\"!", line, path));
				return false;
			}

			if (material.subMaterials.empty())
				material.subMaterials.push_back({ "", "", "", "", 0.0f });

			material.subMaterials.back().specularExponent = exponent;
		}
#ifdef REPORT_UNRECOGNIZED
		else
		{
			ErrMsg(std::format(R"(Unimplemented material flag '{}' on line "{}", file "{}"!)", dataType, line, path));
		}
#endif
	}

	if (material.subMaterials.empty())
	{
		material.subMaterials.push_back({ "default", "", "", "", 50.0f});
	}

	return true;
}


struct FormattedIndexGroup {
	std::vector<uint32_t> indices;
	std::string mtlName;
};

static void FormatRawMesh(
	std::vector<FormattedVertex> &formattedVertices,
	std::vector<FormattedIndexGroup> &formattedIndexGroups,
	const std::vector<RawPosition> &vertexPositions,
	const std::vector<RawTexCoord> &vertexTexCoords,
	const std::vector<RawNormal> &vertexNormals,
	const std::vector<std::vector<RawIndex>> &indexGroups,
	const std::vector<std::string> &mtlGroups)
{
	// Format vertices & index groups
	const size_t groupCount = indexGroups.size();
	for (size_t groupIndex = 0; groupIndex < groupCount; groupIndex++)
	{
		formattedIndexGroups.emplace_back();
		FormattedIndexGroup *formattedGroup = &formattedIndexGroups.back();
		const std::vector<RawIndex> *rawGroup = &indexGroups.at(groupIndex);

		if (mtlGroups.size() > groupIndex)
			formattedGroup->mtlName = mtlGroups.at(groupIndex);

		const size_t startingVertex = formattedVertices.size();

		const size_t groupSize = rawGroup->size();
		for (size_t vertIndex = 0; vertIndex < groupSize; vertIndex++)
		{
			const RawIndex rI = rawGroup->at(vertIndex);
			const RawPosition rP = vertexPositions.at(rI.v);
			const RawTexCoord rT = vertexTexCoords.at(rI.t);
			const RawNormal rN = vertexNormals.at(rI.n);

			formattedGroup->indices.emplace_back(static_cast<uint32_t>(formattedVertices.size()));
			formattedVertices.emplace_back(
				rP.x, rP.y, rP.z,
				rN.x, rN.y, rN.z,
				0,	  0,    0,
				rT.u, rT.v
			);
		}

		// Generate tangents
		for (size_t triIndex = 0; triIndex < groupSize; triIndex += 3)
		{
			FormattedVertex *verts[3] = {
				&formattedVertices.at(startingVertex + triIndex + 0),
				&formattedVertices.at(startingVertex + triIndex + 1),
				&formattedVertices.at(startingVertex + triIndex + 2)
			};

			const DirectX::XMFLOAT3A
				v0 = { verts[0]->px, verts[0]->py, verts[0]->pz },
				v1 = { verts[1]->px, verts[1]->py, verts[1]->pz },
				v2 = { verts[2]->px, verts[2]->py, verts[2]->pz };

			const DirectX::XMFLOAT3A
				edge1 = { v1.x - v0.x, v1.y - v0.y, v1.z - v0.z },
				edge2 = { v2.x - v0.x, v2.y - v0.y, v2.z - v0.z };

			const DirectX::XMFLOAT2A
				uv0 = { verts[0]->u, verts[0]->v },
				uv1 = { verts[1]->u, verts[1]->v },
				uv2 = { verts[2]->u, verts[2]->v };

			const DirectX::XMFLOAT2A
				deltaUV1 = { uv1.x - uv0.x,	uv1.y - uv0.y },
				deltaUV2 = { uv2.x - uv0.x,	uv2.y - uv0.y };

			const float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);
			DirectX::XMFLOAT4A tangent = {
				f * (edge1.x * deltaUV2.y - edge2.x * deltaUV1.y),
				f * (edge1.y * deltaUV2.y - edge2.y * deltaUV1.y),
				f * (edge1.z * deltaUV2.y - edge2.z * deltaUV1.y),
				0.0f
			};

			for (size_t i = 0; i < 3; i++)
			{
				// Gram-Schmidt orthogonalization
				const DirectX::XMFLOAT4A normal = { verts[i]->nx, verts[i]->ny, verts[i]->nz, 0.0f };

				const DirectX::XMVECTOR
					n = *reinterpret_cast<const DirectX::XMVECTOR *>(&normal),
					t = *reinterpret_cast<const DirectX::XMVECTOR *>(&tangent);

				const DirectX::XMVECTOR newTangentVec = DirectX::XMVector3Normalize(
					DirectX::XMVectorSubtract(t,
						DirectX::XMVectorScale(n,
							DirectX::XMVectorGetX(
								DirectX::XMVector3Dot(n, t)))));

				const DirectX::XMFLOAT4A newTangent = *reinterpret_cast<const DirectX::XMFLOAT4A *>(&newTangentVec);

				verts[i]->tx = newTangent.x;
				verts[i]->ty = newTangent.y;
				verts[i]->tz = newTangent.z;
			}
		}
	}

	for (UINT i = 0; i < formattedIndexGroups.size(); i++)
		if (formattedIndexGroups.at(i).indices.empty())
			formattedIndexGroups.erase(formattedIndexGroups.begin() + i--);
}

static void SendFormattedMeshToMeshData(MeshData &meshData,
	const std::vector<FormattedVertex> &formattedVertices,
	const std::vector<FormattedIndexGroup> &formattedIndexGroups,
	const MaterialGroup &material)
{
	// Send vertex data to meshData
	meshData.vertexInfo.nrOfVerticesInBuffer = static_cast<UINT>(formattedVertices.size());
	meshData.vertexInfo.sizeOfVertex = sizeof(FormattedVertex);
	meshData.vertexInfo.vertexData = reinterpret_cast<float *>(new FormattedVertex[meshData.vertexInfo.nrOfVerticesInBuffer]);

	std::memcpy(
		meshData.vertexInfo.vertexData,
		formattedVertices.data(),
		meshData.vertexInfo.sizeOfVertex * meshData.vertexInfo.nrOfVerticesInBuffer
	);

	// Send index data to meshData
	std::vector<uint32_t> inlineIndices;

	const size_t groupCount = formattedIndexGroups.size();
	for (size_t group_i = 0; group_i < groupCount; group_i++)
	{
		MeshData::SubMeshInfo subMeshInfo = { };
		const FormattedIndexGroup *currGroup = &formattedIndexGroups.at(group_i);

		subMeshInfo.startIndexValue = static_cast<UINT>(inlineIndices.size());
		subMeshInfo.nrOfIndicesInSubMesh = static_cast<UINT>(currGroup->indices.size());

		SubMaterial const *currMat = nullptr;
		for (const SubMaterial &subMat : material.subMaterials)
			if (subMat.mtlName == currGroup->mtlName)
			{
				currMat = &subMat;
				break;
			}

		subMeshInfo.ambientTexturePath	= (currMat != nullptr) ? currMat->ambientPath		: "";
		subMeshInfo.diffuseTexturePath	= (currMat != nullptr) ? currMat->diffusePath		: "";
		subMeshInfo.specularTexturePath = (currMat != nullptr) ? currMat->specularPath		: "";
		subMeshInfo.specularExponent	= (currMat != nullptr) ? currMat->specularExponent	: 0.0f;

		inlineIndices.insert(inlineIndices.end(), currGroup->indices.begin(), currGroup->indices.end());
		meshData.subMeshInfo.push_back(subMeshInfo);
	}

	meshData.indexInfo.nrOfIndicesInBuffer = static_cast<UINT>(inlineIndices.size());
	meshData.indexInfo.indexData = new uint32_t[meshData.indexInfo.nrOfIndicesInBuffer];

	std::memcpy(
		meshData.indexInfo.indexData,
		inlineIndices.data(),
		sizeof(uint32_t) * meshData.indexInfo.nrOfIndicesInBuffer
	);

	// Calculate bounds
	DirectX::XMFLOAT4A
		min = {  FLT_MAX,  FLT_MAX,  FLT_MAX, 0 },
		max = { -FLT_MAX, -FLT_MAX, -FLT_MAX, 0 };

	for (const FormattedVertex &vData : formattedVertices)
	{
		if (vData.px < min.x)		min.x = vData.px;
		else if (vData.px > max.x)	max.x = vData.px;

		if (vData.py < min.y)		min.y = vData.py;
		else if (vData.py > max.y)	max.y = vData.py;

		if (vData.pz < min.z)		min.z = vData.pz;
		else if (vData.pz > max.z)	max.z = vData.pz;
	}

	float midX = (min.x + max.x) / 2.0f;
	float midY = (min.y + max.y) / 2.0f;
	float midZ = (min.z + max.z) / 2.0f;

	if (min.x >= max.x - 0.001f)
	{
		min.x = midX - 0.001f;
		max.x = midX + 0.001f;
	}
	if (min.y >= max.y - 0.001f)
	{
		min.y = midY - 0.001f;
		max.y = midY + 0.001f;
	}
	if (min.z >= max.z - 0.001f)
	{
		min.z = midZ - 0.001f;
		max.z = midZ + 0.001f;
	}

	DirectX::BoundingBox box;
	DirectX::BoundingBox().CreateFromPoints(
		box,
		*reinterpret_cast<DirectX::XMVECTOR *>(&min),
		*reinterpret_cast<DirectX::XMVECTOR *>(&max)
	);

	DirectX::BoundingOrientedBox minMaxBox;
	DirectX::BoundingOrientedBox().CreateFromBoundingBox(minMaxBox, box);

	meshData.boundingBox = minMaxBox;
}


bool LoadMeshFromFile(const char *path, MeshData &meshData)
{
	if (meshData.vertexInfo.vertexData != nullptr || 
		meshData.indexInfo.indexData != nullptr)
	{
		ErrMsg("meshData is not nullified!");
		return false;
	}

	std::string ext = path;
	ext.erase(0, ext.find_last_of('.') + 1);

	std::vector<RawPosition> vertexPositions;
	std::vector<RawTexCoord> vertexTexCoords;
	std::vector<RawNormal> vertexNormals;
	std::vector<std::vector<RawIndex>> indexGroups;
	std::vector<std::string> mtlGroups;

	if (ext == "obj")
	{
		if (!ReadWavefront(path, vertexPositions, vertexTexCoords, vertexNormals, indexGroups, mtlGroups, meshData.mtlFile))
		{
			ErrMsg("Failed to read wavefront file!");
			return false;
		}

		const std::string materialPath = path;
		meshData.mtlFile = materialPath.substr(0, materialPath.find_last_of('\\') + 1) + meshData.mtlFile;
	}
	else
	{
		ErrMsg(std::format("Unimplemented mesh file extension '{}'!", ext));
		return false;
	}

	MaterialGroup material;
	if (!ReadMaterial(meshData.mtlFile.c_str(), material))
	{
		ErrMsg("Failed to read material file!");
		return false;
	}

	std::vector<FormattedVertex> formattedVertices;
	std::vector<FormattedIndexGroup> formattedIndexGroups;

	FormatRawMesh(formattedVertices, formattedIndexGroups, vertexPositions, vertexTexCoords, vertexNormals, indexGroups, mtlGroups);

	SendFormattedMeshToMeshData(meshData, formattedVertices, formattedIndexGroups, material);

	return true;	
}

/// Debug Function
bool WriteMeshToFile(const char *path, const MeshData &meshData)
{
	if (meshData.vertexInfo.vertexData == nullptr || 
		meshData.indexInfo.indexData == nullptr)
	{
		ErrMsg("meshData is nullified!");
		return false;
	}

	std::ofstream fileStream(path);

	fileStream << "Loaded Mesh:\n";
	fileStream << "material = " << meshData.mtlFile << "\n\n";

	fileStream << "---------------- Vertex Data ----------------" << '\n';
	fileStream << "count = " << meshData.vertexInfo.nrOfVerticesInBuffer << '\n';
	fileStream << "size = " << meshData.vertexInfo.sizeOfVertex << "\n\n";

	for (size_t i = 0; i < meshData.vertexInfo.nrOfVerticesInBuffer; i++)
	{
		const FormattedVertex *vData = &reinterpret_cast<FormattedVertex*>(meshData.vertexInfo.vertexData)[i];

		fileStream << "Vertex " << i << '\n';

		fileStream << "\tPosition(" << vData->px << ", " << vData->py << ", " << vData->pz << ")\n";
		fileStream << "\tNormal(" << vData->nx << ", " << vData->ny << ", " << vData->nz << ")\n";
		fileStream << "\tTangent(" << vData->tx << ", " << vData->ty << ", " << vData->tz << ")\n";
		fileStream << "\tTexCoord(" << vData->u << ", " << vData->v << ")\n";

		fileStream << '\n';
	}
	fileStream << "---------------------------------------------\n\n";

	fileStream << "---------------- Index Data ----------------\n";
	fileStream << "count = " << meshData.indexInfo.nrOfIndicesInBuffer << '\n';

	for (size_t i = 0; i < meshData.indexInfo.nrOfIndicesInBuffer / 3; i++)
	{
		const uint32_t *iData = &meshData.indexInfo.indexData[i*3];

		fileStream << "indices " << i*3 << "-" << i*3+2 << "\t (" << iData[0] << "/" << iData[1] << "/" << iData[2] << ")\n";
	}
	fileStream << "--------------------------------------------\n\n";

	fileStream << "---------------- Triangle Data ----------------\n";
	fileStream << "count = " << meshData.indexInfo.nrOfIndicesInBuffer / 3 << '\n';

	for (size_t i = 0; i < meshData.indexInfo.nrOfIndicesInBuffer / 3; i++)
	{
		const uint32_t *iData = &meshData.indexInfo.indexData[i*3];

		fileStream << "Triangle " << i+1 << " {";

		for (size_t j = 0; j < 3; j++)
		{
			const FormattedVertex *vData = &reinterpret_cast<FormattedVertex *>(meshData.vertexInfo.vertexData)[iData[j]];
			fileStream << "\n\tv" << j << '\n';

			fileStream << "\t\tP Vector3(" << vData->px << ", " << vData->py << ", " << vData->pz << ")\n";
			fileStream << "\t\tN Vector3(" << vData->nx << ", " << vData->ny << ", " << vData->nz << ")\n";
			fileStream << "\t\tT Vector3(" << vData->tx << ", " << vData->ty << ", " << vData->tz << ")\n";
			fileStream << "\t\tu Vector3(" << vData->u << ", " << vData->v << ", 0)\n";
		}

		fileStream << "}\n\n";
	}
	fileStream << "--------------------------------------------\n\n\n";

	fileStream << "---------------- Submesh Data ----------------\n";
	for (size_t i = 0; i < meshData.subMeshInfo.size(); i++)
	{
		fileStream << "Submesh " << i << '\n';
		fileStream << "\tstart index = " << meshData.subMeshInfo.at(i).startIndexValue << '\n';
		fileStream << "\tlength = " << meshData.subMeshInfo.at(i).nrOfIndicesInSubMesh << '\n';
		fileStream << "\tambient = " << meshData.subMeshInfo.at(i).ambientTexturePath << '\n';
		fileStream << "\tdiffuse = " << meshData.subMeshInfo.at(i).diffuseTexturePath << '\n';
		fileStream << "\tspecular = " << meshData.subMeshInfo.at(i).specularTexturePath << '\n';
		fileStream << "\texponent = " << meshData.subMeshInfo.at(i).specularExponent << '\n';
		fileStream << '\n';
	}
	fileStream << "----------------------------------------------\n\n";
	
	fileStream.close();
	return true;
}


bool LoadTextureFromFile(const char *path, UINT &width, UINT &height, std::vector<unsigned char> &data)
{
	stbi_set_flip_vertically_on_load(1);

	int w, h, comp;
	unsigned char *imgData = stbi_load(path, &w, &h, &comp, STBI_rgb_alpha);
	if (imgData == nullptr)
	{
		ErrMsg(std::format("Failed to load texture from file at path \"{}\"!", path));
		return false;
	}

	width = static_cast<UINT>(w);
	height = static_cast<UINT>(h);
	data = std::vector(imgData, imgData + static_cast<size_t>(4ul * w * h));

	stbi_image_free(imgData);
	return true;
}
bool LoadTextureFromFile(const char *path, UINT &width, UINT &height, std::vector<unsigned short> &data)
{
	stbi_set_flip_vertically_on_load(1);

	int w, h, comp;
	unsigned short *imgData = stbi_load_16(path, &w, &h, &comp, STBI_rgb_alpha);
	if (imgData == nullptr)
	{
		ErrMsg(std::format("Failed to load texture from file at path \"{}\"!", path));
		return false;
	}

	width = static_cast<UINT>(w);
	height = static_cast<UINT>(h);
	data = std::vector(imgData, imgData + static_cast<size_t>(4ul * w * h));

	stbi_image_free(imgData);
	return true;
}
bool LoadTextureFromFile(const char *path, UINT &width, UINT &height, std::vector<float> &data, int nChannels, bool highPrecision)
{
	if (nChannels < 1 || nChannels > 4)
	{
		ErrMsg(std::format("Trying to read incorrect number of channels: {}!", nChannels));
		return false;
	}

	stbi_set_flip_vertically_on_load(1);

	int w, h, comp;

	if (highPrecision)
	{
		unsigned short *imgData = stbi_load_16(path, &w, &h, &comp, nChannels);

		if (imgData == nullptr)
		{
			ErrMsg(std::format("Failed to load texture from file at path \"{}\"!", path));
			return false;
		}

		width = static_cast<UINT>(w);
		height = static_cast<UINT>(h);

		std::vector<unsigned short> temp = std::vector(imgData, imgData + static_cast<size_t>(nChannels * w * h));
		data.resize(temp.size());
		for (int i = 0; i < temp.size(); i++)
			data[i] = (float)temp[i] / 65535.0f;

		stbi_image_free(imgData);
	}
	else
	{
		unsigned char *imgData = stbi_load(path, &w, &h, &comp, nChannels);

		if (imgData == nullptr)
		{
			ErrMsg(std::format("Failed to load texture from file at path \"{}\"!", path));
			return false;
		}

		width = static_cast<UINT>(w);
		height = static_cast<UINT>(h);

		std::vector<unsigned char> temp = std::vector(imgData, imgData + static_cast<size_t>(nChannels * w * h));
		data.resize(temp.size());
		for (int i = 0; i < temp.size(); i++)
			data[i] = (float)temp[i] / 255.0f;

		stbi_image_free(imgData);
	}
	return true;
}

// Helper function to convert float to uint16_t
uint16_t FloatToUint16(float value) {
    return static_cast<uint16_t>(std::clamp(value, 0.0f, 1.0f) * 65535.0f);
}

// Helper function to convert float to uint8_t
uint8_t FloatToUint8(float value) {
    return static_cast<uint8_t>(std::clamp(value, 0.0f, 1.0f) * 255.0f);
}

bool WriteTextureToFile(const char *path, const UINT &width, const UINT &height, const std::vector<float> &data, int nChannels, bool highPrecision)
{
    const int rowSize = width * nChannels * (highPrecision ? sizeof(uint16_t) : sizeof(uint8_t));
    
    // Convert float data to 8-bit format
    std::vector<uint8_t> convertedData(width * height * nChannels);
    for (size_t i = 0; i < convertedData.size(); ++i) {
        convertedData[i] = FloatToUint8(data[i]);
    }

    // Flip the image vertically
    std::vector<uint8_t> flippedData(convertedData.size());
    for (UINT y = 0; y < height; ++y) {
        std::memcpy(&flippedData[y * rowSize], &convertedData[(height - 1 - y) * rowSize], rowSize);
    }

    // Write flipped image to PNG
    if (!stbi_write_png(path, width, height, nChannels, flippedData.data(), rowSize)) {
        ErrMsg(std::format("Failed to write to texture \"{}\"!", path));
        return false;
    }

    return true;
}