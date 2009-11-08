#include "PR.h"

NAMESPACE_PR_BEGIN

/**
 * \brief "KK"
 * 
 * It would load an external <a href="http://software.primefocusworld.com/software/support/krakatoa/">Krakatoa</a> .PRT file and render it.
 */
class KKParticleResolver : public ParticleResolver
{
public:
	KKParticleResolver();
	~KKParticleResolver();

	void SetPath(const char*);
	RtVoid DoIt(RtInt, RtInt, RtToken [], RtPointer []);
private:
	std::string mPath;
};

NAMESPACE_PR_END
