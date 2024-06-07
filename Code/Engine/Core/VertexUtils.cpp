#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/EngineCommon.hpp"

#include <vector>

void TransformVertexArrayXY3D( int numVerts, Vertex_PCU* verts, float uniformScaleXY,
	float rotationDegreesAboutZ, Vec2 const& translationXY )
{

	for (int vertIndex = 0; vertIndex < numVerts; vertIndex++)
	{
		TransformPositionXY3D( verts[vertIndex].m_position, uniformScaleXY, rotationDegreesAboutZ, translationXY );
	}
}

void TransformVertexArrayXY3D( std::vector<Vertex_PCU> &verts, float uniformScaleXY,
	float rotationDegreesAboutZ, Vec2 const& translationXY )
{
	for (int vertIndex = 0; vertIndex < verts.size(); vertIndex++)
	{
		TransformPositionXY3D( verts[vertIndex].m_position, uniformScaleXY, rotationDegreesAboutZ, translationXY );
	}
}

void TransformVertexArray3D( std::vector<Vertex_PCU>& verts, Mat44 const& transform )
{
	for (Vertex_PCU& vert : verts)
	{
		vert.m_position = transform.TransformPosition3D( vert.m_position );
	}
}

void TransformVertexArray3D( std::vector<Vertex_PCUTBN>& verts, Mat44 const& transform )
{
	for (Vertex_PCUTBN& vert : verts)
	{
		vert.m_position = transform.TransformPosition3D( vert.m_position );
	}
}


Vertex_PCU GetTransformedVertex3D( Vertex_PCU vert, Mat44 const& transform )
{
	Vertex_PCU newVert = vert;
	newVert.m_position = transform.TransformPosition3D( vert.m_position );
	return newVert;
}

Vertex_PCU RotateVertexAboutPivot( Vertex_PCU vert, Vec3 const& pivot, Mat44 const& transform )
{
	Vec3 pivotToVert = vert.m_position - pivot;
	Vec3 pivotToNewVert = transform.TransformVectorQuantity3D( pivotToVert );
	Vertex_PCU newVert = vert;
	newVert.m_position = pivot + pivotToNewVert;
	return newVert;
}


Vec2 TransformPolarToCarteDegreeNormal( float rotationDegree )
{
	return Vec2(
		CosDegrees( rotationDegree ),
		SinDegrees( rotationDegree )
	);
}

void TransformVertexArrayXY3D( int numVerts, Vertex_PCU* verts, Vec2 const& iBasis, Vec2 const& jBasis, Vec2 const& translationXY )
{

	for (int vertIndex = 0; vertIndex < numVerts; vertIndex++)
	{
		if (verts[vertIndex].m_position.x == 0 && verts[vertIndex].m_position.y == 0)
			continue;

		TransformPositionXY3D( verts[vertIndex].m_position, iBasis, jBasis, translationXY );
	}
}

void TransformVertexArrayXY3D( std::vector<Vertex_PCU>& verts, Vec2 const& iBasis, Vec2 const& jBasis, Vec2 const& translationXY )
{
	for (int vertIndex = 0; vertIndex < verts.size(); vertIndex++)
	{
		if (verts[vertIndex].m_position.x == 0 && verts[vertIndex].m_position.y == 0)
			continue;

		TransformPositionXY3D( verts[vertIndex].m_position, iBasis, jBasis, translationXY );
	}
}

void AddVertsForAABB2D( std::vector<Vertex_PCU>& verts, AABB2 const& bounds, Rgba8 const& color )
{
	Vec2 bottomLeft = bounds.m_mins;
	Vec2 upperRight = bounds.m_maxs;
	verts.push_back( Vertex_PCU( Vec3( Vec2( bottomLeft.x, bottomLeft.y ) ), color, Vec2(0, 0) ) );
	verts.push_back( Vertex_PCU( Vec3( Vec2( upperRight.x, bottomLeft.y ) ), color, Vec2(1, 0) ) );
	verts.push_back( Vertex_PCU( Vec3( Vec2( upperRight.x, upperRight.y ) ), color, Vec2(1, 1) ) );

	verts.push_back( Vertex_PCU( Vec3( Vec2( upperRight.x, upperRight.y ) ), color, Vec2(1, 1) ) );
	verts.push_back( Vertex_PCU( Vec3( Vec2( bottomLeft.x, upperRight.y ) ), color, Vec2(0, 1) ) );
	verts.push_back( Vertex_PCU( Vec3( Vec2( bottomLeft.x, bottomLeft.y ) ), color, Vec2(0, 0) ) );
}

