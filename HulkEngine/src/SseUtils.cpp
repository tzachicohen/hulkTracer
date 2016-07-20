/*
**	  GPU Raytrace - Diploma thesis, Czech technical university
**	  Copyright (C) 2008 Martin Zlatuska
**
**	  This program is free software: you can redistribute it and/or modify
**    it under the terms of the GNU General Public License as published by
**    the Free Software Foundation, either version 3 of the License, or
**    (at your option) any later version.
**
**    This program is distributed in the hope that it will be useful,
**    but WITHOUT ANY WARRANTY; without even the implied warranty of
**    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**    GNU General Public License for more details.
**
**    You should have received a copy of the GNU General Public License
**    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <xmmintrin.h>
#include <float.h>
#include "SPMatrix.hpp"

__m128 zeroesInit = { 0.0f, 0.0f, 0.0f, 0.0f};
__m128 onesInit   = { 1.0f, 1.0f, 1.0f, 1.0f};

__m128 minimumInit  = { FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX};
__m128 maximumInit  = {-FLT_MAX,-FLT_MAX,-FLT_MAX,-FLT_MAX};
__m128 minimumInit3 = { FLT_MAX, FLT_MAX, FLT_MAX, 0.0f};
__m128 maximumInit3 = {-FLT_MAX,-FLT_MAX,-FLT_MAX, 0.0f};
