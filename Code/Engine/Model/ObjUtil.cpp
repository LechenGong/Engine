#include <fstream>
#include <sstream>

#include "Engine/Model/ObjUtil.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/VertexBuffer.hpp"
#include "Engine/Renderer/IndexBuffer.hpp"

OBJ::OBJ( std::string objPath, Renderer* renderer )
{
	m_renderer = renderer;
	m_rootPath = objPath.substr( 0, objPath.find_last_of( "/" ) );
	LoadRawFile( objPath );
	AddVerts();
}

OBJ::~OBJ()
{
	for (int i = 0; i <= parts; i++)
	{
		delete m_vertexBuffers[i];
		delete m_indexBuffers[i];
	}
}

void OBJ::Render() const
{
	for (int i = 0; i <= parts; i++)
	{
		auto iter = m_texturePaths.find( m_textureNames[i] );
		if ( iter != m_texturePaths.end())
		{
			m_renderer->BindTexture( m_renderer->CreateOrGetTextureFromFile( iter->second.c_str() ) );
		}
		m_renderer->DrawVertexArray( m_vertexBuffers[i], m_indexBuffers[i], m_vertexes[i], m_indexes[i] );
	}
}

void OBJ::LoadRawFile( std::string objPath )
{
	std::ifstream objFile;
	objFile.exceptions( std::fstream::failbit || std::fstream::badbit );
	try
	{
		objFile.open( objPath );
		std::stringstream objStream;
		objStream << objFile.rdbuf();
		objFile.close();
		m_rawOBJ = objStream.str();
	}
	catch (std::ifstream::failure& e)
	{
		//ERROR_RECOVERABLE( Stringf( "Load Model %s Fail", objPath ) );
		UNUSED( e );
	}

	int mtlPathStart = (int)m_rawOBJ.find( "mtllib " );
	int mtlPathEnd = (int)m_rawOBJ.find( "\n", mtlPathStart );
	std::string mtlPath = m_rootPath + '/' + Split( m_rawOBJ.substr( mtlPathStart, mtlPathEnd - mtlPathStart ), ' ' )[1];
	std::ifstream mtlFile;
	mtlFile.exceptions( std::ifstream::failbit | std::ifstream::badbit );
	try
	{
		mtlFile.open( mtlPath );
		std::stringstream mtlStream;
		mtlStream << mtlFile.rdbuf();
		mtlFile.close();
		m_rawMTL = mtlStream.str();
	}
	catch (std::ifstream::failure& e)
	{
		//ERROR_RECOVERABLE( Stringf( "Load Model %s Fail", mtlPath ) );
		UNUSED( e );
	}

	std::ifstream checklistFile;
	std::string checklistPath = m_rootPath + '/' + "Checklist.txt";
	checklistFile.exceptions( std::fstream::failbit || std::fstream::badbit );
	try
	{
		checklistFile.open( checklistPath );
		std::stringstream checklistStream;
		checklistStream << checklistFile.rdbuf();
		checklistFile.close();
		m_rawChecklist = checklistStream.str();
	}
	catch (std::ifstream::failure& e)
	{
		//ERROR_RECOVERABLE( Stringf( "Load Model %s Fail", objPath ) );
		UNUSED( e );
	}
}

