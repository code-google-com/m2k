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
// Only support to interpolate scalar now

#define _USE_MATH_DEFINES
#include <math.h>
#include <time.h>

#include <iostream>
#include <map>
#include <vector>

#include <omp.h>

#include <boost/function.hpp>
#include <boost/scoped_array.hpp>
#include <boost/progress.hpp>

#include <gsl/gsl_rng.h>
#include <gsl/gsl_qrng.h>

#include <ri.h>

#include <sfcnn.hpp>
#include <dpoint.hpp>

#include "PR.h"

using namespace std;

#define POSITION "P"
#define CONSTANTWIDTH "constantwidth"
#define COLOR "Cs"
#define OPACITY "Os"

static float ConstWidth = 0.16666666f;
static RtColor ConstColor = {1.0f, 0.0f, 0.0f};
static RtColor ConstOpacity = {1.0f, 1.0f, 1.0f};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef reviver::dpoint<float,3> ANNPoint;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

inline float RadToDeg(const float R)
{
	return R * 180.0 / M_PI;
}

inline float DegToRad(const float D)
{
	return D * M_PI / 180.0;
}

void CubeToSphere(float3& P)
{
	float Theta = P.x * M_PI * 2.0;
	float U = P.y * 2.0f - 1.0f;
	
	P.x = cosf(Theta) * sqrtf( 1.0f - U*U );
	P.y = sinf(Theta) * sqrtf( 1.0f - U*U );
	P.z = U;
}

void CartesianCoordToSphericalCoord(const float3& XYZ, float3& RTP)
{
	RTP.x = sqrtf( XYZ.x*XYZ.x + XYZ.y*XYZ.y + XYZ.z*XYZ.z );
	if( XYZ.x > 0 )
	{
		RTP.y = atanf( XYZ.y/XYZ.x ) + M_PI;
	}else if( fabsf(XYZ.x) < FLT_EPSILON )
	{
		if( XYZ.y > 0.0f )
		{
			RTP.y = M_PI_2;
		}else
		{
			RTP.y = -M_PI_2;
		}
	}else
	{
		RTP.y = atanf( XYZ.y/XYZ.x );
	}
	RTP.z = acosf( XYZ.z/RTP.x );
}

