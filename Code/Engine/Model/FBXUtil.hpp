#pragma once

#include <string>
#include <vector>
#include "fbxsdk.h"

#include "Engine/Core/Vertex_PCUTBN.hpp"

class Renderer;
class VertexBuffer;

class FBX
{
public:
	FBX() {};
	FBX( std::string filePath, Renderer* renderer );
	~FBX();

	virtual void Render() const;

public:
	FbxManager* m_fbxManager = nullptr;
	FbxScene* m_fbxScene = nullptr;

protected:

private:
	std::string m_filePath;
	bool Initialize();
	bool LoadScene( char const* pSceneName );

	void ProcessNode( FbxNode* pNode );
	void ProcessMesh( FbxNode* pNode );

	void ReadVertex( FbxMesh* pMesh, int ctrlPointIndex, Vec3& position );
	void ReadColor( FbxMesh* pMesh, int ctrlPointIndex, int vertexCount, Rgba8& color );
	void ReadUVs( FbxMesh* pMesh, int ctrlPointIndex, int textureUVIndex, int UVLayer, Vec2& UVs );
	void ReadTangent( FbxMesh* pMesh, int ctrlPointIndex, int vertexCount, Vec3& tangent );
	void ReadBinormal( FbxMesh* pMesh, int ctrlPointIndex, int vertexCount, Vec3& binormal );
	void ReadNormal( FbxMesh* pMesh, int ctrlPointIndex, int vertexCount, Vec3& normal );

private:
	Renderer* m_renderer = nullptr;

	std::vector<Vertex_PCUTBN> m_verts;
	VertexBuffer* m_vertexBuffer = nullptr;
};
