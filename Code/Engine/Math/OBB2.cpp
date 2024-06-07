#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "OBB2.hpp"

OBB2::OBB2( OBB2 const& copyForm )
{
	m_center = copyForm.m_center;
	m_iBasisNormal = copyForm.m_iBasisNormal;
	m_halfDimensions = copyForm.m_halfDimensions;
}

OBB2::OBB2( Vec2 const& center, Vec2 const& iBasisNormal, Vec2 const& halfDimensions )
{
	m_center = center;
	m_iBasisNormal = iBasisNormal;
	m_halfDimensions = halfDimensions;
}

void OBB2::Translate( Vec2 translation )
{
	m_halfDimensions = m_halfDimensions * translation;
}

void OBB2::SetCenter( Vec2 newCenter )
{
	m_center = newCenter;
}

void OBB2::RotateAboutCenter( float rotationDeltaDegree )
{
	float currDegree = Atan2Degrees( m_iBasisNormal.y, m_iBasisNormal.x );
	float newDegree = currDegree + rotationDeltaDegree;
	m_iBasisNormal = Vec2( CosDegrees( newDegree ), SinDegrees( newDegree ) );
}
