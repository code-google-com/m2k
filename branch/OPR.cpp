#include <iostream>

#include <ri.h>

#include "PR.h"

using namespace std;

OriginParticleResolver::OriginParticleResolver()
{
}

OriginParticleResolver::~OriginParticleResolver()
{
}

RtVoid OriginParticleResolver::DoIt(RtInt NVerts, RtInt N, RtToken Tokens[], RtPointer Data[])
{
	RiPointsV(NVerts,N,Tokens,Data);
}
