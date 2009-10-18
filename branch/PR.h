#ifndef PR_H
#define PR_H

#include <memory>
#include <string>

#include <RifPlugin.h>

class ParticleResolver
{
public:
	virtual RtVoid DoIt(RtInt, RtInt, RtToken [], RtPointer []) = 0;
};

class OriginParticleResolver : public ParticleResolver
{
public:
	OriginParticleResolver();
	~OriginParticleResolver();

	RtVoid DoIt(RtInt, RtInt, RtToken [], RtPointer []);
};

class DiffusionParticleResolver : public ParticleResolver
{
public:
	DiffusionParticleResolver();
	~DiffusionParticleResolver();

	void SetRandPattern(const RtInt&);
	void SetNCopies(const RtInt&);
	void SetFalloff(const RtInt&);

	RtVoid DoIt(RtInt, RtInt, RtToken [], RtPointer []);
private:
	RtInt mFalloff;
	RtInt mNCopies;
	RtInt mRandPattern;
};

class ExternalParticleResolver : public ParticleResolver
{
public:
	ExternalParticleResolver();
	~ExternalParticleResolver();

	void SetPath(const char*);

	RtVoid DoIt(RtInt, RtInt, RtToken [], RtPointer []);
private:
	std::string mPath;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////

class ParticleResolverPlugin : public RifPlugin
{
public:
	static std::auto_ptr<ParticleResolver> Instance;

	static RtVoid AttributeV(RtToken, RtInt, RtToken [], RtPointer []);
	static RtVoid PointsV(RtInt, RtInt, RtToken [], RtPointer []);
	
	ParticleResolverPlugin(int, char**);
	virtual ~ParticleResolverPlugin();

	RifFilter& GetFilter();
private:
	RifFilter mRF;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////

#endif
