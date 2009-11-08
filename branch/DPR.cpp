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

#ifdef ENABLE_OPENMP
#include <omp.h>
#endif

#include <boost/function.hpp>
#include <boost/scoped_array.hpp>
#include <boost/progress.hpp>

#include <gsl/gsl_rng.h>
#include <gsl/gsl_qrng.h>

#include <ri.h>

#include <sfcnn.hpp>
#include <dpoint.hpp>

#include "DPR.h"

using namespace std;
using namespace PR;

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

void CubeToSphere(Float3& P)
{
	float Theta = P.x * M_PI * 2.0;
	float U = P.y * 2.0f - 1.0f;
	
	P.x = cosf(Theta) * sqrtf( 1.0f - U*U );
	P.y = sinf(Theta) * sqrtf( 1.0f - U*U );
	P.z = U;
}

void CartesianCoordToSphericalCoord(const Float3& XYZ, Float3& RTP)
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

void SphericalCoordToCartesianCoord(const Float3& RTP, Float3& XYZ)
{
	XYZ.x = RTP.x * cosf(RTP.y) * sinf(RTP.z);
	XYZ.y = RTP.x * sinf(RTP.y) * sinf(RTP.z);
	XYZ.z = RTP.x * cosf(RTP.z);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef boost::function3<void, const int, const int, Float3*> DistFunc;

struct DefaultRand
{
	void operator()(const int S, const int N, Float3* P) const
	{
		srand(S);
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
	void operator()(const int S, const int N, Float3* P) const
	{
		gsl_rng* RNG = gsl_rng_alloc(gsl_rng_rand48);
		gsl_rng_set(RNG,S);
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
	void operator()(const int S, const int N, Float3* P) const
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

DiffusionParticleResolver::DiffusionParticleResolver() : mRandPattern(0), mNCopies(0), mFalloff(-1.0f), mSeed(0x12345678), mCoarseMode(1)
{
}

DiffusionParticleResolver::~DiffusionParticleResolver()
{
}

RtVoid DiffusionParticleResolver::DoIt(RtInt NVerts, RtInt N, RtToken Tokens[], RtPointer Data[])
{
	const int NCopies = mNCopies;
	const int Falloff = mFalloff;
	const int RandPattern = mRandPattern;
	const int Seed = mSeed;
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
		boost::scoped_array<Float3> ClonePoints( new Float3[NCopies] );
		boost::scoped_array<float> CloneWidths( new float[NCopies] );
		boost::scoped_array<Float3> CloneColors( new Float3[NCopies] );
		boost::scoped_array<Float3> CloneOpacities( new Float3[NCopies] );

		// Prepare the four inner particle attributes.
		RtToken Tokens2[4] = {POSITION,"width",COLOR,OPACITY};
		RtPointer Data2[4] = {ClonePoints.get(),CloneWidths.get(),CloneColors.get(),CloneOpacities.get()};

		// Make a random points cluster
		boost::scoped_array<Float3> TemplatePoints( new Float3[NCopies] );
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

		DF(Seed,NCopies,TemplatePoints.get());
		cout<<"ParticleResolverPlugin : DiffusionParticleResolver Use ["<<Seed<<"] Random Seed"<<endl;

		// Make 4 attributes
		int i = 0;
		int NumOfProcess = 1;
#ifdef ENABLE_OPENMP
		NumOfProcess = omp_get_num_procs();
#endif
		cout<<"ParticleResolverPlugin : DiffusionParticleResolver Found ["<<NumOfProcess<<"] Processors"<<endl;

#pragma omp parallel num_threads( NumOfProcess )
		{
#pragma omp for private(i)
			for( i=0; i<NCopies; ++i )
			{
				// Varing P
				Float3 XYZ,RTP;
				XYZ = TemplatePoints[i];
				CubeToSphere(XYZ);
				CartesianCoordToSphericalCoord(XYZ,RTP);
				RTP.x = sqrtf( (float)rand() / (float)RAND_MAX );
				//SphericalCoordToCartesianCoord(RTP,XYZ);
				TemplatePoints[i] = RTP;

				
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

		// Prepare for kNN lookup
		// SeedNNIdx & SeedNNDist are used to store coarse KNN result.
		const int k = 8;
		boost::scoped_array<ANNPoint> KNNPoints( new ANNPoint[NVerts] );
		for( int i=0; i<NVerts; ++i )
		{
			KNNPoints[i][0] = PositionPointer[i][0];
			KNNPoints[i][1] = PositionPointer[i][1];
			KNNPoints[i][2] = PositionPointer[i][2];
		}
		sfcnn<ANNPoint,3,float> KNN( KNNPoints.get(), NVerts, 2 );
		boost::scoped_array<long unsigned int> SeedNNIdx;
		boost::scoped_array<double> SeedNNDist;

		bool UseCoarseLookup = true;

		if( UseCoarseLookup )
		{
			SeedNNIdx.reset( new long unsigned int[NVerts*k] );
			SeedNNDist.reset( new double[NVerts*k] );
#pragma omp parallel num_threads( NumOfProcess )
			{
#pragma omp for private(i)
				for( i=0; i<NVerts; ++i )
				{
					vector<long unsigned int> IdxVec;
					vector<double> DistVec;
					KNN.ksearch( KNNPoints[i], k, IdxVec, DistVec);

					copy( IdxVec.begin(), IdxVec.end(), SeedNNIdx.get()+i*k );
					copy( DistVec.begin(), DistVec.end(), SeedNNDist.get()+i*k );
				}
			}
		}

		// Render them !
		Float3* SeedPosition = (Float3*)PositionPointer;

		boost::progress_display Progress(NVerts);

		boost::progress_timer Timer;
		for( int i=0; i<NVerts; ++i )
		{
			const Float3 CurrSeed = SeedPosition[i];

			// Varying ATTRIBUTES ! 
			// Color Only NOW !
#pragma omp parallel num_threads(NumOfProcess) shared(i)
			{
				int j = 0;
				
			
#pragma omp for private(j)
				for( j=0; j<NCopies; ++j )
				{
					Float3 LookupPos;
					SphericalCoordToCartesianCoord(TemplatePoints[j],LookupPos); 

					LookupPos.x += CurrSeed.x;
					LookupPos.y += CurrSeed.y;
					LookupPos.z += CurrSeed.z;

					if( UseCoarseLookup )
					{

						float BlobbyR = SeedNNDist[i*k+k-1];
						float Red = 0.0f, Green = 0.0f, Blue = 0.0f;
						float TotalWeight = 0.0f;
						for( int a = 0; a<k-1; ++a )
						{
							float r = SeedNNDist[i*k+a];

							float rdivR = r/BlobbyR;
							float W_h = -4.0f/9.0f*powf(rdivR,6.0f) + 17.0f/9.0f*powf(rdivR,4.0f) - 22.0f/9.0f*pow(rdivR,2.0f) + 1.0f;
							TotalWeight += W_h;

							const long unsigned int Idx = SeedNNIdx[i*k+a];

							Red += W_h * CsPointer[Idx][0];
							Green += W_h * CsPointer[Idx][1];
							Blue += W_h * CsPointer[Idx][2];
						}

						TotalWeight = 1.0f;

						CloneColors[j].x = Red / TotalWeight;
						CloneColors[j].y = Green / TotalWeight;
						CloneColors[j].z = Blue / TotalWeight;

						Float3 RealPos = TemplatePoints[j];
						float UnitRadius = TemplatePoints[j].x;
						RealPos.x = UnitRadius*SeedNNDist[i*k+k-3]*1.5f;
						Float3 RealPos1;

						if( Falloff < 0.0f )
							CloneWidths[j] = ConstWidthPointer[0]*0.1f;
						else
							CloneWidths[j] = ConstWidthPointer[0]*powf(1.001f - UnitRadius,Falloff);

						SphericalCoordToCartesianCoord(RealPos,RealPos1);
						ClonePoints[j] = RealPos1;

					}else
					{
						vector<long unsigned int> NNIdx;
						vector<double> NNDist;
						KNN.ksearch( ANNPoint(LookupPos.x,LookupPos.y,LookupPos.z), k, NNIdx, NNDist);

						// Blobby Interpolation
						// BEGIN
						float BlobbyR = NNDist[k-1];
						float Red = 0.0f, Green = 0.0f, Blue = 0.0f;
						for( int a = 0; a<k; ++a )
						{
							float r = NNDist[a];

							float rdivR = r/BlobbyR;
							float W_h = -4.0f/9.0f*powf(rdivR,6.0f) + 17.0f/9.0f*powf(rdivR,4.0f) - 22.0f/9.0f*pow(rdivR,2.0f) + 1.0f;

							const long unsigned int Idx = NNIdx[a];

							Red += W_h * CsPointer[Idx][0];
							Green += W_h * CsPointer[Idx][1];
							Blue += W_h * CsPointer[Idx][2];
						}
						CloneColors[j].x = Red;
						CloneColors[j].y = Green;
						CloneColors[j].z = Blue;

						float UnitRadius = TemplatePoints[j].x;
						Float3 RealPos = TemplatePoints[j];
						RealPos.x *= NNDist[1];
						Float3 RealPos1;

						if( mFalloff < 0.0f )
							CloneWidths[j] = ConstWidthPointer[0]*0.5f;
						else
							CloneWidths[j] = ConstWidthPointer[0]*powf(1.001f - UnitRadius,Falloff);

						SphericalCoordToCartesianCoord(RealPos,RealPos1);
						ClonePoints[j] = RealPos1;
					}
					//// END

					//// Inverse Distance Interpolation
					//// BEGIN
					//float TotalWeight = 0.0f;
					//for( int a=0; a<k; ++a )
					//{
					//	const long unsigned int Idx = NNIdx[a];
					//	const float W = NNDist[a];
					//	TotalWeight += W;
					//	CloneColors[j].x += CsPointer[Idx][0]*W;
					//	CloneColors[j].y += CsPointer[Idx][1]*W;
					//	CloneColors[j].z += CsPointer[Idx][2]*W;
					//}

					//CloneColors[j].x /= TotalWeight;
					//CloneColors[j].y /= TotalWeight;
					//CloneColors[j].z /= TotalWeight;

					//CloneColors[j].x *= 2.0f;
					//CloneColors[j].y *= 2.0f;
					//CloneColors[j].z *= 2.0f;
					//// END

				}
			}

			RiTransformBegin();
			RiTranslate(CurrSeed.x,CurrSeed.y,CurrSeed.z);
			//RiScale(Scale,Scale,Scale);
			RiPointsV(NCopies,4,Tokens2,Data2);
			RiTransformEnd();

			++Progress;
		}
	}catch(const exception& e)
	{
		cerr<<"ParticleResolverPlugin : DiffusionParticleResolver Catch C++ Exception ["<<e.what()<<"]"<<endl;
	}
}

void DiffusionParticleResolver::SetFalloff(const RtFloat& Falloff)
{
	mFalloff = Falloff;
}

void DiffusionParticleResolver::SetNCopies(const RtInt& NCopies)
{
	mNCopies = NCopies;
}

void DiffusionParticleResolver::SetRandPattern(const RtInt& RandPattern)
{
	mRandPattern = RandPattern;
}

void DiffusionParticleResolver::SetSeed(const RtInt& Seed)
{
	mSeed = Seed;
}

void DiffusionParticleResolver::SetCoarseMode(const RtInt& CoarseMode)
{
	mCoarseMode = CoarseMode;
}
