#include "PR.h"

NAMESPACE_PR_BEGIN

/**
 * \brief "Origin"
 *
 * It calls RenderMan's internal RiPointsV function.
 */
class OriginParticleResolver : public ParticleResolver
{
public:
	OriginParticleResolver();
	~OriginParticleResolver();

	RtVoid DoIt(RtInt, RtInt, RtToken [], RtPointer []);
};

NAMESPACE_PR_END