void AddVertsForAABB2D( std::vector<Vertex_PCU>& verts, AABB2 const& bounds, Rgba8 const& color, Vec2 uvAtMins, Vec2 uvAtMaxs )
{
	Vec2 bottomLeft = bounds.m_mins;
	Vec2 upperRight = bounds.m_maxs;
	verts.push_back( Vertex_PCU( Vec3( Vec2( bottomLeft.x, bottomLeft.y ) ), color, Vec2( uvAtMins.x, uvAtMins.y ) ) );
	verts.push_back( Vertex_PCU( Vec3( Vec2( upperRight.x, bottomLeft.y ) ), color, Vec2( uvAtMaxs.x, uvAtMins.y ) ) );
	verts.push_back( Vertex_PCU( Vec3( Vec2( upperRight.x, upperRight.y ) ), color, Vec2( uvAtMaxs.x, uvAtMaxs.y ) ) );

	verts.push_back( Vertex_PCU( Vec3( Vec2( upperRight.x, upperRight.y ) ), color, Vec2( uvAtMaxs.x, uvAtMaxs.y ) ) );
	verts.push_back( Vertex_PCU( Vec3( Vec2( bottomLeft.x, upperRight.y ) ), color, Vec2( uvAtMins.x, uvAtMaxs.y ) ) );
	verts.push_back( Vertex_PCU( Vec3( Vec2( bottomLeft.x, bottomLeft.y ) ), color, Vec2( uvAtMins.x, uvAtMins.y ) ) );
}

void AddVertsForRect2D( std::vector<Vertex_PCU>& vertArray, Vec2 const& bottomLeft, Vec2 const& upperRight, Rgba8 color )
{
	vertArray.push_back( Vertex_PCU( Vec3( Vec2( bottomLeft.x, bottomLeft.y ) ), color ) );
	vertArray.push_back( Vertex_PCU( Vec3( Vec2( upperRight.x, bottomLeft.y ) ), color ) );
	vertArray.push_back( Vertex_PCU( Vec3( Vec2( upperRight.x, upperRight.y ) ), color ) );

	vertArray.push_back( Vertex_PCU( Vec3( Vec2( upperRight.x, upperRight.y ) ), color ) );
	vertArray.push_back( Vertex_PCU( Vec3( Vec2( bottomLeft.x, upperRight.y ) ), color ) );
	vertArray.push_back( Vertex_PCU( Vec3( Vec2( bottomLeft.x, bottomLeft.y ) ), color ) );
}

void AddVertsForRect2D( std::vector<Vertex_PCU>& vertArray, Vec2 const& bottomLeft, Vec2 const& bottomRight, Vec2 const& topRight, Vec2 const& topLeft, Rgba8 color )
{
	vertArray.push_back( Vertex_PCU( Vec3( bottomLeft ), color ) );
	vertArray.push_back( Vertex_PCU( Vec3( bottomRight ), color ) );
	vertArray.push_back( Vertex_PCU( Vec3( topRight ), color ) );

	vertArray.push_back( Vertex_PCU( Vec3( topRight ), color ) );
	vertArray.push_back( Vertex_PCU( Vec3( topLeft ), color ) );
	vertArray.push_back( Vertex_PCU( Vec3( bottomLeft ), color ) );
}

void AddVertsForHollowRect2D( std::vector<Vertex_PCU>& vertArray, float thickness, Vec2 const& bottomLeft, Vec2 const& bottomRight, Vec2 const& topRight, Vec2 const& topLeft, Rgba8 color )
{
	AddVertsForRect2D( vertArray, bottomLeft + Vec2( 0.f, thickness ), bottomRight + Vec2( 0.f, thickness ), bottomRight, bottomLeft, color );
	AddVertsForRect2D( vertArray, bottomRight + Vec2( 0.f, thickness ), bottomRight + Vec2( thickness, thickness ), topRight + Vec2( thickness, -thickness ), topRight + Vec2( 0.f, -thickness ), color );
	AddVertsForRect2D( vertArray, topRight, topLeft, topLeft + Vec2( 0.f, -thickness ), topRight + Vec2( 0.f, -thickness ), color );
	AddVertsForRect2D( vertArray, bottomLeft + Vec2( -thickness, thickness ), bottomLeft + Vec2( 0.f, thickness ), topLeft + Vec2( 0.f, -thickness ), topLeft + Vec2( -thickness, -thickness ), color );
}

