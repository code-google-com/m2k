#include "Approx.h"

#include <iostream>
#include <math.h>

#include <boost/function.hpp>
#include <boost/scoped_array.hpp>

#include <gsl/gsl_vector.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_linalg.h>
#include <gsl/gsl_blas.h>

struct Linear
{
	float operator() (float r, float c) const
	{
		return r;
	}
};

struct TPSpline
{
	float operator() (float r, float c) const
	{
		return r*r*log(r);
	}
};

struct Gaussian
{
	float operator() (float r, float c) const
	{
		return exp(-c*r*r);
	}
};

struct Multiquad
{
	float operator() (float r, float c) const
	{
		return sqrt( r*r + c*c );
	}
};

typedef  boost::function<float(float r,float c)> RBF;

template<class DataType, class CoordType>
void RBFApprox<DataType,CoordType>::GetInterpolatedValue(const size_t Len, const int, const CT* Coords, const DT* Values, const CT& LookupPos, DT& Result)
{
	const float Const = mConst;

	gsl_vector* f = gsl_vector_alloc(Len);
	gsl_vector* c = gsl_vector_alloc(Len);
	gsl_matrix* A = gsl_matrix_alloc(Len,Len);
	gsl_matrix* A_i = gsl_matrix_alloc(Len,Len);

	// Fill the value f
	for(size_t i=0; i<Len; ++i)
		gsl_vector_set(f,i,Values[i]);
	
	//Fill the matrix A_{ij}
	RBF TheBasisFunction = Multiquad();
	for( size_t i=0; i<Len; ++i )
	{
		for( size_t j=0; j<Len; ++j )
		{
			float3 V;
			V.x = Coords[i].x - LookupPos.x;
			V.y = Coords[i].y - LookupPos.y;
			V.z = Coords[i].z - LookupPos.z;
			float Len = sqrtf( V.x*V.x + V.y*V.y + V.z*V.z );
			
			double Phi = myrsf(Len,Const);
			gsl_matrix_set(A,i,j,Phi);
		}
	}

	gsl_permutation* A_perm = gsl_permutation_alloc(N);
	boost::scoped_array<int> A_perm_sign( new int[N] );
	gsl_linalg_LU_decomp(A,A_perm,A_perm_sign.get());
	gsl_linalg_LU_invert(A,A_perm,A_i);

	gsl_blas_dgemv(CblasNoTrans,1.0,A_i,f,0.0,c);

	DT R = (DT)0;
	for( size_t a = 0; a<N; ++a )
	{
		float3 V;
		V.x = Coords[i].x - LookupPos.x;
		V.y = Coords[i].y - LookupPos.y;
		V.z = Coords[i].z - LookupPos.z;
		float Len = sqrtf( V.x*V.x + V.y*V.y + V.z*V.z );

		R += gsl_vector_get(c,a)*myrsf(Len,Const);
	}

	Result = R;
}
