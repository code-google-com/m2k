#include "PR.h"

#include <ri.h>

ExternalParticleResolver::ExternalParticleResolver()
{
}

ExternalParticleResolver::~ExternalParticleResolver()
{
}

void ExternalParticleResolver::SetPath(const char* Path)
{
	mPath = Path;
}

RtVoid ExternalParticleResolver::DoIt(RtInt NVerts, RtInt N, RtToken Tokens[], RtPointer Data[])
{
	RiPointsV(NVerts,N,Tokens,Data);
}
