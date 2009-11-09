// Copyright (c) 2009, Bo Zhou<http://jedimaster.cnblogs.com>, Saber Jlassi<http://www.smallpixel.net/>
// All rights reserved.
// 
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//     * Neither the name of the organization to which the authors belong nor the
//       names of its contributors may be used to endorse or promote products
//       derived from this software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY AUTHORS AND CONTRIBUTORS ''AS IS'' AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL AUTHORS AND CONTRIBUTORS BE LIABLE FOR ANY
// DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include <maya/MFnPlugin.h>
#include <maya/MStatus.h>
#include <maya/MPxCommand.h>
#include <maya/MArgList.h>
#include <maya/MGlobal.h>
#include <maya/MString.h>
#include <maya/MItDag.h>
#include <maya/MObject.h>
#include <maya/MFnParticleSystem.h>
#include <maya/MDoubleArray.h>
#include <maya/MVectorArray.h>
#include <maya/MIntArray.h>

#include <iostream>
#include <memory>
#include <string>
#include <fstream>
#include <sstream>
#include <zlib.h>

#include "PRT.h"

using namespace std;

class M2K : public MPxCommand
{
public:
	static void* creator();
	MStatus doIt(const MArgList&);
};

void* M2K::creator()
{
	return new M2K;
}

