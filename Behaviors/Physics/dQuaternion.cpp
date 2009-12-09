//********************************************************************
// Newton Game dynamics 
// copyright 2000-2004
// By Julio Jerez
// VC: 6.0
// basic Hierarchical Scene Node Class
//********************************************************************

#include "stdafx.h"
#include "dVector.h"
#include "dMatrix.h"
#include "dQuaternion.h"

enum QUAT_INDEX
{
   X_INDEX=0,
   Y_INDEX=1,
   Z_INDEX=2
};
static QUAT_INDEX QIndex [] = {Y_INDEX, Z_INDEX, X_INDEX};


dQuaternion::dQuaternion (const dMatrix &matrix)
{
	dFloat *ptr;
	dFloat trace;
	QUAT_INDEX i;
	QUAT_INDEX j;
	QUAT_INDEX k;

	trace = matrix[0][0] + matrix[1][1] + matrix[2][2];

	if (trace > dFloat(0.0f)) {
		trace = sqrt (trace + dFloat(1.0f));
		m_q0 = dFloat (0.5f) * trace;
		trace = dFloat (0.5f) / trace;

		m_q1 = (matrix[2][1] - matrix[1][2]) * trace;
		m_q2 = (matrix[0][2] - matrix[2][0]) * trace;
		m_q3 = (matrix[1][0] - matrix[0][1]) * trace;

	} else {
		i = X_INDEX;
		if (matrix[Y_INDEX][Y_INDEX] > matrix[X_INDEX][X_INDEX]) {
			i = Y_INDEX;
		}
		if (matrix[Z_INDEX][Z_INDEX] > matrix[i][i]) {
			i = Z_INDEX;
		}
		j = QIndex [i];
		k = QIndex [j];

		trace = dFloat(1.0f) + matrix[i][i] - matrix[j][j] - matrix[k][k];
		trace = sqrt (trace);

		ptr = &m_q1;
		ptr[i] = dFloat (0.5f) * trace;
		trace = dFloat (0.5f) / trace;
		ptr[3] = (matrix[k][j] - matrix[j][k]) * trace;
		ptr[j] = (matrix[j][i] + matrix[i][j]) * trace;
		ptr[k] = (matrix[k][i] + matrix[i][k]) * trace;
	}

	m_q1 *= dFloat (-1.0f);
	m_q2 *= dFloat (-1.0f);;
	m_q3 *= dFloat (-1.0f);;

#if _DEBUG
	dFloat err = abs (DotProduct(*this) - dFloat(1.0f));
	_ASSERTE (err < dFloat(1.0e-3f));
#endif

}


dQuaternion::dQuaternion (const dVector &unitAxis, dFloat Angle)
{
	dFloat sinAng;

	Angle *= dFloat (0.5f);
	m_q0 = cos (Angle);
	sinAng = sin (Angle);

#ifdef _DEBUG
	if (abs (Angle) > dFloat(1.0e-6f)) {
		_ASSERTE (abs (dFloat(1.0f) - unitAxis % unitAxis) < dFloat(1.0e-3f));
	} 
#endif
	m_q1 = unitAxis.m_x * sinAng;
	m_q2 = unitAxis.m_y * sinAng;
	m_q3 = unitAxis.m_z * sinAng;
}


dVector dQuaternion::CalcAverageOmega (const dQuaternion &QB, dFloat dt) const
{
	dFloat dirMag;
	dFloat dirMag2;
	dFloat omegaMag;
	dFloat dirMagInv;


	dQuaternion dq (Inverse() * QB);
	dVector omegaDir (dq.m_q1, dq.m_q2, dq.m_q3);

	dirMag2 = omegaDir % omegaDir;
	if (dirMag2	< dFloat(dFloat (1.0e-5f) * dFloat (1.0e-5f))) {
		return dVector (dFloat(0.0f), dFloat(0.0f), dFloat(0.0f), dFloat(0.0f));
	}

	dirMagInv = dFloat (1.0f) / sqrt (dirMag2);
	dirMag = dirMag2 * dirMagInv;

	omegaMag = dFloat(2.0f) * atan2 (dirMag, dq.m_q0) / dt;

	return omegaDir.Scale (dirMagInv * omegaMag);
}


dQuaternion dQuaternion::Slerp (const dQuaternion &QB, dFloat t) const 
{
	dFloat dot;
	dFloat ang;
	dFloat Sclp;
	dFloat Sclq;
	dFloat den;
	dFloat sinAng;
	dQuaternion Q;

	dot = DotProduct (QB);

	if ((dot + dFloat(1.0f)) > dFloat(1.0e-5f)) {
		if (dot < dFloat(0.995f)) {

			ang = acos (dot);

			sinAng = sin (ang);
			den = dFloat(1.0f) / sinAng;

			Sclp = sin ((dFloat(1.0f) - t ) * ang) * den;
			Sclq = sin (t * ang) * den;

		} else  {
			Sclp = dFloat(1.0f) - t;
			Sclq = t;
		}

		Q.m_q0 = m_q0 * Sclp + QB.m_q0 * Sclq;
		Q.m_q1 = m_q1 * Sclp + QB.m_q1 * Sclq;
		Q.m_q2 = m_q2 * Sclp + QB.m_q2 * Sclq;
		Q.m_q3 = m_q3 * Sclp + QB.m_q3 * Sclq;

	} else {
		Q.m_q0 =  m_q3;
		Q.m_q1 = -m_q2;
		Q.m_q2 =  m_q1;
		Q.m_q3 =  m_q0;

		Sclp = sin ((dFloat(1.0f) - t) * dFloat (3.1416f *0.5f));
		Sclq = sin (t * dFloat (3.1416f * 0.5f));

		Q.m_q0 = m_q0 * Sclp + Q.m_q0 * Sclq;
		Q.m_q1 = m_q1 * Sclp + Q.m_q1 * Sclq;
		Q.m_q2 = m_q2 * Sclp + Q.m_q2 * Sclq;
		Q.m_q3 = m_q3 * Sclp + Q.m_q3 * Sclq;
	}

	dot = Q.DotProduct (Q);
	if ((dot) < (1.0f - 1.0e-4f) ) {
		dot = dFloat(1.0f) / sqrt (dot);
		//dot = dgRsqrt (dot);
		Q.m_q0 *= dot;
		Q.m_q1 *= dot;
		Q.m_q2 *= dot;
		Q.m_q3 *= dot;
	}
	return Q;
}

