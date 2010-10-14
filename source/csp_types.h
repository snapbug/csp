/*
	CSP_TYPES.H
	-----------
*/
#ifdef _MSC_VER
	typedef unsigned long long uint64_t;
	typedef unsigned int uint32_t;
	typedef unsigned short uint16_t;
	typedef unsigned char uint8_t; 
	typedef signed long long int64_t;
	typedef signed int int32_t;
	typedef signed short int16_t;
	typedef signed char int8_t; 
	#include <float.h> // for _isnan and _finite
	#define isnan _isnan
	#define isinf(x) (!_finite(x) && !isnan(x))
	#define strtoull(x, y, z) _strtoui64(x, y, z)
#else
	#include <stdint.h>
#endif

#ifndef FALSE
	#define FALSE 0
#endif
#ifndef TRUE
	#define TRUE (!FALSE)
#endif

#ifndef MIN
	#define MIN(a, b) ((b) < (a) ? (b) : (a))
#endif
#ifndef MAX
	#define MAX(a, b) ((b) > (a) ? (b) : (a))
#endif

#define puts(x) printf(x "\n")
#define clip(x, low, high) (low > x ? low : x < high ? x : high)
#define tri_offset(i, j, n) (((((2 * (n)) - 1 - (i)) * (i)) / 2) - (i) + (j) - 1)
#define UNUSED(x) (x = x)
