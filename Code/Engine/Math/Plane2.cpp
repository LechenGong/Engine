#include "Engine/Math/Plane2.hpp"
#include "Engine/Math/MathUtils.hpp"

Plane2::Plane2( Plane2 const& copyForm )
{
	m_normal = copyForm.m_normal;
	m_distanceFromOrigin = copyForm.m_distanceFromOrigin;
}

Plane2::Plane2( Vec2 const& normal, float distanceFromOrigin )
{
	m_normal = normal;
	m_distanceFromOrigin = distanceFromOrigin;
}

float Plane2::GetDistanceOfPoint( Vec2 const& point ) const
{
	return DotProduct2D( point, m_normal ) - m_distanceFromOrigin;
}

bool Plane2::IsPointInFrontOfPlane( Vec2 const& point ) const
{
	return DotProduct2D( point, m_normal ) > m_distanceFromOrigin;
}

bool Plane2::DoPlane2Intersect( Plane2 const& planeA, Plane2 const& planeB )
{
	float det = planeA.m_normal.x * planeB.m_normal.y - planeA.m_normal.y * planeB.m_normal.x;
	return std::abs( det ) > 1e-6f;
}

Vec2 Plane2::GetIntersectPos( Plane2 const& planeA, Plane2 const& planeB )
{
	float det = planeA.m_normal.x * planeB.m_normal.y - planeA.m_normal.y * planeB.m_normal.x;
	float x = (planeA.m_distanceFromOrigin * planeB.m_normal.y - planeB.m_distanceFromOrigin * planeA.m_normal.y) / det;
	float y = (planeA.m_normal.x * planeB.m_distanceFromOrigin - planeB.m_normal.x * planeA.m_distanceFromOrigin) / det;
	return Vec2( x, y );
}
