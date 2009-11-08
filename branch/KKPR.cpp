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

#include "KKPR.h"
#include "PRT.h"

#include <iostream>
#include <memory>
#include <string>

#include <boost/scoped_array.hpp>

using namespace std;
using namespace PR;

#define POSITION "Position"
#define COLOR "Color"
#define WIDTH "Width"
#define OPACITY "Opacity"

static float ConstWidth = 0.16666666f;

KKParticleResolver::KKParticleResolver()
{
}

KKParticleResolver::~KKParticleResolver()
{
}

void KKParticleResolver::SetPath(const char* Path)
{
	mPath = Path;
}

RtVoid KKParticleResolver::DoIt(RtInt NVerts, RtInt N, RtToken Tokens[], RtPointer Data[])
{
	auto_ptr<PRT> PRTFile( new PRT );
	const string Path = mPath;
	if( !PRTFile->ReadFromFile( Path.c_str() ) )
	{
		cerr<<"ParticleResolverPlugin : KKParticleResolver Can't Open PRT File ["<<Path<<"]"<<endl;
		return;
	}

	const long long Count = PRTFile->GetCount();
	
	cout<<"ParticleResolverPlugin : KKParticleResolver Get ["<<PRTFile->GetCount()<<"] Points"<<endl;

	PRTChannelDict ChDict;
	PRTFile->GetChannels(ChDict);

	for( PRTChannelDict::iterator i = ChDict.begin(); i != ChDict.end(); ++i )
	{
		cout<<"ParticleResolverPlugin : KKParticleResolver Detected ["<<i->first<<"] Channel"<<endl;
	}

	RtToken Tokens2[4];
	RtPointer Data2[4];

	PRTChannelDict::iterator itr = ChDict.find(POSITION);
	if( itr == ChDict.end() )
	{
		cerr<<"ParticleResolverPlugin : KKParticleResolver Can't Find ["<<POSITION<<"] Channel"<<endl;
		return;
	}else{
		Tokens2[0] = "P";
		Data2[0] = (RtPointer)itr->second.Data.get();
	}

	itr = ChDict.find(COLOR);
	if( itr == ChDict.end() )
	{
		cerr<<"ParticleResolverPlugin : KKParticleResolver Can't Find ["<<COLOR<<"] Channel"<<endl;
		return;
	}else{
		Tokens2[1] = "Cs";
		Data2[1] = (RtPointer)itr->second.Data.get();
	}


	itr = ChDict.find(WIDTH);
	if( itr == ChDict.end() )
	{
		cerr<<"ParticleResolverPlugin : KKParticleResolver Can't Find ["<<WIDTH<<"] Channel"<<endl;
		
		Tokens2[2] = "constwidth";
		Data2[2] = &ConstWidth;
	}else
	{
		Tokens2[2] = "width";
		Data2[2] = &ConstWidth;
	}

	boost::scoped_array<Float3> Os( new Float3[Count] );

	itr = ChDict.find(OPACITY);
	Tokens2[3] = "Os";
	if( itr == ChDict.end() )
	{
		cerr<<"ParticleResolverPlugin : KKParticleResolver Can't Find ["<<OPACITY<<"] Channel"<<endl;
		for( long long i=0; i<Count; ++i )
			Os[i].x = Os[i].y = Os[i].z = 1.0f;
		Data2[3] = (RtPointer)Os.get();
	}else
	{
		Data2[3] = (RtPointer)itr->second.Data.get();
	}

	RiPointsV(Count,4,Tokens2,Data2);

}
