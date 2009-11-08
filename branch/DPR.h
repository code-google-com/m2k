#include "PR.h"

NAMESPACE_PR_BEGIN

/**
 * \brief "Diffusion"
 *
 * This kind of ParticleResolver clones more points from orgin seed points according your parameters in RIB like following,
 * \code
 * Attribute "user" "uniform int PRType" [ 0 ]
 * Attribute "user" "uniform int NCopies" [ 256 ]
 * Attribute "user" "uniform int RandPattern" [2]
 * Attribute "user" "uniform float Falloff" [1.0]
 * \endcode
 * There are several attributes for this,
 * - \b PRType means which type of ParticleSolver you want to assign to current particle shape, "0" = OriginParticleResolver, "1" = DiffusionParticleResolver.
 * - \b NCopies means the count of cloned particles from one seed particle.
 * - \b RandPattern means different random generator would be used to generate the spherical points set, "0" = rand, "1" = drand48, "2" = sobol.
 * - \b Falloff means the width of cloned points in cluster will be falloff by an powf() function.
 */
class DiffusionParticleResolver : public ParticleResolver
{
public:
	DiffusionParticleResolver();
	~DiffusionParticleResolver();

	void SetFalloff(const RtFloat&);
	void SetNCopies(const RtInt&);
	void SetRandPattern(const RtInt&);
	void SetSeed(const RtInt&);
	void SetCoarseMode(const RtInt&);

	/**
	 * \todo
	 * Current version use <a href="http://openmp.org/">OpenMP</a> to utilize your CPU as many as possible by \b omp_get_num_procs.
	 * It's still slow because it performs k-nearest-neighbor lookup from each cloned point, we also could lookup at only seed position
	 * to reuse the neighbors.
	 */
	RtVoid DoIt(RtInt, RtInt, RtToken [], RtPointer []);
private:
	RtFloat mFalloff;
	RtInt mNCopies;
	RtInt mRandPattern;
	RtInt mSeed;
	RtInt mCoarseMode;
};

NAMESPACE_PR_END
