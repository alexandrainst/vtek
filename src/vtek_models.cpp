#include "vtek_vulkan.pch"
#include "vtek_models.hpp"

#include "vtek_logging.hpp"

// ASSIMP
#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags
#include <assimp/version.h>
// #include <assimp/pbrmaterial.h> // <deprecated>
#include <assimp/material.h>
#include <assimp/GltfMaterial.h>
#include <assimp/StringUtils.h>

// STANDARD
#include <vector>


/* struct implementation */
// TODO: Temporary placement and idea
struct Mesh
{
	std::vector<glm::vec3> vertices;
};

struct vtek::Model
{
	std::vector<Mesh> meshes;
};



/* helper functions */
static bool load_scene_node()
{

}

static bool load_scene_meshes(vtek::Model* model, const aiScene* scene)
{
	aiNode* node = scene->mRootNode;

	// A node contains 0..x meshes, each of which we add to the model
	for (unsigned int i = 0; i < node->mNumMeshes; i++)
	{
		aiMesh* ai_mesh = scene->mMeshes[node->mMeshes[i]];
	}

	return false;
}



/* models interface */
vtek::Model* vtek::model_load_obj(
	vtek::Directory* directory, std::string_view filename, vtek::Device* device)
{
	Assimp::Importer importer;

	// Usage flags for Assimp:

	unsigned int read_flags = 0;
	// models should probably always be triangulated if they aren't already
	// NOTE: Assimp's built-in triangulation doesn't handle polygons that are
	// self-intersecting or non-planar. So best to ensure asset validity
	// before importing.
	read_flags |= aiProcess_Triangulate;

	// Save some space
	read_flags |= aiProcess_JoinIdenticalVertices;

	// NOTE: Optional UV-flip, might not be needed for Vulkan!
	//read_flags |= aiProcess_FlipUVs;

	// NOTE: Optional tangent space for normal mapping (I think?)
	//read_flags |= aiProcess_CalcTangentSpace;

	// TODO: What does this do?
	//read_flags |= aiProcess_SortByPType;

	// NOTE: Default face winding is counter-clockwise, can be changed.
	//read_flags |= aiProcess_FlipWindingOrder;

	// NOTE: If left-handed coordinate system is needed (e.g. DirectX).
	//read_flags |= aiProcess_ConvertToLeftHanded;

	if (!vtek::file_exists(directory, filename))
	{
		vtek_log_error("Cannot load model from obj - file does not exist!");
		return nullptr;
	}

	std::string path = vtek::directory_get_path(directory, filename);
	const aiScene* scene = importer.ReadFile(path, read_flags);
	if (scene == nullptr || scene->mRootNode == nullptr ||
	    (scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE))
	{
		vtek_log_error("Failed to load scene with assimp: {}",
		               importer.GetErrorString());
		return nullptr;
	}

	auto model = new vtek::Model;
	if (!load_scene_meshes(model, scene))
	{


		delete model;
	}

	return nullptr;
}

void vtek::model_destroy(vtek::Model* model)
{

}