void OBJ::AddVerts()
{
	std::vector<Vec3> positions;
	std::vector<Vec2> uvs;
	std::vector<Vec3> normals;

	m_vertexes.resize( 1 );
	m_indexes.resize( 1 );
	m_vertexBuffers.resize( 1 );
	m_indexBuffers.resize( 1 );

	Strings lines = Split( m_rawOBJ, '\n' );
	for (int i = 0; i < lines.size(); i++)
	{
		Strings contents = Split( lines[i], ' ', true, 1 );
		if (contents.size() != 2)
			continue;
		// Assume always valid data?

		if (contents[0] == "v")
		{
			positions.push_back( Vec3( contents[1] ) );
		}
		else if (contents[0] == "vt")
		{
			uvs.push_back( Vec2( contents[1] ) );
		}
		else if (contents[0] == "vn")
		{
			normals.push_back( Vec3( contents[1] ) );
		}
		else if (contents[0] == "f")
		{
			Strings vertsString = Split( contents[1], ' ' );
			std::vector<Vertex_PCUTBN> verts;
			vertsString.reserve( vertsString.size() );
			for (int j = 0; j < vertsString.size(); j++)
			{
				Strings data = Split( vertsString[j], '/' );
				verts.push_back( Vertex_PCUTBN{
					positions[std::stoi( data[0] ) - 1] * 10.f,
					Rgba8::WHITE,
					uvs[std::stoi( data[1] ) - 1],
					Vec3::ZERO,
					Vec3::ZERO,
					normals[std::stoi( data[2] ) - 1]
					} );
			}
			if (verts.size() == 3)
			{
				int vertsCount = (int)m_vertexes[parts].size();
				m_vertexes[parts].push_back( verts[0] );
				m_vertexes[parts].push_back( verts[1] );
				m_vertexes[parts].push_back( verts[2] );
				m_indexes[parts].push_back( vertsCount + 0 );
				m_indexes[parts].push_back( vertsCount + 1 );
				m_indexes[parts].push_back( vertsCount + 2 );
			}
			else if (verts.size() == 4)
			{
				int vertsCount = (int)m_vertexes[parts].size();
				m_vertexes[parts].push_back( verts[0] );
				m_vertexes[parts].push_back( verts[1] );
				m_vertexes[parts].push_back( verts[2] );
				m_vertexes[parts].push_back( verts[3] );
				m_indexes[parts].push_back( vertsCount + 0 );
				m_indexes[parts].push_back( vertsCount + 1 );
				m_indexes[parts].push_back( vertsCount + 2 );
				m_indexes[parts].push_back( vertsCount + 2 );
				m_indexes[parts].push_back( vertsCount + 3 );
				m_indexes[parts].push_back( vertsCount + 0 );
			}
		}
		else if (contents[0] == "usemtl")
		{
			if (parts >= 0)
			{
				m_vertexBuffers.push_back( nullptr );
				m_indexBuffers.push_back( nullptr );
				m_vertexBuffers[parts] = m_renderer->CreateVertexBuffer( m_vertexes[parts].size() * sizeof( Vertex_PCUTBN ) );
				m_indexBuffers[parts] = m_renderer->CreateIndexBuffer( m_indexes[parts].size() * sizeof( unsigned int ) );
				Mat44 mat = EulerAngles{ 90.f, 0.f, 90.f }.GetAsMatrix_IFwd_JLeft_KUp();
				TransformVertexArray3D( m_vertexes[parts], mat );
			}
			parts++;
			m_vertexes.push_back( std::vector<Vertex_PCUTBN>() );
			m_indexes.push_back( std::vector<unsigned int>() );
			m_textureNames.push_back( contents[1] );
		}
	}
	m_vertexBuffers.push_back( nullptr );
	m_indexBuffers.push_back( nullptr );
	m_vertexBuffers[parts] = m_renderer->CreateVertexBuffer( m_vertexes[parts].size() * sizeof( Vertex_PCUTBN ) );
	m_indexBuffers[parts] = m_renderer->CreateIndexBuffer( m_indexes[parts].size() * sizeof( unsigned int ) );
	Mat44 mat = EulerAngles{ 90.f, 0.f, 90.f }.GetAsMatrix_IFwd_JLeft_KUp();
	TransformVertexArray3D( m_vertexes[parts], mat );

	Strings checklist = Split( m_rawChecklist, '\n' );
	for (int i = 0; i < checklist.size(); i++)
	{
		Strings paths = Split( checklist[i], ' ' );
		m_texturePaths[paths[0]] = paths.size() > 1 ? m_rootPath + '/' + paths[1] : "";
	}
}