void AddVertsForRing2D( std::vector<Vertex_PCU>& verts, Vec2 const& center, float const& radius, float const& thick, Rgba8 color )
{
	constexpr int numSlices = 24;
	float SLICE_DEGREES = 360.f / static_cast<float>(numSlices);

	float innerRadius = radius - thick * 0.5f;
	float outerRadius = radius + thick * 0.5f;

	for (int triIndex = 0; triIndex < numSlices; triIndex++)
	{
		Vec2 bottomLeft = center + Vec2(
			outerRadius * CosDegrees( SLICE_DEGREES * static_cast<float>(triIndex) ),
			outerRadius * SinDegrees( SLICE_DEGREES * static_cast<float>(triIndex) )
		);
		Vec2 bottomRight = center + Vec2(
			outerRadius * CosDegrees( SLICE_DEGREES * static_cast<float>(triIndex + 1) ),
			outerRadius * SinDegrees( SLICE_DEGREES * static_cast<float>(triIndex + 1) )
		);
		Vec2 topRight = center + Vec2(
			innerRadius * CosDegrees( SLICE_DEGREES * static_cast<float>(triIndex + 1) ),
			innerRadius * SinDegrees( SLICE_DEGREES * static_cast<float>(triIndex + 1) )
		);
		Vec2 topLeft = center + Vec2(
			innerRadius * CosDegrees( SLICE_DEGREES * static_cast<float>(triIndex) ),
			innerRadius * SinDegrees( SLICE_DEGREES * static_cast<float>(triIndex) )
		);

		Vertex_PCU buildQuad[6] =
		{
			Vertex_PCU( Vec3( bottomLeft ), color, Vec2( 0.f, 0.f ) ),
			Vertex_PCU( Vec3( bottomRight ), color, Vec2( 0.f, 0.f ) ),
			Vertex_PCU( Vec3( topRight ), color, Vec2( 0.f, 0.f ) ),

			Vertex_PCU( Vec3( topRight ), color, Vec2( 0.f, 0.f ) ),
			Vertex_PCU( Vec3( topLeft ), color, Vec2( 0.f, 0.f ) ),
			Vertex_PCU( Vec3( bottomLeft ), color, Vec2( 0.f, 0.f ) ),
		};
		AddVertsForRect2D( verts, bottomLeft, bottomRight, topRight, topLeft, color );
	}
}

void AddVertsForDisc2D( std::vector<Vertex_PCU>& verts, Vec2 const& center, float radius, Rgba8 const& color )
{
	constexpr int numSlices = 24;
	float SLICE_DEGREES = 360.f / static_cast<float>(numSlices);

	std::vector<Vertex_PCU> vertexRing;
	vertexRing.reserve( numSlices );

	for (int vertIndex = 0; vertIndex < numSlices; vertIndex++)
	{
		vertexRing.push_back( Vertex_PCU(
			Vec3(
				radius * CosDegrees( vertIndex * SLICE_DEGREES ),
				radius * SinDegrees( vertIndex * SLICE_DEGREES ),
				0.f
			) + Vec3( center ),
			color,
			Vec2( 0.f, 0.f )
		) );
	}

	for (int triIndex = 0; triIndex < numSlices; triIndex++)
	{
		verts.push_back( vertexRing[triIndex] );
		verts.push_back( vertexRing[(triIndex + 1) % 24] );
		verts.push_back( Vertex_PCU( Vec3( center ), color, Vec2( 0.f, 0.f ) ) );
	}
}

void AddVertsForCapsule2D( std::vector<Vertex_PCU>& verts, Vec2 const& boneStart, Vec2 const& boneEnd, float radius, Rgba8 const& color )
{
	constexpr int numSlices = 24;
	float SLICE_DEGREES = 360.f / static_cast<float>(numSlices);
	
	Vec2 boneVector = boneEnd - boneStart;
	float boneDegree = Atan2Degrees( boneVector.y, boneVector.x );

	std::vector<Vertex_PCU> vertexRing;
	vertexRing.reserve( 72 );
	for (int vertIndex = 0; vertIndex < numSlices; vertIndex++)
	{
		vertexRing.push_back( Vertex_PCU(
			Vec3(
				radius * CosDegrees( vertIndex * SLICE_DEGREES + boneDegree - 90.f ),
				radius * SinDegrees( vertIndex * SLICE_DEGREES + boneDegree - 90.f ),
				0.f
			) + ((vertIndex < (numSlices >> 1)) ? Vec3( boneEnd ) : Vec3( boneStart )),
			color,
			Vec2( 0.f, 0.f )
		) );
	}
	for (int triIndex = 0; triIndex < numSlices; triIndex++)
	{
		verts.push_back( vertexRing[triIndex] );
		verts.push_back( vertexRing[(triIndex + 1) % 24] );
		verts.push_back( Vertex_PCU( Vec3( (triIndex < (numSlices >> 1)) ? Vec3( boneEnd ) : Vec3( boneStart ) ), color, Vec2( 0.f, 0.f ) ) );
	}
	Vec2 center = boneStart + boneVector * 0.5f;
	Vec2 iBasisNormal = Vec2( boneVector.y, -boneVector.x ).GetNormalized();
	Vec2 halfDimensions = Vec2( radius, boneVector.GetLength() * 0.5f );
	AddVertsForOBB2D( verts, OBB2( center, iBasisNormal, halfDimensions ), color );
}

