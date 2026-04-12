#pragma once
#include <string>
#include <vector>
#include <filesystem>
#include <iostream>
#include <tiny_obj_loader.h>
#include <array>
#include <algorithm>
#include <glm/vec3.hpp>

struct ObjSubMeshMaterial {
	std::string diffuseTexturePath; // relative to resources root, empty if none
	std::string shapeName;
};

// Tries to find a texture file in common locations relative to the resources root.
// Returns the path relative to resourcesRoot if found, or empty string if not found.
inline std::string ResolveTexturePath(
	const std::string& texName,
	const std::string& mtlBaseDir,
	const std::filesystem::path& resourcesRoot)
{
	namespace fs = std::filesystem;

	fs::path texPath(texName);

	// If the path is absolute (e.g. from another dev's Blender export), extract just the filename
	std::string searchName = texPath.is_absolute() ? texPath.filename().string() : texName;

	// 1) Try relative to .mtl directory (standard behavior, only for relative paths)
	if (!texPath.is_absolute())
	{
		fs::path candidate = fs::weakly_canonical(fs::path(mtlBaseDir) / searchName);
		if (fs::exists(candidate))
			return fs::relative(candidate, resourcesRoot).string();
	}

	// 2) Try in Textures/ folder (common convention)
	{
		fs::path candidate = resourcesRoot / "Textures" / fs::path(searchName).filename();
		if (fs::exists(candidate))
			return fs::relative(candidate, resourcesRoot).string();
	}

	// 3) Try at the resources root
	{
		fs::path candidate = resourcesRoot / fs::path(searchName).filename();
		if (fs::exists(candidate))
			return fs::relative(candidate, resourcesRoot).string();
	}

	// Not found anywhere
	return "";
}

inline std::vector<std::pair<std::string, std::array<float, 3>>> GetShapePositions(
	const std::string& objRelativePath,
	const std::filesystem::path& resourcesRoot,
	const std::string& nameFilter)
{
	std::filesystem::path objAbsPath = resourcesRoot / objRelativePath;
	std::string mtlBaseDir = objAbsPath.parent_path().string();

	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string warn, err;

	bool ok = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err,
		objAbsPath.string().c_str(), mtlBaseDir.c_str(), true);
	if (!ok)
		return {};

	std::vector<std::pair<std::string, std::array<float, 3>>> result;
	for (const auto& shape : shapes)
	{
		if (shape.name.find(nameFilter) == std::string::npos)
			continue;

		float minX = 1e30f, minY = 1e30f, minZ = 1e30f;
		float maxX = -1e30f, maxY = -1e30f, maxZ = -1e30f;

		for (const auto& idx : shape.mesh.indices)
		{
			float x = attrib.vertices[3 * idx.vertex_index + 0];
			float y = attrib.vertices[3 * idx.vertex_index + 1];
			float z = attrib.vertices[3 * idx.vertex_index + 2];
			if (x < minX) minX = x; if (x > maxX) maxX = x;
			if (y < minY) minY = y; if (y > maxY) maxY = y;
			if (z < minZ) minZ = z; if (z > maxZ) maxZ = z;
		}

		result.push_back({ shape.name, { (minX + maxX) / 2, (minY + maxY) / 2, (minZ + maxZ) / 2 } });
	}
	return result;
}

