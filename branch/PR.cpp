#include <iostream>
#include <memory>

#include "PR.h"

using namespace std;

#define PRTYPE "PRType"
#define NCOPIES "NCopies"

RifPlugin* RifPluginManufacture(int argc, char **argv)
{
	return new ParticleResolverPlugin(argc,argv);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////

auto_ptr<ParticleResolver> ParticleResolverPlugin::Instance;

RtVoid ParticleResolverPlugin::AttributeV(RtToken Name, RtInt N, RtToken Tokens[], RtPointer Data[])
{
	if( strcmp("user",Name) == 0 )
	{
		for( int i=0; i<N; ++i )
		{
			if( strstr(Tokens[i],PRTYPE) )
			{
				int* p = (int*)Data[0];
				switch( *p )
				{
				case 0:
					cout<<"ParticleResolverPlugin : Using OriginParticleResolver"<<endl;
					Instance.reset( new OriginParticleResolver );
					break;
				case 1:
					cout<<"ParticleResolverPlugin : Using DiffusionParticleResolver"<<endl;
					Instance.reset( new DiffusionParticleResolver );
					break;
				}
			}else if( strstr(Tokens[i],NCOPIES) )
			{
				int NCopies = *(int*)Data[0];
				DiffusionParticleResolver* p = dynamic_cast<DiffusionParticleResolver*>( Instance.get() );
				if( p )
				{
					p->SetNCopies(NCopies);
					cout<<"ParticleResolverPlugin : NCopies ["<< NCopies <<"]"<<endl;
				}else
				{
					cerr<<"ParticleResolverPlugin : Only DiffusionParticleResolver Use ["<<NCOPIES<<"]"<<endl;
				}
			}
		}
	}
	RiAttributeV(Name,N,Tokens,Data);
}

RtVoid ParticleResolverPlugin::PointsV(RtInt NVerts, RtInt N, RtToken Tokens[], RtPointer Data[])
{
	Instance->DoIt(NVerts,N,Tokens,Data);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////

ParticleResolverPlugin::ParticleResolverPlugin(int argc, char** argv)
{
	mRF.AttributeV = ParticleResolverPlugin::AttributeV;
	mRF.PointsV = ParticleResolverPlugin::PointsV;

	Instance.reset( new OriginParticleResolver );
}

ParticleResolverPlugin::~ParticleResolverPlugin()
{
	Instance.reset();
}

RifFilter& ParticleResolverPlugin::GetFilter()
{
	return mRF;
}