void AddVertsForLineSegment2D( std::vector<Vertex_PCU>& verts, Vec2 const& start, Vec2 const& end, float thickness, Rgba8 const& color )
{
	Vec2 lineVector = end - start;
	Vec2 center = start + lineVector * 0.5f;
	Vec2 iBasisNormal = Vec2( lineVector.y, -lineVector.x ).GetNormalized();
	Vec2 halfDimensions = Vec2( thickness, thickness + lineVector.GetLength() * 0.5f );
	AddVertsForOBB2D( verts, OBB2( center, iBasisNormal, halfDimensions ), color );
}

void AddVertsForOBB2D( std::vector<Vertex_PCU>& verts, OBB2 const& box, Rgba8 const& color )
{
	Vec2 iBasis = box.m_iBasisNormal;
	Vec2 jBasis = Vec2( -iBasis.y, iBasis.x );
	Vec2 bottomLeft = box.m_center + box.m_halfDimensions.x * iBasis + box.m_halfDimensions.y * jBasis;
	Vec2 bottomRight = box.m_center + box.m_halfDimensions.x * iBasis - box.m_halfDimensions.y * jBasis;
	Vec2 topRight = box.m_center - box.m_halfDimensions.x * iBasis - box.m_halfDimensions.y * jBasis;
	Vec2 topLeft = box.m_center - box.m_halfDimensions.x * iBasis + box.m_halfDimensions.y * jBasis;

	AddVertsForRect2D( verts, bottomLeft, bottomRight, topRight, topLeft, color );
}

void AddVertsForArrow2D( std::vector<Vertex_PCU>& verts, Vec2 tailPos, Vec2 tipPos, float arrowSize, float lineThickness, Rgba8 const& color  )
{
	AddVertsForLineSegment2D( verts, tipPos, tailPos, lineThickness, color );
	//drawLine( tipPos, tailPos, lineThickness * 0.5, color );
	Vec2 arrowNormal = (tailPos - tipPos).GetNormalized();
	Vec2 leftArm = arrowNormal.GetRotatedDegrees( 45 );
	Vec2 rightArm = arrowNormal.GetRotatedDegrees( -45 );
	AddVertsForLineSegment2D( verts, tipPos, tipPos + leftArm * arrowSize, lineThickness, color );
	AddVertsForLineSegment2D( verts, tipPos, tipPos + rightArm * arrowSize, lineThickness, color );
}

void AddVertsForQuad3D( std::vector<Vertex_PCU>& verts, Vec3 const& bottomLeft, Vec3 const& bottomRight, Vec3 const& topRight, Vec3 const& topLeft, Rgba8 const& color, AABB2 const& UVs )
{
	verts.push_back( Vertex_PCU( bottomLeft, color, Vec2( UVs.m_mins ) ) );
	verts.push_back( Vertex_PCU( bottomRight, color, Vec2( UVs.m_maxs.x, UVs.m_mins.y ) ) );
	verts.push_back( Vertex_PCU( topRight, color, Vec2( UVs.m_maxs ) ) );
	verts.push_back( Vertex_PCU( topRight, color, Vec2( UVs.m_maxs ) ) );
	verts.push_back( Vertex_PCU( topLeft, color, Vec2( UVs.m_mins.x, UVs.m_maxs.y ) ) );
	verts.push_back( Vertex_PCU( bottomLeft, color, Vec2( UVs.m_mins ) ) );
}

void AddVertsForQuad3D( std::vector<Vertex_PCU>& verts, std::vector<unsigned int>& indexes, Vec3 const& bottomLeft, Vec3 const& bottomRight, Vec3 const& topRight, Vec3 const& topLeft, Rgba8 const& color, AABB2 const& UVs )
{
	int index = (int)verts.size();

	verts.push_back( Vertex_PCU( bottomLeft, color, Vec2( UVs.m_mins ) ) );
	verts.push_back( Vertex_PCU( bottomRight, color, Vec2( UVs.m_maxs.x, UVs.m_mins.y ) ) );
	verts.push_back( Vertex_PCU( topRight, color, Vec2( UVs.m_maxs ) ) );
	verts.push_back( Vertex_PCU( topLeft, color, Vec2( UVs.m_mins.x, UVs.m_maxs.y ) ) );

	indexes.push_back( index + 0 );
	indexes.push_back( index + 1 );
	indexes.push_back( index + 2 );
	indexes.push_back( index + 2 );
	indexes.push_back( index + 3 );
	indexes.push_back( index + 0 );
}

