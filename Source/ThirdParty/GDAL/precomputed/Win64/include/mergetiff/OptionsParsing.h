#ifndef _MERGETIFF_OPTIONS_PARSING
#define _MERGETIFF_OPTIONS_PARSING

#include "ArgsArray.h"
#include "SmartPointers.h"

#include <gdal_utils.h>
#include <gdal_version.h>

//Boilerplate macro for declaring our convenience functions for parsing utility options
#define _MERGETIFF_OPTS_PARSER_METHOD(dtype) static inline dtype##Ref parse##dtype(ArgsArray& args) { return dtype##Ref(dtype##New(args.get(), nullptr)); }

namespace mergetiff {

class OptionsParsing
{
	public:
		
		//Provide convenient wrapper methods for parsing options for each of GDAL's utility programs
		_MERGETIFF_OPTS_PARSER_METHOD(GDALInfoOptions)
		_MERGETIFF_OPTS_PARSER_METHOD(GDALTranslateOptions)
		_MERGETIFF_OPTS_PARSER_METHOD(GDALWarpAppOptions)
		_MERGETIFF_OPTS_PARSER_METHOD(GDALVectorTranslateOptions)
		_MERGETIFF_OPTS_PARSER_METHOD(GDALDEMProcessingOptions)
		_MERGETIFF_OPTS_PARSER_METHOD(GDALNearblackOptions)
		_MERGETIFF_OPTS_PARSER_METHOD(GDALGridOptions)
		_MERGETIFF_OPTS_PARSER_METHOD(GDALRasterizeOptions)
		_MERGETIFF_OPTS_PARSER_METHOD(GDALBuildVRTOptions)
		
		//Wrapper methods for GDAL utility programs introduced in GDAL 3.1
		#if GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(3,1,0)
		_MERGETIFF_OPTS_PARSER_METHOD(GDALMultiDimInfoOptions)
		_MERGETIFF_OPTS_PARSER_METHOD(GDALMultiDimTranslateOptions)
		#endif
};

} //End namespace mergetiff

#endif
