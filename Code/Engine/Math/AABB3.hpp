#pragma once

#include <cmath>

#include "Engine/Math/Vec3.hpp"

struct AABB3
{
public:
	Vec3 m_mins;
	Vec3 m_maxs;

public:
	//Construction & Deconstruction
	AABB3() {}
	~AABB3() {}
	AABB3( AABB3 const& copyForm );
	explicit AABB3( float minX, float minY, float minZ, float maxX, float maxY, float maxZ );
	explicit AABB3( Vec3 const& mins, Vec3 const& maxs );

	AABB3 const GetMoved( Vec3 const& delta ) const;

	// Accessors (const methods)
// 	bool IsPointInside( Vec2 const& point ) const;
// 	Vec2 const GetCenter() const;
// 	Vec2 const GetDimensions() const;
// 	Vec2 const GetNearestPoint( Vec2 const& refernencePosition ) const;
// 	Vec2 const GetPointAtUV( Vec3 const& uv ) const;
// 	Vec2 const GetUVForPoint( Vec3 const& point ) const;
	//Mutator (non-const methods)
// 	void Translate( Vec2 const& trasnlationToApply );
// 	void SetCenter( Vec2 const& newCenter );
// 	void SetDimensions( Vec2 const& newDimensions );

	void StretchToIncludePoint( Vec2 const& point );

	static const AABB3 ZERO_TO_ONE;
};
