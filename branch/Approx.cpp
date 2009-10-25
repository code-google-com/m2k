#include "Approx.h"

#define _USE_MATH_DEFINES
#include <float.h>
#include <math.h>

template<class DataType, class CoordType>
WyvillApprox<DataType,CoordType>::WyvillApprox(const float R) : mR(R)
{
}

template<class DataType, class CoordType>
WyvillApprox<DataType,CoordType>::~WyvillApprox()
{
}

template<class DataType, class CoordType>
void WyvillApprox<DataType,CoordType>::GetInterpolatedValue(const size_t Len, const int D, const CT* Coords, const DT* Values, const CT& P, DT& Result)
{
	const float R = mR;
	for( size_t i=0; i<Len; ++i )
	{
		float Dist = 0.0f;
		for( int d=0; d<D; ++d )
		{
			float L = Coords[i][d] - P[d];
			Dist += L*L;
		}
		if( Dist > R )
	}
}
