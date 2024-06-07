#pragma comment(lib,"libfbxsdk.lib")
#include "Engine/Model/FBXUtil.hpp"
#include "Engine/Core/Vertex_PCUTBN.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/VertexBuffer.hpp"

FBX::FBX( std::string filePath, Renderer* renderer )
{
	m_renderer = renderer;
	m_filePath = filePath;
	Initialize();
	LoadScene( "bruh" );
	ProcessNode( m_fbxScene->GetRootNode() );
}

FBX::~FBX()
{
	delete m_vertexBuffer;
	m_vertexBuffer = nullptr;
}

void FBX::Render() const
{
	m_renderer->BindTexture( nullptr );
 	m_renderer->DrawVertexBuffer( m_vertexBuffer, (int)m_verts.size(), 0 );
}

bool FBX::Initialize()
{
	if (m_fbxManager)
	{
		m_fbxManager->Destroy();
	}
	m_fbxManager = FbxManager::Create();
	if (m_fbxManager == NULL)
	{
		return false;
	}

	FbxIOSettings* ios = FbxIOSettings::Create( m_fbxManager, IOSROOT );
	m_fbxManager->SetIOSettings( ios );

	(*(m_fbxManager->GetIOSettings())).SetBoolProp( IMP_FBX_MATERIAL, true );
	(*(m_fbxManager->GetIOSettings())).SetBoolProp( IMP_FBX_TEXTURE, true );
	(*(m_fbxManager->GetIOSettings())).SetBoolProp( IMP_FBX_LINK, false );
	(*(m_fbxManager->GetIOSettings())).SetBoolProp( IMP_FBX_SHAPE, false );
	(*(m_fbxManager->GetIOSettings())).SetBoolProp( IMP_FBX_GOBO, false );
	(*(m_fbxManager->GetIOSettings())).SetBoolProp( IMP_FBX_ANIMATION, true );
	(*(m_fbxManager->GetIOSettings())).SetBoolProp( IMP_FBX_GLOBAL_SETTINGS, true );

	bool lEmbedMedia = true;
	(*(m_fbxManager->GetIOSettings())).SetBoolProp( EXP_FBX_MATERIAL, true );
	(*(m_fbxManager->GetIOSettings())).SetBoolProp( EXP_FBX_TEXTURE, true );
	(*(m_fbxManager->GetIOSettings())).SetBoolProp( EXP_FBX_EMBEDDED, lEmbedMedia );
	(*(m_fbxManager->GetIOSettings())).SetBoolProp( EXP_FBX_SHAPE, true );
	(*(m_fbxManager->GetIOSettings())).SetBoolProp( EXP_FBX_GOBO, true );
	(*(m_fbxManager->GetIOSettings())).SetBoolProp( EXP_FBX_ANIMATION, true );
	(*(m_fbxManager->GetIOSettings())).SetBoolProp( EXP_FBX_GLOBAL_SETTINGS, true );

	FbxString lExtension = "dll";
	FbxString lPath = FbxGetApplicationDirectory();
	m_fbxManager->LoadPluginsDirectory( lPath.Buffer(), lExtension.Buffer() );

	m_fbxScene = FbxScene::Create( m_fbxManager, "" );

	return true;
}

bool FBX::LoadScene( char const* pSceneName )
{
	UNUSED( pSceneName );
	if (m_fbxManager == NULL)
	{
		return false;
	}

// 	int major, minor, revision;
// 	FbxManager::GetFileFormatVersion( major, minor, revision );

	FbxImporter* fbxImporter = FbxImporter::Create( m_fbxManager, "" );

	bool importStatus = fbxImporter->Initialize( m_filePath.c_str(), -1, m_fbxManager->GetIOSettings() );

	if (!importStatus)
		return false;

	m_fbxScene->Clear();
	importStatus = fbxImporter->Import( m_fbxScene );

	fbxImporter->Destroy();

	return importStatus;
}

void FBX::ProcessNode(FbxNode* pNode)
{
	//FbxNodeAttribute::EType attributeType;

	if (pNode->GetNodeAttribute())
	{
		switch (pNode->GetNodeAttribute()->GetAttributeType())
		{
		case FbxNodeAttribute::eMesh:
			ProcessMesh( pNode );
			break;
		case FbxNodeAttribute::eSkeleton:
			break;
		case FbxNodeAttribute::eLight:
			break;
		case FbxNodeAttribute::eCamera:
			break;
		default:
			break;
		}
	}

	for (int i = 0; i < pNode->GetChildCount(); i++)
	{
		ProcessNode( pNode->GetChild( i ) );
	}
}