void AddVertsForQuad3D( std::vector<Vertex_PCUTBN>& verts, std::vector<unsigned int>& indexes, Vec3 const& bottomLeft, Vec3 const& bottomRight, Vec3 const& topRight, Vec3 const& topLeft, Rgba8 const& color, AABB2 const& UVs )
{
	Vec3 normal = CrossProduct3D( bottomRight - bottomLeft, topLeft - bottomLeft ).GetNormalized();

	int index = (int)verts.size();

	verts.push_back( Vertex_PCUTBN( bottomLeft, color, Vec2( UVs.m_mins ), Vec3::ZERO, Vec3::ZERO, normal ) );
	verts.push_back( Vertex_PCUTBN( bottomRight, color, Vec2( UVs.m_maxs.x, UVs.m_mins.y ), Vec3::ZERO, Vec3::ZERO, normal ) );
	verts.push_back( Vertex_PCUTBN( topRight, color, Vec2( UVs.m_maxs ), Vec3::ZERO, Vec3::ZERO, normal ) );
	verts.push_back( Vertex_PCUTBN( topLeft, color, Vec2( UVs.m_mins.x, UVs.m_maxs.y ), Vec3::ZERO, Vec3::ZERO, normal ) );

	indexes.push_back( index + 0 );
	indexes.push_back( index + 1 );
	indexes.push_back( index + 2 );
	indexes.push_back( index + 2 );
	indexes.push_back( index + 3 );
	indexes.push_back( index + 0 );
}

void AddVertsForRoundedQuad3D( std::vector<Vertex_PCUTBN>& verts, std::vector<unsigned int>& indexes, Vec3 const& bottomLeft, Vec3 const& bottomRight, Vec3 const& topRight, Vec3 const& topLeft, Rgba8 const& color, AABB2 const& UVs )
{
	Vec3 bottomLeftNormal;
	Vec3 bottomMidNormal;
	Vec3 bottomRightNormal;
	Vec3 topLeftNormal;
	Vec3 topMidNormal;
	Vec3 topRightNormal;

	Vec3 bottomMid = (bottomLeft + bottomRight) * 0.5f;
	Vec3 topMid = (topLeft + topRight) * 0.5f;

	bottomLeftNormal = (bottomLeft - bottomRight).GetNormalized();
	bottomRightNormal = -bottomLeftNormal;
	topLeftNormal = (topLeft - topRight).GetNormalized();
	topRightNormal = -topLeftNormal;
	topMidNormal = CrossProduct3D( topLeft - topMid, bottomMid - topMid ).GetNormalized();
	bottomMidNormal = CrossProduct3D( topMid - bottomMid, bottomLeft - bottomMid ).GetNormalized();

	int index = (int)verts.size();

	verts.push_back( Vertex_PCUTBN( bottomLeft, color, Vec2( UVs.m_mins ), Vec3::ZERO, Vec3::ZERO, bottomLeftNormal ) );
	verts.push_back( Vertex_PCUTBN( bottomMid, color, Vec2( (UVs.m_maxs.x + UVs.m_mins.x) * 0.5f, UVs.m_mins.y ), Vec3::ZERO, Vec3::ZERO, bottomMidNormal ) );
	verts.push_back( Vertex_PCUTBN( topMid, color, Vec2( (UVs.m_maxs.x + UVs.m_mins.x) * 0.5f, UVs.m_maxs.y ), Vec3::ZERO, Vec3::ZERO, topMidNormal ) );
	verts.push_back( Vertex_PCUTBN( topLeft, color, Vec2( UVs.m_mins.x, UVs.m_maxs.y ), Vec3::ZERO, Vec3::ZERO, topLeftNormal ) );

	indexes.push_back( index + 0 );
	indexes.push_back( index + 1 );
	indexes.push_back( index + 2 );
	indexes.push_back( index + 2 );
	indexes.push_back( index + 3 );
	indexes.push_back( index + 0 );

	verts.push_back( Vertex_PCUTBN( bottomMid, color, Vec2( (UVs.m_maxs.x + UVs.m_mins.x) * 0.5f, UVs.m_mins.y ), Vec3::ZERO, Vec3::ZERO, bottomMidNormal ) );
	verts.push_back( Vertex_PCUTBN( bottomRight, color, Vec2( UVs.m_maxs.x, UVs.m_mins.y ), Vec3::ZERO, Vec3::ZERO, bottomRightNormal ) );
	verts.push_back( Vertex_PCUTBN( topRight, color, Vec2( UVs.m_maxs ), Vec3::ZERO, Vec3::ZERO, topRightNormal ) );
	verts.push_back( Vertex_PCUTBN( topMid, color, Vec2( (UVs.m_maxs.x + UVs.m_mins.x) * 0.5f, UVs.m_maxs.y ), Vec3::ZERO, Vec3::ZERO, topMidNormal ) );

	indexes.push_back( index + 4 );
	indexes.push_back( index + 5 );
	indexes.push_back( index + 6 );
	indexes.push_back( index + 6 );
	indexes.push_back( index + 7 );
	indexes.push_back( index + 4 );
}

