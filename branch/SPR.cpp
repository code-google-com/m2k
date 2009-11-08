/*
* Copyright (c) 1998, Bo Zhou <Bo.Schwarzstein@gmail.com>
* All rights reserved.
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
*     * Redistributions of source code must retain the above copyright
*       notice, this list of conditions and the following disclaimer.
*     * Redistributions in binary form must reproduce the above copyright
*       notice, this list of conditions and the following disclaimer in the
*       documentation and/or other materials provided with the distribution.
*     * Neither the name of the Bo Zhou, nor the names of its contributors
*       may be used to endorse or promote products derived from this
*       software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
* EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL THE REGENTS AND CONTRIBUTORS BE LIABLE FOR ANY
* DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
* ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <iostream>

#include <boost/shared_array.hpp>

#include <fbxsdk.h>

#include "SPR.h"

using namespace std;
using namespace PR;

class RawModel
{
public:
	RawModel() : NF(0), NI(0), NV(0), NN(0)
	{
	}
	size_t NF, NI, NV, NN;
	boost::shared_array<unsigned int> F,I;
	boost::shared_array<Float3> V,N;
};

/** \todo To deal with .obj and .fbx
 *
 */
static bool ReadWavefrontObj(const char* Path)
{
	FILE* FP = fopen(Path,"r");
	char LineBuf[2049];
	while( fgets(LineBuf,2048,FP) )
	{
	}
	return true;
}

static bool ReadFBX(const char* Path, RawModel& Model)
{
	KFbxSdkManager* SdkMgr = KFbxSdkManager::Create();
	KFbxScene* Scn = KFbxScene::Create(SdkMgr, "");
	KFbxImporter* Imp = KFbxImporter::Create(SdkMgr,"");

	bool R = Imp->Initialize(Path);
	if( R != true )
	{
		cerr<<"[ERROR] Can't Load FBX File ["<<Path<<"]"<<endl;
		// R = false;
	}else
	{
		if( Imp->Import(Scn) )
		{

		}else
		{
			R = false;
		}
	}

	Imp->Destroy();
	Scn->Destroy();
	SdkMgr->Destroy();
	return R;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

SurfaceParticleResolver::SurfaceParticleResolver() : mSubdLevel(0)
{
}

SurfaceParticleResolver::~SurfaceParticleResolver()
{
}

void SurfaceParticleResolver::SetPath(const char* Path)
{
	mPath = Path;
}

void SurfaceParticleResolver::SetSubdLevel(const int& SubdLevel)
{
	mSubdLevel = SubdLevel;
}

RtVoid SurfaceParticleResolver::DoIt(RtInt NVerts, RtInt N, RtToken Tokens[], RtPointer Data[])
{
	// The polygon model should be in object coordinate system.
}