MStatus M2K::doIt(const MArgList& Args)
{
	if( Args.length() < 3 )
	{
		MGlobal::displayError("M2K need the EXPORT PATH and a PARTICLESHAPE's NAME, and at least one ATTRIBUTE's NAME you want to export." );
		return MStatus::kFailure;
	}
	
	MString Path( Args.asString(0) );
	MString PSName( Args.asString(1) );

	MItDag PSItr(MItDag::kDepthFirst, MFn::kParticle);
	MObject PSObj;
	for( ; ! PSItr.isDone(); PSItr.next() )
	{
		if( PSItr.fullPathName().indexW(PSName) > 0 )
			PSObj = PSItr.currentItem();
	}
	if( PSObj.apiType() != MFn::kParticle )
	{
		MGlobal::displayError("M2K can't find your PARTICLESHAPE.");
		return MStatus::kFailure;
	}
	
	MFnParticleSystem PS(PSObj);

	unsigned int Count = PS.count();

	try
	{
		auto_ptr<PRT> PRTFile( new PRT(Count) );
		//Dump attributes
		for( unsigned int i=2; i<Args.length(); ++i )
		{
			string AttrName( Args.asString(i).toLowerCase().asChar() );
			if( AttrName == "position" )
			{
				MVectorArray VA;
				PS.position( VA );
				Array RawArray( new char[sizeof(float)*Count*3] );
				float* p = (float*)RawArray.get();
				for( unsigned int a=0; a<Count; ++a )
				{
					unsigned int b = a*3;
					MVector P = VA[a];
					p[b+0] = (float)P.x;
					p[b+1] = (float)P.y;
					p[b+2] = (float)P.z;
				}
				PRTFile->AddChannel("Position",kFLOAT32,3,RawArray);

				//Remember to release memory.
				VA.setLength(0);
			}else if( AttrName == "acceleration" )
			{
				MVectorArray VA;
				PS.acceleration( VA );
				Array RawArray( new char[sizeof(float)*Count*3] );
				float* p = (float*)RawArray.get();
				for( unsigned int a=0; a<Count; ++a )
				{
					unsigned int b = a*3;
					MVector P = VA[a];
					p[b+0] = (float)P.x;
					p[b+1] = (float)P.y;
					p[b+2] = (float)P.z;
				}
				PRTFile->AddChannel("Acceleration",kFLOAT32,3,RawArray);
				VA.setLength(0);
			}else if( AttrName == "radius" )
			{
				MDoubleArray DA;
				PS.radius( DA );
				Array RawArray( new char[sizeof(float)*Count] );
				float* p = (float*)RawArray.get();
				for( unsigned int a=0; a<Count; ++a )
				{
					p[a] = (float)DA[a];
				}
				PRTFile->AddChannel("Radius",kFLOAT32,1,RawArray);
				DA.setLength(0);
			}else if( AttrName == "velocity" )
			{
				MVectorArray VA;
				PS.velocity(VA);
				Array RawArray( new char[sizeof(float)*Count*3] );
				float* p = (float*)RawArray.get();
				for( unsigned int a=0; a<Count; ++a )
				{
					unsigned int b = a*3;
					MVector P = VA[a];
					p[b+0] = (float)P.x;
					p[b+1] = (float)P.y;
					p[b+2] = (float)P.z;
				}
				PRTFile->AddChannel("Velocity",kFLOAT32,3,RawArray);
				VA.setLength(0);
			}else if( AttrName == "id" )
			{
				MIntArray IA;
				PS.particleIds( IA );
				Array RawArray( new char[sizeof(int)*Count] );
				int* p = (int*)RawArray.get();
				for( unsigned int a=0; a<Count; ++a )
				{
					p[a] = IA[a];
				}
				PRTFile->AddChannel("ID",kINT32,1,RawArray);
				IA.setLength(0);
			}else if( AttrName == "age" )
			{
				MDoubleArray DA;
				PS.age( DA );

				Array RawArray( new char[sizeof(float)*Count] );
				float* p = (float*)RawArray.get();
				for( unsigned int a=0; a<Count; ++a )
				{
					p[a] = (float)DA[a];
				}
				PRTFile->AddChannel("Age",kFLOAT32,1,RawArray);

				DA.setLength(0);
			}else if( AttrName == "opacity" )
			{
				MDoubleArray DA;
				PS.opacity( DA );

				Array RawArray( new char[sizeof(float)*Count] );
				float* p = (float*)RawArray.get();
				for( unsigned int a=0; a<Count; ++a )
				{
					p[a] = (float)DA[a];
				}
				PRTFile->AddChannel("Opacity",kFLOAT32,1,RawArray);

				DA.setLength(0);
			}else if( AttrName ==  "color" )
			{
			}
			else
			{
				//User's self-defined attributes.
				istringstream ISS( AttrName );
				int Arity = 0;
				string RealName, Type;
				ISS>>RealName>>Arity>>Type;

				if( RealName.size() == 0 || Arity == 0 || Type.size() != 1 )
					continue;
				
				MStatus S;

				if( Arity == 1 )
				{
					if( Type == "i" )
					{
						MIntArray IA;
						
						PS.getPerParticleAttribute( MString(RealName.c_str()),IA,&S);
						if( S == MStatus::kSuccess )
						{
							Array RawArray( new char[sizeof(int)*Count] );
							int* p = (int*)RawArray.get();
							for( unsigned int a=0; a<Count; ++a )
							{
								p[a] = IA[a];
							}
							PRTFile->AddChannel(RealName,kINT32,1,RawArray);
							IA.setLength(0);
						}

						IA.setLength(0);
					}else if( Type == "f" )
					{
						MDoubleArray DA;

						PS.getPerParticleAttribute( MString(RealName.c_str()), DA, &S );
						if( S == MStatus::kSuccess )
						{
							cout<<"M2K got ["<<RealName<<"]"<<endl;
							Array RawArray( new char[sizeof(float)*Count] );
							float* p = (float*)RawArray.get();
							for( unsigned int a=0; a<Count; ++a )
							{
								p[a] = (float)DA[a];
							}
							PRTFile->AddChannel(RealName,kFLOAT32,1,RawArray);
							DA.setLength(0);
						}
						
						DA.setLength(0);
					}
				}else if( Arity == 3 )
				{
					MVectorArray VA;
					PS.getPerParticleAttribute( MString(RealName.c_str()), VA );
					if( S == MStatus::kSuccess )
					{
						Array RawArray( new char[sizeof(float)*Count*3] );
						float* p = (float*)RawArray.get();
						for( unsigned int a=0; a<Count; ++a )
						{
							unsigned int b = a*3;
							MVector P = VA[a];
							p[b+0] = (float)P.x;
							p[b+1] = (float)P.y;
							p[b+2] = (float)P.z;
						}
						PRTFile->AddChannel(RealName,kFLOAT32,3,RawArray);
					}
					VA.setLength(0);
				}
			}
		}

		PRTFile->SaveToFile( Path.asChar() );

	}
	catch(const exception& e)
	{
		MString Info("M2K caught an C++ exception : ");
		Info += e.what();
		MGlobal::displayError(Info);
		return MStatus::kFailure;
	}
	catch(...)
	{
		MGlobal::displayError("M2K caught an UNKNOWN exception");
		return MStatus::kFailure;
	}

	return MStatus::kSuccess;
}

#ifdef WIN32
#define M2K_EXPORT __declspec(dllexport)
#else
#define M2K_EXPORT
#endif

MStatus M2K_EXPORT initializePlugin( MObject O )
{
	MFnPlugin plugin( O, "OpenSource ", "Any", "Any");
	MStatus status = plugin.registerCommand("m2k",M2K::creator );
	if (!status) 
		status.perror("registerCommand");
	return status;
}

MStatus M2K_EXPORT uninitializePlugin( MObject O )
{
	MFnPlugin plugin( O );
	MStatus status = plugin.deregisterCommand("m2k");
	if (!status) 
		status.perror("deregisterCommand");
	return status;
}