void AddVertsForAABB3D( std::vector<Vertex_PCU>& verts, AABB3 const& bounds, Rgba8 const& color/* = Rgba8::WHITE*/, AABB2 const& UVs/* = AABB2::ZERO_TO_ONE*/ )
{
	Vec3 point1( bounds.m_maxs.x, bounds.m_maxs.y, bounds.m_maxs.z );
	Vec3 point2( bounds.m_mins.x, bounds.m_maxs.y, bounds.m_maxs.z );
	Vec3 point3( bounds.m_mins.x, bounds.m_mins.y, bounds.m_maxs.z );
	Vec3 point4( bounds.m_maxs.x, bounds.m_mins.y, bounds.m_maxs.z );
	Vec3 point5( bounds.m_maxs.x, bounds.m_maxs.y, bounds.m_mins.z );
	Vec3 point6( bounds.m_mins.x, bounds.m_maxs.y, bounds.m_mins.z );
	Vec3 point7( bounds.m_mins.x, bounds.m_mins.y, bounds.m_mins.z );
	Vec3 point8( bounds.m_maxs.x, bounds.m_mins.y, bounds.m_mins.z );
	AddVertsForQuad3D( verts, point3, point4, point1, point2, color, UVs );//z
	AddVertsForQuad3D( verts, point6, point5, point8, point7, color, UVs );//-z
	AddVertsForQuad3D( verts, point5, point6, point2, point1, color, UVs );//y
	AddVertsForQuad3D( verts, point7, point8, point4, point3, color, UVs );//-y
	AddVertsForQuad3D( verts, point8, point5, point1, point4, color, UVs );//x
	AddVertsForQuad3D( verts, point6, point7, point3, point2, color, UVs );//-x
}

void AddVertsForOBB3D( std::vector<Vertex_PCU>& verts, OBB3 const& bounds, Rgba8 const& color, AABB2 const& UVs )
{
	Vec3 point1( bounds.m_center + bounds.m_iBasisNormal * bounds.m_halfDimensions.x + bounds.m_jBasisNormal * bounds.m_halfDimensions.y + bounds.m_kBasisNormal * bounds.m_halfDimensions.z );
	Vec3 point2( bounds.m_center - bounds.m_iBasisNormal * bounds.m_halfDimensions.x + bounds.m_jBasisNormal * bounds.m_halfDimensions.y + bounds.m_kBasisNormal * bounds.m_halfDimensions.z );
	Vec3 point3( bounds.m_center - bounds.m_iBasisNormal * bounds.m_halfDimensions.x - bounds.m_jBasisNormal * bounds.m_halfDimensions.y + bounds.m_kBasisNormal * bounds.m_halfDimensions.z );
	Vec3 point4( bounds.m_center + bounds.m_iBasisNormal * bounds.m_halfDimensions.x - bounds.m_jBasisNormal * bounds.m_halfDimensions.y + bounds.m_kBasisNormal * bounds.m_halfDimensions.z );
	Vec3 point5( bounds.m_center + bounds.m_iBasisNormal * bounds.m_halfDimensions.x + bounds.m_jBasisNormal * bounds.m_halfDimensions.y - bounds.m_kBasisNormal * bounds.m_halfDimensions.z );
	Vec3 point6( bounds.m_center - bounds.m_iBasisNormal * bounds.m_halfDimensions.x + bounds.m_jBasisNormal * bounds.m_halfDimensions.y - bounds.m_kBasisNormal * bounds.m_halfDimensions.z );
	Vec3 point7( bounds.m_center - bounds.m_iBasisNormal * bounds.m_halfDimensions.x - bounds.m_jBasisNormal * bounds.m_halfDimensions.y - bounds.m_kBasisNormal * bounds.m_halfDimensions.z );
	Vec3 point8( bounds.m_center + bounds.m_iBasisNormal * bounds.m_halfDimensions.x - bounds.m_jBasisNormal * bounds.m_halfDimensions.y - bounds.m_kBasisNormal * bounds.m_halfDimensions.z );
	AddVertsForQuad3D( verts, point3, point4, point1, point2, color, UVs );
	AddVertsForQuad3D( verts, point6, point5, point8, point7, color, UVs );
	AddVertsForQuad3D( verts, point5, point6, point2, point1, color, UVs );
	AddVertsForQuad3D( verts, point7, point8, point4, point3, color, UVs );
	AddVertsForQuad3D( verts, point8, point5, point1, point4, color, UVs );
	AddVertsForQuad3D( verts, point6, point7, point3, point2, color, UVs );
}

