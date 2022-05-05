#ifndef _MERGETIFF_SMART_POINTERS
#define _MERGETIFF_SMART_POINTERS

#include "LibrarySettings.h"

#include <cpl_conv.h>
#include <gdal.h>
#include <gdal_priv.h>
#include <gdal_utils.h>
#include <gdal_version.h>
#include <ogr_core.h>
#include <ogr_spatialref.h>

#define _MERGETIFF_DELETER_CLASS(dtype, callback) 

namespace mergetiff {

//Template class for smart pointer deleter objects that call functions which accept T* pointers
template <typename T, void(*DeleterFunc)(T*)>
class _CustomDeleterBaseT
{
	public:
		inline void operator()(T* t) {
			DeleterFunc(t);
		}
};

//Template class for smart pointer deleter objects that call functions which accept void* pointers
template <void(*DeleterFunc)(void*)>
class _CustomDeleterBaseVoid
{
	public:
		inline void operator()(void* v) {
			DeleterFunc(v);
		}
};

//Boilerplate macros for declaring our smart pointer types
#define _MERGETIFF_VOID_DELETER_SMART_POINTER(dtype, callback) typedef MERGETIFF_SMART_POINTER_TYPE<dtype, _CustomDeleterBaseVoid<callback>> dtype##Ref
#define _MERGETIFF_T_DELETER_SMART_POINTER(dtype, callback) typedef MERGETIFF_SMART_POINTER_TYPE<dtype, _CustomDeleterBaseT<dtype, callback>> dtype##Ref
#define _MERGETIFF_OPTS_DELETER_SMART_POINTER(dtype) typedef MERGETIFF_SMART_POINTER_TYPE<dtype, _CustomDeleterBaseT<dtype, dtype##Free>> dtype##Ref

//Smart pointer type for raw C strings returned by the GDAL API
//(Note that you could also use the CPLString class from <cpl_string.h> and its Seize() method to achieve the same RAII behaviour, albeit with a memory copy and more verbose syntax)
typedef MERGETIFF_SMART_POINTER_TYPE<char[], _CustomDeleterBaseVoid<CPLFree>> CPLStringRef;

//Smart pointer type for GDAL datasets
_MERGETIFF_VOID_DELETER_SMART_POINTER(GDALDataset, GDALClose);

//Smart pointer type for OGR coordinate transformation objects
_MERGETIFF_VOID_DELETER_SMART_POINTER(OGRCoordinateTransformation, OCTDestroyCoordinateTransformation);

//Smart pointer types for the various options classes for the GDAL utility programs declared in <gdal_utils.h>
_MERGETIFF_OPTS_DELETER_SMART_POINTER(GDALInfoOptions);
_MERGETIFF_OPTS_DELETER_SMART_POINTER(GDALTranslateOptions);
_MERGETIFF_OPTS_DELETER_SMART_POINTER(GDALWarpAppOptions);
_MERGETIFF_OPTS_DELETER_SMART_POINTER(GDALVectorTranslateOptions);
_MERGETIFF_OPTS_DELETER_SMART_POINTER(GDALDEMProcessingOptions);
_MERGETIFF_OPTS_DELETER_SMART_POINTER(GDALNearblackOptions);
_MERGETIFF_OPTS_DELETER_SMART_POINTER(GDALGridOptions);
_MERGETIFF_OPTS_DELETER_SMART_POINTER(GDALRasterizeOptions);
_MERGETIFF_OPTS_DELETER_SMART_POINTER(GDALBuildVRTOptions);

//Smart pointer types for GDAL utility programs introduced in GDAL 3.1
#if GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(3,1,0)
_MERGETIFF_OPTS_DELETER_SMART_POINTER(GDALMultiDimInfoOptions);
_MERGETIFF_OPTS_DELETER_SMART_POINTER(GDALMultiDimTranslateOptions);
#endif

} //End namespace mergetiff

#endif