void FBX::ProcessMesh( FbxNode* pNode )
{
	FbxMesh* mesh = pNode->GetMesh();
	if (mesh == NULL)
		return;

	//D3DXVECTOR3 vertex[3];
	//D3DXVECTOR4 color[3];
	//D3DXVECTOR3 normal[3];
	//D3DXVECTOR3 tangent[3];
	//D3DXVECTOR2 uv[3][2];

	int triangleCount = mesh->GetPolygonCount();
	int vertexCounter = 0;
	
	for (int i = 0; i < triangleCount; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			int ctrlPointVertex = mesh->GetPolygonVertex( i, j );
			Vec3 position;
			ReadVertex( mesh, ctrlPointVertex, position );

			Rgba8 color;
			ReadColor( mesh, ctrlPointVertex, vertexCounter, color );

			Vec2 uv;
			ReadUVs( mesh, ctrlPointVertex, mesh->GetTextureUVIndex( i, j ), 0, uv );

// 			Vec2 uvLightMap;
// 			ReadUVs( mesh, ctrlPointVertex, mesh->GetTextureUVIndex( i, j ), 1, uvLightMap );

			Vec3 tangent;
			ReadTangent( mesh, ctrlPointVertex, vertexCounter, tangent );

			Vec3 binormal;
			ReadBinormal( mesh, ctrlPointVertex, vertexCounter, binormal );

			Vec3 normal;
			ReadNormal( mesh, ctrlPointVertex, vertexCounter, normal );

			Vertex_PCUTBN vertex( position, color, uv, tangent, binormal, normal );

			m_verts.push_back( vertex );

 			vertexCounter++;
		}
	}
	if (m_verts.size() > 0)
	{
		if (m_vertexBuffer != nullptr)
		{
			delete m_vertexBuffer;
			m_vertexBuffer = nullptr;
		}
		m_vertexBuffer = m_renderer->CreateVertexBuffer( m_verts.size() * sizeof( Vertex_PCUTBN ) );
	}
}

void FBX::ReadVertex( FbxMesh* pMesh, int ctrlPointIndex, Vec3& position )
{
	FbxVector4* vertexArray = pMesh->GetControlPoints();
	position.x = (float)vertexArray[ctrlPointIndex][0];
	position.y = (float)vertexArray[ctrlPointIndex][1];
	position.z = (float)vertexArray[ctrlPointIndex][2];
}

void FBX::ReadColor( FbxMesh* pMesh, int ctrlPointIndex, int vertexCount, Rgba8& color )
{
	if (pMesh->GetElementVertexColorCount() < 1)
		return;

	FbxGeometryElementVertexColor* vertexColor = pMesh->GetElementVertexColor( 0 );

	switch (vertexColor->GetMappingMode())
	{
	case FbxGeometryElement::eByControlPoint:
	{
		switch (vertexColor->GetReferenceMode())
		{
		case FbxGeometryElement::eDirect:
		{
			color.r = (unsigned char)(255.f * vertexColor->GetDirectArray().GetAt( ctrlPointIndex ).mRed);
			color.g = (unsigned char)(255.f * vertexColor->GetDirectArray().GetAt( ctrlPointIndex ).mGreen);
			color.b = (unsigned char)(255.f * vertexColor->GetDirectArray().GetAt( ctrlPointIndex ).mBlue);
			color.a = (unsigned char)(255.f * vertexColor->GetDirectArray().GetAt( ctrlPointIndex ).mAlpha);
			break;
		}
		case FbxGeometryElement::eIndexToDirect:
		{
			int index = vertexColor->GetIndexArray().GetAt( ctrlPointIndex );
			color.r = (unsigned char)(255.f * vertexColor->GetDirectArray().GetAt( index ).mRed);
			color.g = (unsigned char)(255.f * vertexColor->GetDirectArray().GetAt( index ).mGreen);
			color.b = (unsigned char)(255.f * vertexColor->GetDirectArray().GetAt( index ).mBlue);
			color.a = (unsigned char)(255.f * vertexColor->GetDirectArray().GetAt( index ).mAlpha);
			break;
		}
		default:
			break;
		}
		break;
	}
	case FbxGeometryElement::eByPolygonVertex:
	{
		switch (vertexColor->GetReferenceMode())
		{
		case FbxGeometryElement::eDirect:
		{
			color.r = (unsigned char)(255.f * vertexColor->GetDirectArray().GetAt( vertexCount ).mRed);
			color.g = (unsigned char)(255.f * vertexColor->GetDirectArray().GetAt( vertexCount ).mGreen);
			color.b = (unsigned char)(255.f * vertexColor->GetDirectArray().GetAt( vertexCount ).mBlue);
			color.a = (unsigned char)(255.f * vertexColor->GetDirectArray().GetAt( vertexCount ).mAlpha);
			break;
		}
		case FbxGeometryElement::eIndexToDirect:
		{
			int index = vertexColor->GetIndexArray().GetAt( vertexCount );
			color.r = (unsigned char)(255.f * vertexColor->GetDirectArray().GetAt( index ).mRed);
			color.g = (unsigned char)(255.f * vertexColor->GetDirectArray().GetAt( index ).mGreen);
			color.b = (unsigned char)(255.f * vertexColor->GetDirectArray().GetAt( index ).mBlue);
			color.a = (unsigned char)(255.f * vertexColor->GetDirectArray().GetAt( index ).mAlpha);
			break;
		}
		default:
			break;
		}
		break;
	}
	}
}

