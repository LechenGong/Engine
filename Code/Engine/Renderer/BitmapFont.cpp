#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Renderer/Sprite.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/MathUtils.hpp"

BitmapFont::BitmapFont( char const* fontFilePathNameWithNoExtension, Texture& fontTexture )
	: m_fontGlyphsSpriteSheet( fontTexture, IntVec2( 16, 16 ) )
	, m_fontFilePathNameWithNoExtension( fontFilePathNameWithNoExtension )
{
}

Texture& BitmapFont::GetTexture()
{
	return m_fontGlyphsSpriteSheet.GetTexture();
}

void BitmapFont::AddVertsForText2D( std::vector<Vertex_PCU>& vertexArray, Vec2 const& textMins, float cellHeight, std::string const& text, Rgba8 const& tint, float cellAspect )
{
	vertexArray.reserve( text.length() * 6 );
	float cellWidth = cellHeight * cellAspect;
	for (int stringIndex = 0; stringIndex < text.length(); stringIndex++)
	{
		unsigned char character = text[stringIndex];
		AABB2 uv = m_fontGlyphsSpriteSheet.GetSpriteUVs( character );
		AABB2 textPos( textMins + Vec2( stringIndex * cellWidth, 0 ), textMins + Vec2( (stringIndex + 1) * cellWidth, cellHeight ) );
		AddVertsForAABB2D( vertexArray, textPos, tint, uv.m_mins, uv.m_maxs );
	}
}

void BitmapFont::AddVertsForTextInBox2D( std::vector<Vertex_PCU>& vertexArray, AABB2 const& box, float cellHeight, std::string const& text, Rgba8 const& tint, float cellAspect, Vec2 const& alignment, TextBoxMode mode, int maxGlyphsToDraw )
{
	Strings texts = Split( text, '\n', true );

	if (text.size() <= 0)
		return;
	
	float cellWidth = cellHeight * cellAspect;
	float theoryHeight = cellHeight * texts.size();
	float theoryWidth = 0.f;
	for (std::string textIndex : texts)
	{
		theoryWidth = MAX( theoryWidth, textIndex.length() * cellWidth );
	}

	Vec2 shrinkRatio = Vec2::ONE;
	Vec2 boxSize = box.GetDimensions();
	if (mode == TextBoxMode::SHRINK)
	{
		if (theoryWidth > boxSize.x)
		{
			
			float shrinkX = boxSize.x / theoryWidth;
			theoryWidth = boxSize.x;
			theoryHeight *= shrinkX;
			shrinkRatio *= shrinkX;

		}
		if (theoryHeight > boxSize.y)
		{
			float shrinkY = boxSize.y / theoryHeight;
			theoryHeight = boxSize.y;
			theoryWidth *= shrinkY;
			shrinkRatio *= shrinkY;
		}
	}

	float newCellWidth = cellWidth * shrinkRatio.x;
	float newCellHeight = cellHeight * shrinkRatio.y;

	for (int textIndex = 0; textIndex < texts.size(); ++textIndex)
	{
		if (maxGlyphsToDraw <= 0)
			return;

		if (texts[textIndex].length() > maxGlyphsToDraw)
		{
			texts[textIndex] = texts[textIndex].substr( 0, maxGlyphsToDraw );
		}

		float textWidth = newCellWidth * texts[textIndex].length();
		Vec2 localMins = Vec2( alignment.x * (boxSize.x - textWidth), alignment.y * (boxSize.y - theoryHeight) + newCellHeight * (texts.size() - 1 - textIndex) );
		Vec2 textMins = box.m_mins + localMins;
		AddVertsForText2D( vertexArray, textMins, newCellHeight, texts[textIndex], tint, cellAspect );
		maxGlyphsToDraw -= static_cast<int>(texts[textIndex].length());
	}
}

void BitmapFont::AddvertsForText3DAtOriginXForward( std::vector<Vertex_PCU>& vertexArray, float cellHeight, std::string const& text, Rgba8 const& tint, float cellAspect, Vec2 const& alignment, int maxGlyphsToDraw )
{
	Strings texts = Split( text, '\n', true );

	if (text.size() <= 0)
		return;

	float cellWidth = cellHeight * cellAspect;
	float theoryHeight = cellHeight * texts.size();
	float theoryWidth = 0.f;
	int textCount = 0;
	for (std::string textIndex : texts)
	{
		theoryWidth = MAX( theoryWidth, textIndex.length() * cellWidth );
		textCount += (int)textIndex.length();
	}
	AABB2 box( Vec2( (alignment.x - 1.f) * theoryWidth, (alignment.y - 1.f) * theoryHeight ), Vec2( alignment.x * theoryWidth, alignment.y * theoryHeight ) );
	Vec2 boxSize = box.GetDimensions();
	std::vector<Vertex_PCU> ZVerts;
	for (int textIndex = 0; textIndex < texts.size(); ++textIndex)
	{
		if (maxGlyphsToDraw <= 0)
			return;

		if (texts[textIndex].length() > maxGlyphsToDraw)
		{
			texts[textIndex] = texts[textIndex].substr( 0, maxGlyphsToDraw );
		}
		float textWidth = cellWidth * texts[textIndex].length();
		Vec2 localMins = Vec2( alignment.x * (boxSize.x - textWidth), alignment.y * (boxSize.y - theoryHeight) + cellHeight * (texts.size() - 1 - textIndex) );
		Vec2 textMins = box.m_mins + localMins;
		AddVertsForText2D( ZVerts, textMins, cellHeight, texts[textIndex], tint, cellAspect );
		maxGlyphsToDraw -= static_cast<int>(texts[textIndex].length());
	}
	Mat44 mat = Mat44::CreateZRotationDegrees( 90.f );
	mat.Append( Mat44::CreateXRotationDegrees( 90.f ) );
	TransformVertexArray3D( ZVerts, mat );
	for (Vertex_PCU vert : ZVerts)
	{
		vertexArray.push_back( vert );
	}
}

float BitmapFont::GetTextWidth( float cellHeight, std::string const& text, float cellAspect )
{
	return cellHeight * cellAspect * text.length();
}

float BitmapFont::GetGlyphAspect( int glyphUnicode ) const
{
	UNUSED( glyphUnicode );
	return 1.0f;
}
