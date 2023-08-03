#include "vtek_vulkan.pch"
#include "vtek_models.hpp"

#include "vtek_buffer.hpp"
#include "vtek_logging.hpp"

// ASSIMP (git submodule)
#include "assimp/Importer.hpp"      // C++ importer interface
#include "assimp/scene.h"           // Output data structure
#include "assimp/postprocess.h"     // Post processing flags
#include "assimp/version.h"
// #include <assimp/pbrmaterial.h> // <deprecated> // TODO: Remove!
#include "assimp/material.h"
#include "assimp/GltfMaterial.h"
#include "assimp/StringUtils.h"

// STANDARD
#include <vector>


/* struct implementation */
struct vtek::Model
{
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec3> normals;

	uint32_t numVertices {0U};

	vtek::Buffer* vertexBuffer {nullptr};
	vtek::Buffer* normalBuffer {nullptr};
};



/* helper functions */
static void load_scene_mesh(
	vtek::Model* model, aiMesh* mesh, const vtek::ModelInfo* info)
{
	//bool vertices = true; // REVIEW: Why would we ever _not_ want to load vertex positions!
	bool normals = mesh->HasNormals() && info->loadNormals;
	//bool texCoords = false; // TODO: Needs an image loader in place in vtek first!
	//bool tangents = false; // TODO: Needs an implementation of normal mapping!

	for (unsigned int i = 0; i < mesh->mNumVertices; i++)
	{
		model->vertices.push_back(glm::vec3(
			mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z));

		if (normals)
		{
			model->normals.push_back(glm::vec3(
				mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z));
		}
	}
}

static void load_scene_node(
	vtek::Model* model, const aiScene* scene, aiNode* node,
	const vtek::ModelInfo* info)
{
	// A node contains 0..x meshes, each of which we add to the model
	for (unsigned int i = 0; i < node->mNumMeshes; i++)
	{
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		load_scene_mesh(model, mesh, info);
		// NEXT: Load materials
	}

	// Recurse on each child node
	for (unsigned int i = 0; i < node->mNumChildren; i++)
	{
		load_scene_node(model, scene, node->mChildren[i], info);
	}
}

static bool create_buffers(
	vtek::Model* model, const vtek::ModelInfo* info, vtek::Device* device)
{
	// Positions
	vtek::BufferInfo bufferInfo{};
	bufferInfo.size = sizeof(glm::vec3) * model->vertices.size();
	bufferInfo.writePolicy = vtek::BufferWritePolicy::write_once;

	using BUFlag = vtek::BufferUsageFlag;
	bufferInfo.usageFlags = BUFlag::transfer_dst | BUFlag::vertex_buffer;

	model->vertexBuffer = vtek::buffer_create(&bufferInfo, device);
	if (model->vertexBuffer == nullptr)
	{
		log_error("Failed to create vertex buffer for model vertices!");
		return false;
	}

	vtek::BufferRegion region {
		.offset = 0,
		.size = bufferInfo.size
	};
	// TODO: We could do `vtek::buffer_write_data(buffers[], void* data[], region[], device)` ?
	if (!vtek::buffer_write_data(
		    model->vertexBuffer, model->vertices.data(), &region, device))
	{
		log_error("Failed to write data to model vertex buffer!");
		return false;
	}

	// Normals
	if (info->loadNormals && model->normals.size() > 0)
	{
		model->normalBuffer = vtek::buffer_create(&bufferInfo, device);
		if (model->normalBuffer == nullptr)
		{
			log_error("Failed to create vertex buffer for model normals!");
			return false;
		}

		if (!vtek::buffer_write_data(
			    model->normalBuffer, model->normals.data(), &region, device))
		{
			log_error("Failed to write data to model normal buffer!");
			return false;
		}
	}

	return true;
}

static void destroy_model_buffers(vtek::Model* model, vtek::Device* device)
{
	if (model->vertexBuffer != nullptr)
	{
		vtek::buffer_destroy(model->vertexBuffer);
		model->vertexBuffer = nullptr;
	}
	if (model->normalBuffer != nullptr)
	{
		vtek::buffer_destroy(model->normalBuffer);
		model->normalBuffer = nullptr;
	}
}



/* models interface */
vtek::Model* vtek::model_load_obj(
	const vtek::ModelInfo* info, vtek::Directory* directory,
	std::string_view filename, vtek::Device* device)
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
	//read_flags |= aiProcess_JoinIdenticalVertices;

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
	// TODO: Pre-routine where we allocate space for all mesh data!
	// TODO: This will remove need for std::vector reallocations!
	load_scene_node(model, scene, scene->mRootNode, info);
	if (!create_buffers(model, info, device))
	{
		vtek_log_error("Failed to create buffers for loaded model!");
		destroy_model_buffers(model, device);
		delete model;
		return nullptr;
	}

	model->numVertices = model->vertices.size();
	model->vertices.clear();
	model->normals.clear();

	return model;
}

void vtek::model_destroy(vtek::Model* model, vtek::Device* device)
{
	if (model == nullptr) { return; }

	destroy_model_buffers(model, device);
	delete model;
}

const vtek::Buffer* vtek::model_get_vertex_buffer(vtek::Model* model)
{
	return model->vertexBuffer;
}

const vtek::Buffer* vtek::model_get_normal_buffer(vtek::Model* model)
{
	return model->normalBuffer;
}

uint32_t vtek::model_get_num_vertices(vtek::Model* model)
{
	return model->numVertices;
}
