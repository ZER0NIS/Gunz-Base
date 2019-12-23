#pragma once

#include "MMath.h"
#include "RTypes.h"
#include "MUtil.h"
#include "MDebug.h"
#include <cassert>
#define _USE_MATH_DEFINES
#include <cmath>
#include <algorithm>
#include <mmintrin.h>

using std::min;
using std::max;
using std::sin;
using std::cos;
using std::tan;
using std::asin;
using std::acos;
using std::atan;
using std::atan2;
using std::abs;
using std::pow;
using std::log;


_NAMESPACE_REALSPACE2_BEGIN

inline bool Equals(float a, float b)
{
	return IS_EQ(a, b);
}

inline bool Equals(const v3& a, const v3& b)
{
	for (size_t i{}; i < 3; ++i)
		if (!Equals(a[i], b[i]))
			return false;

	return true;
}

inline bool Equals(const rmatrix& a, const rmatrix& b)
{
	for (size_t i{}; i < 4; ++i)
		for (size_t j{}; j < 4; ++j)
			if (!Equals(a(i, j), b(i, j)))
				return false;

	return true;
}

inline bool Equals(const rquaternion& a, const rquaternion& b)
{
	for (size_t i{}; i < 4; ++i)
		if (!Equals(a[i], b[i]))
			return false;

	return true;
}

_NAMESPACE_REALSPACE2_END
