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
	#include <float.h> // for _isnan
	#define isnan _isnan
	#define isinf(x) (!_finite(x) && !isnan(x))
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
	#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif
#ifndef MAX
	#define MAX(a, b) ((a) > (b) ? (a) : (b))
#endif

#define clip(x, low, high) (low > x ? low : x < high ? x : high)