void FBX::ReadUVs( FbxMesh* pMesh, int ctrlPointIndex, int textureUVIndex, int UVLayer, Vec2& UVs )
{
	if (UVLayer >= 2 || pMesh->GetElementUVCount() <= UVLayer)
		return;

	FbxGeometryElementUV* uv = pMesh->GetElementUV( UVLayer );

	switch (uv->GetMappingMode())
	{
	case FbxGeometryElement::eByControlPoint:
	{
		switch (uv->GetReferenceMode())
		{
		case FbxGeometryElement::eDirect:
		{
			UVs.x = (float)uv->GetDirectArray().GetAt( ctrlPointIndex )[0];
			UVs.y = (float)uv->GetDirectArray().GetAt( ctrlPointIndex )[1];
			break;
		}
		case FbxGeometryElement::eIndexToDirect:
		{
			
			int index = uv->GetIndexArray().GetAt( ctrlPointIndex );
			UVs.x = (float)uv->GetDirectArray().GetAt( index )[0];
			UVs.y = (float)uv->GetDirectArray().GetAt( index )[1];
			break;
		}
		default:
			break;
		}
		break;
	}
	case FbxGeometryElement::eByPolygonVertex:
	{
		switch (uv->GetReferenceMode())
		{
		case FbxGeometryElement::eDirect:
			break;

		case FbxGeometryElement::eIndexToDirect:
		{
			UVs.x = (float)uv->GetDirectArray().GetAt( textureUVIndex )[0];
			UVs.y = (float)uv->GetDirectArray().GetAt( textureUVIndex )[1];
			break;
		}
		default:
			break;
		}
		break;
	}
	}
}

void FBX::ReadTangent( FbxMesh* pMesh, int ctrlPointIndex, int vertexCount, Vec3& tangent )
{
	if (pMesh->GetElementTangentCount() < 1)
	{
		return;
	}
	FbxGeometryElementTangent* vertexTangent = pMesh->GetElementTangent( 0 );

	switch (vertexTangent->GetMappingMode())
	{
	case FbxGeometryElement::eByControlPoint:
	{
		switch (vertexTangent->GetReferenceMode())
		{
		case FbxGeometryElement::eDirect:
		{
			tangent.x = (float)vertexTangent->GetDirectArray().GetAt( ctrlPointIndex )[0];
			tangent.y = (float)vertexTangent->GetDirectArray().GetAt( ctrlPointIndex )[1];
			tangent.z = (float)vertexTangent->GetDirectArray().GetAt( ctrlPointIndex )[2];
			break;
		}
		case FbxGeometryElement::eIndexToDirect:
		{
			int index = vertexTangent->GetIndexArray().GetAt( ctrlPointIndex );
			tangent.x = (float)vertexTangent->GetDirectArray().GetAt( index )[0];
			tangent.y = (float)vertexTangent->GetDirectArray().GetAt( index )[1];
			tangent.z = (float)vertexTangent->GetDirectArray().GetAt( index )[2];
			break;
		}
		default:
			break;
		}
		break;
	}
	case FbxGeometryElement::eByPolygonVertex:
	{
		switch (vertexTangent->GetReferenceMode())
		{
		case FbxGeometryElement::eDirect:
		{
			tangent.x = (float)vertexTangent->GetDirectArray().GetAt( vertexCount )[0];
			tangent.y = (float)vertexTangent->GetDirectArray().GetAt( vertexCount )[1];
			tangent.z = (float)vertexTangent->GetDirectArray().GetAt( vertexCount )[2];
			break;
		}
		case FbxGeometryElement::eIndexToDirect:
		{
			int index = vertexTangent->GetIndexArray().GetAt( vertexCount );
			tangent.x = (float)vertexTangent->GetDirectArray().GetAt( index )[0];
			tangent.y = (float)vertexTangent->GetDirectArray().GetAt( index )[1];
			tangent.z = (float)vertexTangent->GetDirectArray().GetAt( index )[2];
			break;
		}
		default:
			break;
		}
		break;
	}
	default:
		break;
	}
}