void AddVertsForSphere3D( std::vector<Vertex_PCU>& verts, Vec3 const& center, float radius, Rgba8 const& color/* = Rgba8::WHITE*/, AABB2 const& UVs/* = AABB2::ZERO_TO_ONE*/, int numLongtitude/* = 32*/, int numLatitude/* = 16*/ )
{
	float latitudeDegrees = 180.f / numLatitude;
	float longtitudeDegrees = 360.f / numLongtitude;
	float uvGridHeight = 1.f / (float)numLatitude;
	float uvGridWidth = 1.f / (float)numLongtitude;
	for (int i = 0; i < numLatitude; i++)
	{
		for (int j = 0; j < numLongtitude; j++)
		{
			Vec3 bottomLeft = center + Vec3::MakeFromPolarDegrees( 90.f - latitudeDegrees * i, longtitudeDegrees * j, radius );
			Vec3 buttomRight = center + Vec3::MakeFromPolarDegrees( 90.f - latitudeDegrees * i, longtitudeDegrees * (j + 1), radius );
			Vec3 topRight = center + Vec3::MakeFromPolarDegrees( 90.f - latitudeDegrees * (i + 1), longtitudeDegrees * (j + 1), radius );
			Vec3 topLeft = center + Vec3::MakeFromPolarDegrees( 90.f - latitudeDegrees * (i + 1), longtitudeDegrees * j, radius );
			Vec2 uvAtMin = Vec2( uvGridWidth * j + UVs.m_mins.x, UVs.m_maxs.y - uvGridHeight * (i + 1) );
			Vec2 uvAtMax = Vec2( uvGridWidth * (j + 1.f) + UVs.m_mins.x, UVs.m_maxs.y - uvGridHeight * i );
			AddVertsForQuad3D( verts, bottomLeft, buttomRight, topRight, topLeft, color, AABB2( uvAtMin, uvAtMax ) );
		}
	}
}

void AddVertsForCylinder3D( std::vector<Vertex_PCU>& verts, Vec3 const& start, Vec3 const& end, float radius, Rgba8 const& color, AABB2 const& UVs, int numSlices )
{
	Vec3 kBasis = ((end - start) * 100.f).GetNormalized();
	Vec3 iBasis = Vec3( 1.f, 0.f, 0.f );
	Vec3 jBasis = Vec3( 0.f, 1.f, 0.f );
	if (kBasis != Vec3( 0.f, 0.f, 1.f ) && kBasis != Vec3( 0.f, 0.f, -1.f ))
	{
		jBasis = CrossProduct3D( Vec3( 0.f, 0.f, 1.f ), kBasis );
		iBasis = CrossProduct3D( jBasis, kBasis );
	}
	float SLICE_DEGREES = 360.f / static_cast<float>(numSlices);
	Mat44 transform( iBasis, jBasis, kBasis, Vec3::ZERO );
	std::vector<Vertex_PCU> vertexRingStart;
	vertexRingStart.reserve( numSlices );
	std::vector<Vertex_PCU> vertexRingEnd;
	vertexRingEnd.reserve( numSlices );
	float height = (end - start).GetLength();
	Vec2 UVsSize = UVs.GetDimensions();
	for (int vertIndex = 0; vertIndex < numSlices; vertIndex++)
	{
		float CD = CosDegrees( vertIndex * SLICE_DEGREES );
		float SD = SinDegrees( vertIndex * SLICE_DEGREES );
		vertexRingStart.push_back( Vertex_PCU(
			Vec3(
				radius * CD,
				radius * SD,
				0.f
			) + start,
			color,
			UVs.m_mins + Vec2( (CD * 0.5f + 0.5f) * UVsSize.x, (SD * 0.5f + 0.5f) * UVsSize.y )
		) );
		vertexRingEnd.push_back( Vertex_PCU(
			Vec3(
				radius * CD,
				radius * SD,
				0.f
			) + start + Vec3(0.f, 0.f, height),
			color,
			UVs.m_mins + Vec2( (CD * 0.5f + 0.5f) * UVsSize.x, (SD * 0.5f + 0.5f) * UVsSize.y )
		) );
	}
	for (int triIndex = 0; triIndex < numSlices; triIndex++)
	{
		Vertex_PCU pointA = RotateVertexAboutPivot( vertexRingStart[(triIndex + 1) % numSlices], start, transform );
		Vertex_PCU pointB = RotateVertexAboutPivot( vertexRingStart[triIndex], start, transform );
		Vertex_PCU pointC = RotateVertexAboutPivot( Vertex_PCU( start, color, UVs.m_mins + Vec2( 0.5f * UVsSize.x, 0.5f * UVsSize.y ) ), start, transform );
		verts.push_back( pointA );
		verts.push_back( pointB );
		verts.push_back( pointC );
	}
	for (int triIndex = 0; triIndex < numSlices; triIndex++)
	{
		Vertex_PCU pointA = RotateVertexAboutPivot( vertexRingEnd[triIndex], start, transform );
		Vertex_PCU pointB = RotateVertexAboutPivot( vertexRingEnd[(triIndex + 1) % numSlices], start, transform );
		Vertex_PCU pointC = RotateVertexAboutPivot( Vertex_PCU( start + Vec3( 0.f, 0.f, height ), color, UVs.m_mins + Vec2( 0.5f * UVsSize.x, 0.5f * UVsSize.y ) ), start, transform );
		verts.push_back( pointA );
		verts.push_back( pointB );
		verts.push_back( pointC );
	}
	float pieceLength = 1.f / numSlices;
	for (int triIndex = 0; triIndex < numSlices; triIndex++)
	{
		Vec3 bottomLeft = RotateVertexAboutPivot( vertexRingStart[triIndex], start, transform ).m_position;
		Vec3 buttomRight = RotateVertexAboutPivot( vertexRingStart[(triIndex + 1) % numSlices], start, transform ).m_position;
		Vec3 topRight = RotateVertexAboutPivot( vertexRingEnd[(triIndex + 1) % numSlices], start, transform ).m_position;
		Vec3 topLeft = RotateVertexAboutPivot( vertexRingEnd[triIndex], start, transform ).m_position;
		AddVertsForQuad3D( verts, bottomLeft, buttomRight, topRight, topLeft, color, AABB2( Vec2( UVs.m_mins.x + triIndex * pieceLength * UVsSize.x, UVs.m_mins.y ), Vec2( UVs.m_mins.x + ((triIndex + 1) % numSlices) * pieceLength * UVsSize.x, UVs.m_maxs.y ) ) );
	}
}