void SphericalCoordToCartesianCoord(const float3& RTP, float3& XYZ)
{
	XYZ.x = RTP.x * cosf(RTP.y) * sinf(RTP.z);
	XYZ.y = RTP.x * sinf(RTP.y) * sinf(RTP.z);
	XYZ.z = RTP.x * cosf(RTP.z);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef boost::function2<void, const int, float3*> DistFunc;

struct DefaultRand
{
	void operator()(const int N, float3* P) const
	{
		srand(time(0));
		for( int i=0; i<N; ++i )
		{
			P[i].x = (float)rand() / (float)RAND_MAX;
			P[i].y = (float)rand() / (float)RAND_MAX;
			P[i].z = (float)rand() / (float)RAND_MAX;
		}
	}
};

struct DRAND48Rand
{
	void operator()(const int N, float3* P) const
	{
		gsl_rng* RNG = gsl_rng_alloc(gsl_rng_rand48);
		gsl_rng_set(RNG,time(0));
		for( int i=0; i<N; ++i )
		{
			P[i].x = (float)gsl_rng_uniform(RNG);
			P[i].y = (float)gsl_rng_uniform(RNG);
			P[i].z = (float)gsl_rng_uniform(RNG);
		}
		gsl_rng_free(RNG);
	}
};

struct SobolQRand
{
	void operator()(const int N, float3* P) const
	{
		gsl_qrng* QRNG = gsl_qrng_alloc(gsl_qrng_sobol, 2);
		double Result[2];
		for( int i=0; i<N; ++i )
		{
			gsl_qrng_get(QRNG,Result);
			P[i].x = (float)Result[0];
			P[i].y = (float)Result[1];
			P[i].z = 0.0f;
		}
		gsl_qrng_free(QRNG);
	}
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

DiffusionParticleResolver::DiffusionParticleResolver() : mRandPattern(0), mNCopies(0), mFalloff(0)
{
}

DiffusionParticleResolver::~DiffusionParticleResolver()
{
}

RtVoid DiffusionParticleResolver::DoIt(RtInt NVerts, RtInt N, RtToken Tokens[], RtPointer Data[])
{
	const int NCopies = mNCopies;
	//const int Falloff = mFalloff;
	const int RandPattern = mRandPattern;
	// We do not need to do anything.
	if( NCopies == 0 )
	{
		RiPointsV(NVerts,N,Tokens,Data);
		return;
	}

	bool FindPosition = false;
	bool FindConstWidth = false;
	bool FindColor = false;
	bool FindOpacity = false;

	RtPoint* PositionPointer = NULL;
	RtFloat* ConstWidthPointer = NULL;
	RtColor* CsPointer = NULL;
	RtColor* OsPointer = NULL;
	int N2 = 0;
	for( int i=0; i<N; ++i )
	{
		if( strcmp(Tokens[i],POSITION) == 0 )
		{
			cout<<"ParticleResolverPlugin : DiffusionParticleResolver Found Attribute ["<<POSITION<<"]"<<endl;
			PositionPointer = (RtPoint*)Data[i];

			FindPosition = true;
		}else if( strcmp(Tokens[i],CONSTANTWIDTH) == 0 )
		{
			cout<<"ParticleResolverPlugin : DiffusionParticleResolver Found Attribute ["<<CONSTANTWIDTH<<"]"<<endl;
			ConstWidthPointer = (RtFloat*)Data[i];

			FindConstWidth = true;
			N2++;
		}else if( strcmp(Tokens[i],COLOR) == 0 )
		{
			cout<<"ParticleResolverPlugin : DiffusionParticleResolver Found Attribute ["<<COLOR<<"]"<<endl;
			CsPointer = (RtColor*)Data[i];

			FindColor = true;
		}else if( strcmp(Tokens[i],OPACITY) == 0 )
		{
			cout<<"ParticleResolverPlugin : DiffusionParticleResolver Found Attribute ["<<OPACITY<<"]"<<endl;
			OsPointer = (RtColor*)Data[i];

			FindOpacity = true;
		}
	}

	try
	{
		boost::scoped_array<float3> ClonePoints( new float3[NCopies] );
		boost::scoped_array<float> CloneWidths( new float[NCopies] );
		boost::scoped_array<float3> CloneColors( new float3[NCopies] );
		boost::scoped_array<float3> CloneOpacities( new float3[NCopies] );

		// Prepare the four inner particle attributes.
		RtToken Tokens2[4] = {POSITION,"varying float width",COLOR,OPACITY};
		RtPointer Data2[4] = {ClonePoints.get(),CloneWidths.get(),CloneColors.get(),CloneOpacities.get()};

		// Make a random points cluster
		boost::scoped_array<float3> TemplatePoints( new float3[NCopies] );
		DistFunc DF;
		switch( RandPattern )
		{
		case 1:
			DF = DRAND48Rand();
			cout<<"ParticleResolverPlugin : DiffusionParticleResolver Using [drand48] RNG"<<endl;
			break;
		case 2:
			DF = SobolQRand();
			cout<<"ParticleResolverPlugin : DiffusionParticleResolver Using [Sobol] QRNG"<<endl;
			break;
		default:
			DF = DefaultRand();
			cout<<"ParticleResolverPlugin : DiffusionParticleResolver Using [rand] RNG"<<endl;
			break;
		}

		DF(NCopies,TemplatePoints.get());

		// Make 4 attributes
		int i = 0;

		int NumOfProcess = omp_get_num_procs();
		cout<<"ParticleResolverPlugin : DiffusionParticleResolver Found ["<<NumOfProcess<<"] Processors"<<endl;

#pragma omp parallel num_threads( NumOfProcess )
		{
#pragma omp for private(i)
			for( i=0; i<NCopies; ++i )
			{
				// Varing P
				float3 XYZ,RTP;
				XYZ = TemplatePoints[i];
				CubeToSphere(XYZ);
				CartesianCoordToSphericalCoord(XYZ,RTP);
				RTP.x = sqrtf( (float)rand() / (float)RAND_MAX );
				//SphericalCoordToCartesianCoord(RTP,XYZ);
				TemplatePoints[i] = RTP;
				
				//
				CloneWidths[i] = ConstWidthPointer[0];

				
				if( FindOpacity )
				{
					CloneOpacities[i].x = OsPointer[i][0];
					CloneOpacities[i].y = OsPointer[i][1];
					CloneOpacities[i].z = OsPointer[i][2];
				}else
				{
					CloneOpacities[i].x = 1.0f;
					CloneOpacities[i].y = 1.0f;
					CloneOpacities[i].z = 1.0f;
				}
			}

		}

		boost::scoped_array<ANNPoint> KNNPoints( new ANNPoint[NVerts] );
		for( int i=0; i<NVerts; ++i )
		{
			KNNPoints[i][0] = PositionPointer[i][0];
			KNNPoints[i][1] = PositionPointer[i][1];
			KNNPoints[i][2] = PositionPointer[i][2];
		}
		sfcnn<ANNPoint,3,float> KNN( KNNPoints.get(), NVerts, 2 );

		// Render them !
		float3* SeedPosition = (float3*)PositionPointer;

		boost::progress_display Progress(NVerts);

		boost::progress_timer Timer;
		for( int i=0; i<NVerts; ++i )
		{
			const float3 CurrSeed = SeedPosition[i];

			// Varying ATTRIBUTES ! 
			// Color Only NOW !
#pragma omp parallel num_threads(NumOfProcess) shared(i)
			{
			int j = 0;
			
#pragma omp for private(j)
			for( j=0; j<NCopies; ++j )
			{
				float3 LookupPos;
				SphericalCoordToCartesianCoord(TemplatePoints[j],LookupPos); 

				LookupPos.x += CurrSeed.x;
				LookupPos.y += CurrSeed.y;
				LookupPos.z += CurrSeed.z;
				const int k = 128;
				vector<long unsigned int> NNIdx;
				vector<double> NNDist;
				KNN.ksearch( ANNPoint(LookupPos.x,LookupPos.y,LookupPos.z), k, NNIdx, NNDist );

				float TotalWeight = 0.0f;
				for( int a=0; a<k; ++a )
				{
					const float W = NNDist[a];
					TotalWeight += W;

					CloneColors[j].x += CsPointer[i][0]*W;
					CloneColors[j].y += CsPointer[i][1]*W;
					CloneColors[j].z += CsPointer[i][2]*W;
				}

				CloneColors[j].x /= TotalWeight;
				CloneColors[j].y /= TotalWeight;
				CloneColors[j].z /= TotalWeight;

				float3 RealPos;
				SphericalCoordToCartesianCoord(TemplatePoints[j],RealPos);
				ClonePoints[j] = RealPos;
			}
			}

			RiTransformBegin();
			RiTranslate(CurrSeed.x,CurrSeed.y,CurrSeed.z);
			RiPointsV(NCopies,4,Tokens2,Data2);
			RiTransformEnd();

			++Progress;
		}
	}catch(const exception& e)
	{
		cerr<<"ParticleResolverPlugin : DiffusionParticleResolver Catch C++ Exception ["<<e.what()<<"]"<<endl;
	}
}

void DiffusionParticleResolver::SetNCopies(const RtInt& NCopies)
{
	mNCopies = NCopies;
}

void DiffusionParticleResolver::SetRandPattern(const RtInt& RandPattern)
{
	mRandPattern = RandPattern;
}

void DiffusionParticleResolver::SetFalloff(const RtInt& Falloff)
{
	mFalloff = Falloff;
}