void FBX::ReadBinormal( FbxMesh* pMesh, int ctrlPointIndex, int vertexCount, Vec3& binormal )
{
	if (pMesh->GetElementBinormalCount() < 1)
	{
		return;
	}
	FbxGeometryElementBinormal* vertexBinormal = pMesh->GetElementBinormal( 0 );

	switch (vertexBinormal->GetMappingMode())
	{
	case FbxGeometryElement::eByControlPoint:
	{
		switch (vertexBinormal->GetReferenceMode())
		{
		case FbxGeometryElement::eDirect:
		{
			binormal.x = (float)vertexBinormal->GetDirectArray().GetAt( ctrlPointIndex )[0];
			binormal.y = (float)vertexBinormal->GetDirectArray().GetAt( ctrlPointIndex )[1];
			binormal.z = (float)vertexBinormal->GetDirectArray().GetAt( ctrlPointIndex )[2];
			break;
		}
		case FbxGeometryElement::eIndexToDirect:
		{
			int index = vertexBinormal->GetIndexArray().GetAt( ctrlPointIndex );
			binormal.x = (float)vertexBinormal->GetDirectArray().GetAt( index )[0];
			binormal.y = (float)vertexBinormal->GetDirectArray().GetAt( index )[1];
			binormal.z = (float)vertexBinormal->GetDirectArray().GetAt( index )[2];
			break;
		}
		default:
			break;
		}
		break;
	}
	case FbxGeometryElement::eByPolygonVertex:
	{
		switch (vertexBinormal->GetReferenceMode())
		{
		case FbxGeometryElement::eDirect:
		{
			binormal.x = (float)vertexBinormal->GetDirectArray().GetAt( vertexCount )[0];
			binormal.y = (float)vertexBinormal->GetDirectArray().GetAt( vertexCount )[1];
			binormal.z = (float)vertexBinormal->GetDirectArray().GetAt( vertexCount )[2];
			break;
		}
		case FbxGeometryElement::eIndexToDirect:
		{
			int index = vertexBinormal->GetIndexArray().GetAt( vertexCount );
			binormal.x = (float)vertexBinormal->GetDirectArray().GetAt( index )[0];
			binormal.y = (float)vertexBinormal->GetDirectArray().GetAt( index )[1];
			binormal.z = (float)vertexBinormal->GetDirectArray().GetAt( index )[2];
			break;
		}
		default:
			break;
		}
		break;
	}
	default:
		break;
	}
}


void FBX::ReadNormal( FbxMesh* pMesh, int ctrlPointIndex, int vertexCount, Vec3& normal )
{
	if (pMesh->GetElementNormalCount() < 1)
	{
		return;
	}
	FbxGeometryElementNormal* vertexNormal = pMesh->GetElementNormal( 0 );

	switch (vertexNormal->GetMappingMode())
	{
	case FbxGeometryElement::eByControlPoint:
	{
		switch (vertexNormal->GetReferenceMode())
		{
		case FbxGeometryElement::eDirect:
		{
			normal.x = (float)vertexNormal->GetDirectArray().GetAt( ctrlPointIndex )[0];
			normal.y = (float)vertexNormal->GetDirectArray().GetAt( ctrlPointIndex )[1];
			normal.z = (float)vertexNormal->GetDirectArray().GetAt( ctrlPointIndex )[2];
			break;
		}
		case FbxGeometryElement::eIndexToDirect:
		{
			int index = vertexNormal->GetIndexArray().GetAt( ctrlPointIndex );
			normal.x = (float)vertexNormal->GetDirectArray().GetAt( index )[0];
			normal.y = (float)vertexNormal->GetDirectArray().GetAt( index )[1];
			normal.z = (float)vertexNormal->GetDirectArray().GetAt( index )[2];
			break;
		}
		default:
			break;
		}
		break;
	}
	case FbxGeometryElement::eByPolygonVertex:
	{
		switch (vertexNormal->GetReferenceMode())
		{
		case FbxGeometryElement::eDirect:
		{
			normal.x = (float)vertexNormal->GetDirectArray().GetAt( vertexCount )[0];
			normal.y = (float)vertexNormal->GetDirectArray().GetAt( vertexCount )[1];
			normal.z = (float)vertexNormal->GetDirectArray().GetAt( vertexCount )[2];
			break;
		}
		case FbxGeometryElement::eIndexToDirect:
		{
			int index = vertexNormal->GetIndexArray().GetAt( vertexCount );
			normal.x = (float)vertexNormal->GetDirectArray().GetAt( index )[0];
			normal.y = (float)vertexNormal->GetDirectArray().GetAt( index )[1];
			normal.z = (float)vertexNormal->GetDirectArray().GetAt( index )[2];
			break;
		}
		default:
			break;
		}
		break;
	}
	default:
		break;
	}
}
