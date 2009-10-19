#define _USE_MATH_DEFINES
#include <math.h>
#include <time.h>

#include <iostream>

#include <boost/function.hpp>
#include <boost/scoped_array.hpp>

#include <gsl/gsl_rng.h>
#include <gsl/gsl_qrng.h>

#include <ri.h>

#include "PR.h"

using namespace std;

#define POSITION "P"
#define CONSTANTWIDTH "constantwidth"
#define COLOR "Cs"
#define OPACITY "Os"

static float ConstWidth = 0.16666666f;
static RtColor ConstColor = {1.0f, 0.0f, 0.0f};
static RtColor ConstOpacity = {1.0f, 1.0f, 1.0f};

struct float3
{
	float x,y,z;
};

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

	bool FoundPosition = false, FoundConstWidth = false, FoundColor = false, FoundOpacity = false;
	RtPoint* PositionPointer = NULL;
	RtFloat* ConstWidthPointer = NULL;
	RtColor* CsPointer = NULL;
	RtColor* OsPointer = NULL;
	for( int i=0; i<N; ++i )
	{
		if( strcmp(Tokens[i],POSITION) == 0 )
		{
			cout<<"ParticleResolverPlugin : DiffusionParticleResolver Found Attribute ["<<POSITION<<"]"<<endl;
			FoundPosition = true;
			PositionPointer = (RtPoint*)Data[i];
		}else if( strcmp(Tokens[i],CONSTANTWIDTH) == 0 )
		{
			cout<<"ParticleResolverPlugin : DiffusionParticleResolver Found Attribute ["<<CONSTANTWIDTH<<"]"<<endl;
			FoundConstWidth = true;
			ConstWidthPointer = (RtFloat*)Data[i];
		}else if( strcmp(Tokens[i],COLOR) == 0 )
		{
			cout<<"ParticleResolverPlugin : DiffusionParticleResolver Found Attribute ["<<COLOR<<"]"<<endl;
			FoundColor = true;
			CsPointer = (RtColor*)Data[i];
		}else if( strcmp(Tokens[i],OPACITY) == 0 )
		{
			cout<<"ParticleResolverPlugin : DiffusionParticleResolver Found Attribute ["<<OPACITY<<"]"<<endl;
			FoundOpacity = true;
			OsPointer = (RtColor*)Data[i];
		}
	}

	try
	{
		float3* TemplatePoints = new float3[NCopies];

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

		DF(NCopies,TemplatePoints);

		for( int i=0; i<NCopies; ++i )
		{
			float3 XYZ,RTP;
			XYZ = TemplatePoints[i];
			CubeToSphere(XYZ);
			CartesianCoordToSphericalCoord(XYZ,RTP);
			RTP.x = powf( (float)rand() / (float)RAND_MAX, 0.5f );
			SphericalCoordToCartesianCoord(RTP,XYZ);
			TemplatePoints[i] = XYZ;
		}

		// Prepare the four inner particle attributes.
		RtToken Tokens2[4] = {NULL,NULL,NULL,NULL};
		RtPointer Data2[4] = {NULL,NULL,NULL,NULL};

		Tokens2[0] = POSITION;
		Data2[0] = TemplatePoints;

		int N2 = 1;

		if( FoundConstWidth )
		{
			Tokens2[N2] = CONSTANTWIDTH;
			Data2[N2] = ConstWidthPointer;
			++N2;
		}else
		{
			Tokens[N2] = CONSTANTWIDTH;
			Data2[N2] = &ConstWidth;
			++N2;
		}
		

		if( FoundColor )
		{
			Tokens2[N2] = COLOR;
			Data2[N2] = CsPointer;
			++N2;
		}else
		{
			Tokens2[N2] = COLOR;
			Data2[N2] = &ConstColor[0];
			++N2;
		}
		

		if( FoundOpacity )
		{
			Tokens2[N2] = OPACITY;
			Data2[N2] = OsPointer;
			++N2;
		}else
		{
			Tokens2[N2] = OPACITY;
			Data2[N2] = &ConstOpacity[0];
			++N2;
		}

		// Render them !
		float3* SeedPosition = (float3*)PositionPointer;
		for( int i=0; i<NVerts; ++i )
		{
			RiTransformBegin();
			RiTranslate(SeedPosition[i].x,SeedPosition[i].y,SeedPosition[i].z);
			RiPointsV(NCopies,N2,Tokens2,Data2);
			RiTransformEnd();
		}

		delete [] TemplatePoints;
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
