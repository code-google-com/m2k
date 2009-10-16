#include <iostream>

#include <gsl/gsl_rng.h>
#include <gsl/gsl_qrng.h>

#include <ri.h>

#include "PR.h"

using namespace std;

DiffusionParticleResolver::DiffusionParticleResolver() : mNCopies(0), mFalloff(0)
{
}

DiffusionParticleResolver::~DiffusionParticleResolver()
{
}

RtVoid DiffusionParticleResolver::DoIt(RtInt NVerts, RtInt N, RtToken Tokens[], RtPointer Data[])
{
	const int NCopies = mNCopies;
	const int Falloff = mFalloff;

	// We do not need to do anything.
	if( NCopies == 0 )
	{
		RiPointsV(NVerts,N,Tokens,Data);
		return;
	}
}

void DiffusionParticleResolver::SetNCopies(const RtInt& NCopies)
{
	mNCopies = NCopies;
}

void DiffusionParticleResolver::SetFalloff(const RtInt& Falloff)
{
	mFalloff = Falloff;
}