// Parses an .obj and its associated .mtl to extract per-shape (= per sub-mesh) texture info.
// objRelativePath: e.g. "Models/Map.obj" (relative to resources root, same as MeshLoader::Load)
// resourcesRoot: absolute path to the Ressources folder
// Returns one entry per shape in the same order as MeshLoader creates sub-meshes.
inline std::vector<ObjSubMeshMaterial> GetObjMaterials(
	const std::string& objRelativePath,
	const std::filesystem::path& resourcesRoot)
{
	std::filesystem::path objAbsPath = resourcesRoot / objRelativePath;
	std::string mtlBaseDir = objAbsPath.parent_path().string();

	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string warn, err;

	bool ok = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err,
		objAbsPath.string().c_str(), mtlBaseDir.c_str(), true);

	if (!err.empty())
		std::cerr << "[OBJ MTL] Error: " << err << std::endl;
	if (!ok)
		throw std::runtime_error("Failed to parse .obj materials: " + warn + err);

	std::vector<ObjSubMeshMaterial> result;
	result.reserve(shapes.size());

	for (const auto& shape : shapes)
	{
		ObjSubMeshMaterial info;
		info.shapeName = shape.name;

		// Use the first face's material as the dominant material for this shape
		if (!shape.mesh.material_ids.empty() && shape.mesh.material_ids[0] >= 0)
		{
			int matId = shape.mesh.material_ids[0];
			if (matId < static_cast<int>(materials.size()))
			{
				const auto& mat = materials[matId];
				if (!mat.diffuse_texname.empty())
				{
					info.diffuseTexturePath = ResolveTexturePath(
						mat.diffuse_texname, mtlBaseDir, resourcesRoot);
				}

					}
				}
				else
				{
				}

		result.push_back(info);
	}

	return result;
}

// Returns the bounding-box center of an entire OBJ model (all shapes combined).
// Useful to compute the offset between bounding-box center and local origin.
inline glm::vec3 GetModelBBoxCenter(
	const std::string& objRelativePath,
	const std::filesystem::path& resourcesRoot)
{
	std::filesystem::path objAbsPath = resourcesRoot / objRelativePath;
	std::string mtlBaseDir = objAbsPath.parent_path().string();

	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string warn, err;

	if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err,
		objAbsPath.string().c_str(), mtlBaseDir.c_str(), true))
		return glm::vec3(0);

	glm::vec3 minP(1e30f, 1e30f, 1e30f), maxP(-1e30f, -1e30f, -1e30f);
	for (size_t i = 0; i + 2 < attrib.vertices.size(); i += 3)
	{
		float x = attrib.vertices[i], y = attrib.vertices[i + 1], z = attrib.vertices[i + 2];
		if (x < minP.x) minP.x = x; if (x > maxP.x) maxP.x = x;
		if (y < minP.y) minP.y = y; if (y > maxP.y) maxP.y = y;
		if (z < minP.z) minP.z = z; if (z > maxP.z) maxP.z = z;
	}
	return (minP + maxP) * 0.5f;
}

struct ShapeAABB {
	std::string name;
	glm::vec3 min;
	glm::vec3 max;
};

inline std::vector<ShapeAABB> GetShapeAABBs(
	const std::string& objRelativePath,
	const std::filesystem::path& resourcesRoot,
	const std::vector<std::string>& nameFilters,
	const std::vector<std::string>& excludeFilters = {})
{
	std::filesystem::path objAbsPath = resourcesRoot / objRelativePath;
	std::string mtlBaseDir = objAbsPath.parent_path().string();

	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string warn, err;

	bool ok = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err,
		objAbsPath.string().c_str(), mtlBaseDir.c_str(), true);
	if (!ok)
		return {};

	std::vector<ShapeAABB> result;
	for (const auto& shape : shapes)
	{
		bool excluded = false;
		for (const auto& excl : excludeFilters)
			if (shape.name.find(excl) != std::string::npos) { excluded = true; break; }
		if (excluded) continue;

		bool matched = nameFilters.empty();
		for (const auto& filter : nameFilters)
			if (shape.name.find(filter) != std::string::npos) { matched = true; break; }
		if (!matched) continue;

		float minX = 1e30f, minY = 1e30f, minZ = 1e30f;
		float maxX = -1e30f, maxY = -1e30f, maxZ = -1e30f;

		for (const auto& idx : shape.mesh.indices)
		{
			float x = attrib.vertices[3 * idx.vertex_index + 0];
			float y = attrib.vertices[3 * idx.vertex_index + 1];
			float z = attrib.vertices[3 * idx.vertex_index + 2];
			if (x < minX) minX = x; if (x > maxX) maxX = x;
			if (y < minY) minY = y; if (y > maxY) maxY = y;
			if (z < minZ) minZ = z; if (z > maxZ) maxZ = z;
		}

		result.push_back({ shape.name, glm::vec3(minX, minY, minZ), glm::vec3(maxX, maxY, maxZ) });
	}
	return result;
}
