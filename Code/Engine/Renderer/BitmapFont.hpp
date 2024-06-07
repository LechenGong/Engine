#pragma once

#include <string>

#include "Engine/Renderer/Sprite.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/Vec2.hpp"

enum TextBoxMode
{
	SHRINK,
	OVERRUN
};

class BitmapFont
{
	friend class Renderer; // Only the Renderer can create new BitmapFont objects!

private:
	BitmapFont( char const* fontFilePathNameWithNoExtension, Texture& fontTexture );

public:
	Texture& GetTexture();

	void AddVertsForText2D( std::vector<Vertex_PCU>& vertexArray, Vec2 const& textMins,
		float cellHeight, std::string const& text, Rgba8 const& tint = Rgba8::WHITE, float cellAspect = 0.7f );

	void AddVertsForTextInBox2D( std::vector<Vertex_PCU>& vertexArray, AABB2 const& box, float cellHeight,
		std::string const& text, Rgba8 const& tint = Rgba8::WHITE, float cellAspect = 0.7f,
		Vec2 const& alignment = Vec2( .5f, .5f ), TextBoxMode mode = TextBoxMode::SHRINK, int maxGlyphsToDraw = 99999999 );

	void AddvertsForText3DAtOriginXForward( std::vector<Vertex_PCU>& vertexArray, float cellHeight,
		std::string const& text, Rgba8 const& tint = Rgba8::WHITE, float cellAspect = 0.7f,
		Vec2 const& alignment = Vec2( .5f, .5f ), int maxGlyphsToDraw = 99999999 );

	float GetTextWidth( float cellHeight, std::string const& text, float cellAspect = 0.7f );
	std::string const& GetImageFilePath() const { return m_fontFilePathNameWithNoExtension; }

protected:
	float GetGlyphAspect( int glyphUnicode ) const; // For now this will always return 1.0f!!!

protected:
	std::string	m_fontFilePathNameWithNoExtension;
	SpriteSheet m_fontGlyphsSpriteSheet;
};
