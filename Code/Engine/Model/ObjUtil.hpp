#pragma once

#include <vector>
#include <string>
#include <map>

#include "Engine/Core/Vertex_PCUTBN.hpp"

class VertexBuffer;
class IndexBuffer;
class Renderer;

class OBJ
{
public:
	OBJ() {};
	OBJ( std::string objPath, Renderer* renderer );
	~OBJ();

	virtual void Render() const;

protected:
	void LoadRawFile(std::string objPath);
	void AddVerts();

	int parts = -1;
	std::vector<std::vector<Vertex_PCUTBN>> m_vertexes;
	std::vector<std::vector<unsigned int>> m_indexes;
	std::map<std::string, std::string> m_texturePaths;
	std::vector<std::string> m_textureNames;
	
	std::vector<VertexBuffer*> m_vertexBuffers;
	std::vector<IndexBuffer*> m_indexBuffers;

	Renderer* m_renderer = nullptr;

	std::string m_rootPath;
	std::string m_rawOBJ;
	std::string m_rawMTL;
	std::string m_rawChecklist;
};