#pragma once
#include <vector>
#include <optional>
#include <map>
#include <string>

#include <glm/glm.hpp>

namespace
{
	using Vertex = glm::vec3;
	using Vertices = std::vector<Vertex>;

	using ColorVertex = glm::vec4;
	using ColorVertices = std::vector<ColorVertex>;

	using NormalVertex = glm::vec3;
	using NormalVertices = std::vector<NormalVertex>;

	using TextureVertex  = glm::vec2;
	using TextureVertices = std::vector<TextureVertex>;


	using Indices = std::vector<uint32_t>;

	enum class TextureType
	{
		DIFFUSE,
		SPECULAR,
		AMBIENT,
		UNKNOWN,
		MAX_TEXTURES
	};
	TextureType& operator++ (TextureType& tt)
	{
		tt = static_cast<TextureType>(static_cast<int>(tt));
		return tt;
	}
	using FilePath = std::string;
	using Textures = std::map<TextureType, FilePath>;
}

struct Mesh
{
	Vertices vertices;
	ColorVertices colorVertices;
	NormalVertices normalVertices;
	TextureVertices textureVertices;

	Indices indices;
	Textures textures;
};

