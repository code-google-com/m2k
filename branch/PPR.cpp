/**
* AUTHOR : Bo Zhou
* TIME : 16:39 2009/10/20
* CONTENT : PPR's processing progress could be merged as following steps,
* 1) Read external polygon model from particles, e.
* 2) Subdivide it by Catmull-Clark or Loop.
* 3) Get centric position of each quad.
* 4) Get level set from the tent function.
* 5) Fill each voxel with attributes.
* 6) Render each voxel with random points in it with interpolated attributes.
*/

#include <iostream>

#include <boost/shared_array.hpp>

struct float3
{
	float x,y,z;
};

using namespace std;

class RawModel
{
public:
	size_t NF, NI, NV, NN;
	boost::shared_array<unsigned int> F,I;
	boost::shared_array<float3> V,N;
};

PolygonParticleResolver::PolygonParticleResolver() : mSubdLevel(0)
{
}

PolygonParticleResolver::~PolygonParticleResolver()
{
}

void PolygonParticleResolver::SetPath(const char* Path)
{
	mPath = Path;
}

void PolygonParticleResolver::SetSubdLevel(const int& SubdLevel)
{
	mSubdLevel = SubdLevel;
}

RtVoid PolygonParticleResolver::DoIt(RtInt NVerts, RtInt N, RtToken Tokens[], RtPointer Data[])
{
	// The polygon model should be in object coordinate system.
}
