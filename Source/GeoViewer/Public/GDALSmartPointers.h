// Originally from the UnrealGDAL plugin.
// https://github.com/TensorWorks/UnrealGDAL/blob/master/Source/UnrealGDAL/Public/SmartPointers.h

#pragma once

#include "GDALHeaders.h"
#include "Core/Public/Templates/UniquePtr.h"

//Smart pointer type for affine transformation coefficients
typedef TUniquePtr<double[]> GeoTransformRef;

//Alias the smart pointer types provided by mergetiff
typedef mergetiff::CPLStringRef CPLStringRef;
typedef mergetiff::GDALDatasetRef GDALDatasetRef;
typedef mergetiff::OGRCoordinateTransformationRef OGRCoordinateTransformationRef;
typedef mergetiff::GDALInfoOptionsRef GDALInfoOptionsRef;
typedef mergetiff::GDALTranslateOptionsRef GDALTranslateOptionsRef;
typedef mergetiff::GDALWarpAppOptionsRef GDALWarpAppOptionsRef;
typedef mergetiff::GDALVectorTranslateOptionsRef GDALVectorTranslateOptionsRef;
typedef mergetiff::GDALDEMProcessingOptionsRef GDALDEMProcessingOptionsRef;
typedef mergetiff::GDALNearblackOptionsRef GDALNearblackOptionsRef;
typedef mergetiff::GDALGridOptionsRef GDALGridOptionsRef;
typedef mergetiff::GDALRasterizeOptionsRef GDALRasterizeOptionsRef;
typedef mergetiff::GDALBuildVRTOptionsRef GDALBuildVRTOptionsRef;
#if GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(3,1,0)
typedef mergetiff::GDALMultiDimInfoOptionsRef GDALMultiDimInfoOptionsRef;
typedef mergetiff::GDALMultiDimTranslateOptionsRef GDALMultiDimTranslateOptionsRef;
#endif
