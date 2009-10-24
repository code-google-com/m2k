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

#ifndef PR_H
#define PR_H

#include <memory>
#include <string>
#include <RifPlugin.h>

/**
 * \brief Simple vector class
 * \warning there is already a class float3 in CUDA
 */
struct float3
{
	float3() : x(0.0f), y(0.0f), z(0.0f)
	{
	}
	~float3()
	{
	}
	float x,y,z;
};

/**
 * \brief Interface for variable implemenation
 */
class ParticleResolver
{
public:

	/**
	 * Pure virtual function for further implementation. It's the same as \b RiPointsV.
	 * You can get more information from <a href="https://renderman.pixar.com/products/rispec/rispec_pdf/RISpec3_2.pdf">RenderMan Interface Specification</a>.
	 */
	virtual RtVoid DoIt(RtInt, RtInt, RtToken [], RtPointer []) = 0;
};

/**
 * \brief "Origin"
 */
class OriginParticleResolver : public ParticleResolver
{
public:
	OriginParticleResolver();
	~OriginParticleResolver();

	RtVoid DoIt(RtInt, RtInt, RtToken [], RtPointer []);
};

/**
 * \brief "Diffusion"
 *
 * This kind of ParticleResolver clones more points from orgin seed points according your parameters in RIB like following,
 * \code
 * Attribute "user" "uniform int PRType" [ 0 ]
 * Attribute "user" "uniform int NCopies" [ 256 ]
 * Attribute "user" "uniform int RandPattern" [2]
 * \endcode
 * There are several attributes for this,
 * - \b PRType means which type of ParticleSolver you want to assign to current particle shape, "0" = OriginParticleResolver, "1" = DiffusionParticleResolver.
 * - \b NCopies means the count of cloned particles from one seed particle.
 * - \b RandPattern means different random generator would be used to generate the spherical points set, "0" = rand, "1" = drand48, "2" = sobol.
 */
class DiffusionParticleResolver : public ParticleResolver
{
public:
	DiffusionParticleResolver();
	~DiffusionParticleResolver();

	void SetRandPattern(const RtInt&);
	void SetNCopies(const RtInt&);
	void SetFalloff(const RtInt&);

	/**
	 * \todo
	 * Current version use <a href="http://openmp.org/">OpenMP</a> to utilize your CPU as many as possible by \b omp_get_num_procs.
	 * But it's still slow because it performs k-nearest-neighbor lookup at each cloned point.
	 */
	RtVoid DoIt(RtInt, RtInt, RtToken [], RtPointer []);
private:
	RtInt mFalloff;
	RtInt mNCopies;
	RtInt mRandPattern;
};

/**
 * \brief "Polygon"
 *
 * It will
 * - Apply(or not) subdivision on an Wavefront OBJ mesh.
 * - Sample primitives to generate a initialized distribution to represent the geometry.
 * - Convert the points into a volume to generate a tent field.
 * - Solve out level set of the volume.
 * - Fill each inner voxel and generate more points.
 */
class PolygonParticleResolver : public ParticleResolver
{
public:
	PolygonParticleResolver();
	~PolygonParticleResolver();
	
	void SetPath(const char*);
	void SetSubdLevel(const int&);
	
	/**
	 * \todo
	 * Have not been implemented yet.
	 */
	RtVoid DoIt(RtInt, RtInt, RtToken [], RtPointer []);
private:
	int mSubdLevel;
	std::string mPath;
};

/**
 * \brief "External"
 *
 * It would load an external procedural primitive DSO to do more.
 */
class ExternalParticleResolver : public ParticleResolver
{
public:
	ExternalParticleResolver();
	~ExternalParticleResolver();

	void SetPath(const char*);

	/**
	 * \todo
	 * Have not been implemented yet.
	 */
	RtVoid DoIt(RtInt, RtInt, RtToken [], RtPointer []);
private:
	std::string mPath;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * \brief Instanced by RenderMan
 */
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
