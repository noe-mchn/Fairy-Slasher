#pragma once
#include <string>
#include <vector>
#include <filesystem>
#include <iostream>
#include <tiny_obj_loader.h>

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

	if (!warn.empty())
		std::cout << "[OBJ MTL] Warning: " << warn << std::endl;
	if (!err.empty())
		std::cerr << "[OBJ MTL] Error: " << err << std::endl;
	if (!ok)
		throw std::runtime_error("Failed to parse .obj materials: " + warn + err);

	std::cout << "[OBJ MTL] " << objRelativePath << " : "
		<< shapes.size() << " shapes, " << materials.size() << " materials" << std::endl;

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

					if (info.diffuseTexturePath.empty())
						std::cerr << "  [WARN] shape [" << shape.name
							<< "] texture \"" << mat.diffuse_texname
							<< "\" NOT FOUND anywhere!" << std::endl;
				}

				std::cout << "  shape [" << shape.name << "] -> material \""
					<< mat.name << "\" -> texture \""
					<< (info.diffuseTexturePath.empty() ? "(none)" : info.diffuseTexturePath)
					<< "\"" << std::endl;
			}
		}
		else
		{
			std::cout << "  shape [" << shape.name << "] -> no material assigned" << std::endl;
		}

		result.push_back(info);
	}

	return result;
}
