/*
*  Vector4.cpp
*
*  Created on: 2014/10/18
*      Author: Dimitri Kourkoulis
*              http://dimitros.be
*     License: GNU LGPL
*/

#include "Vector4.h"
#include <cstddef>

namespace small3d {

	Vector4::Vector4(const float &x, const float &y, const float &z, const float &w) {
	
		this->x = x;
		this->y = y;
		this->z = z;
		this->w = w;
	}

	Vector4::~Vector4() {
		
	}

	void Vector4::getValueArray(float *valueArray)
	{
		valueArray[0] = x;
		valueArray[1] = y;
		valueArray[2] = z;
		valueArray[3] = w;
	}

}