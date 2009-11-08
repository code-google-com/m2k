#include "VPR.h"

#include <float.h>

using namespace std;
using namespace PR;

VolumetricParticleSolver::VolumetricParticleSolver()
{
	mRes[0] = mRes[1] = mRes[2] = 2;
}

VolumetricParticleSolver::~VolumetricParticleSolver()
{
}

RtVoid VolumetricParticleSolver::DoIt(RtInt NVert, RtInt N, RtToken Tokens[], RtPointer Data[])
{
	// Calculate BB
	RtPoint* PosPointer = NULL;
	for( int i=0; i<N; ++i )
	{
		if( strcmp(Tokens[i],"P") == 0 )
		{
			PosPointer = (RtPoint*)Data[i];
			break;
		}
	}

	Float3 BB[2];// Min & Max
	BB[0].x = BB[0].y = BB[0].z = FLT_MAX;
	BB[1].x = BB[1].y = BB[1].z = - FLT_MAX;
	for( int i=0; i<NVert; ++i )
	{
		BB[0].x = min( BB[0].x, PosPointer[i][0] );
		BB[0].y = min( BB[0].y, PosPointer[i][1] );
		BB[0].z = min( BB[0].z, PosPointer[i][2] );

		BB[1].x = max( BB[1].x, PosPointer[i][0] );
		BB[1].y = max( BB[1].y, PosPointer[i][1] );
		BB[1].z = max( BB[1].z, PosPointer[i][2] );
	}



}

void VolumetricParticleSolver::SetRes(const RtInt& X, const RtInt& Y, const RtInt& Z)
{
}