void AddVertsForCone3D( std::vector<Vertex_PCU>& verts, Vec3 const& start, Vec3 const& end, float radius, Rgba8 const& color, AABB2 const& UVs, int numSlices )
{
	Vec3 kBasis = ((end - start) * 100.f).GetNormalized();
	Vec3 iBasis = Vec3( 1.f, 0.f, 0.f );
	Vec3 jBasis = Vec3( 0.f, 1.f, 0.f );
	if (kBasis != Vec3( 0.f, 0.f, 1.f ) && kBasis != Vec3( 0.f, 0.f, -1.f ))
	{
		jBasis = CrossProduct3D( Vec3( 0.f, 0.f, 1.f ), kBasis );
		iBasis = CrossProduct3D( jBasis, kBasis );
	}
	float SLICE_DEGREES = 360.f / static_cast<float>(numSlices);
	Mat44 transform( iBasis, jBasis, kBasis, Vec3::ZERO );
	std::vector<Vertex_PCU> vertexRing;
	vertexRing.reserve( numSlices );
	float height = (end - start).GetLength();
	Vec2 UVsSize = UVs.GetDimensions();
	for (int vertIndex = 0; vertIndex < numSlices; vertIndex++)
	{
		float CD = CosDegrees( vertIndex * SLICE_DEGREES );
		float SD = SinDegrees( vertIndex * SLICE_DEGREES );
		vertexRing.push_back( Vertex_PCU(
			Vec3(
				radius * CD,
				radius * SD,
				0.f
			) + start,
			color,
			UVs.m_mins + Vec2( (CD * 0.5f + 0.5f) * UVsSize.x, (SD * 0.5f + 0.5f) * UVsSize.y )
		) );
	}
	for (int triIndex = 0; triIndex < numSlices; triIndex++)
	{
		Vertex_PCU pointA = RotateVertexAboutPivot( vertexRing[(triIndex + 1) % numSlices], start, transform );
		Vertex_PCU pointB = RotateVertexAboutPivot( vertexRing[triIndex], start, transform );
		Vertex_PCU pointC = RotateVertexAboutPivot( Vertex_PCU( start, color, UVs.m_mins + Vec2( 0.5f * UVsSize.x, 0.5f * UVsSize.y ) ), start, transform );
		verts.push_back( pointA );
		verts.push_back( pointB );
		verts.push_back( pointC );
	}
	for (int triIndex = 0; triIndex < numSlices; triIndex++)
	{
		Vertex_PCU bottomLeft = RotateVertexAboutPivot( vertexRing[triIndex], start, transform );
		Vertex_PCU buttomRight = RotateVertexAboutPivot( vertexRing[(triIndex + 1) % numSlices], start, transform );
		Vertex_PCU topVert = RotateVertexAboutPivot( Vertex_PCU( start + Vec3( 0.f, 0.f, height ), color, Vec2( 0.5f * UVsSize.x, 0.5f * UVsSize.y ) ), start, transform );
		verts.push_back( bottomLeft );
		verts.push_back( buttomRight );
		verts.push_back( topVert );
	}
}

AABB2 GetVertexBounds2D( std::vector<Vertex_PCU>& verts )
{
	if (verts.size() == 0)
		return AABB2();

	float minX = verts[0].m_position.x;
	float maxX = verts[0].m_position.x;
	float minY = verts[0].m_position.y;
	float maxY = verts[0].m_position.y;
	for (Vertex_PCU vert : verts)
	{
		minX = (minX > vert.m_position.x) ? vert.m_position.x : minX;
		maxX = (maxX < vert.m_position.x) ? vert.m_position.x : maxX;
		minY = (minY > vert.m_position.y) ? vert.m_position.y : minY;
		maxY = (maxY < vert.m_position.y) ? vert.m_position.y : maxY;
	}
	return AABB2( Vec2( minX, minY ), Vec2( maxX, maxY ) );
}
