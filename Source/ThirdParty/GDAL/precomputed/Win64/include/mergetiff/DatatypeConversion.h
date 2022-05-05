#ifndef _MERGETIFF_DATATYPE_CONVERSION
#define _MERGETIFF_DATATYPE_CONVERSION

#include "ErrorHandling.h"

#include <gdal.h>
#include <stdint.h>

namespace mergetiff {
namespace DatatypeConversion {

template <typename PrimitiveTy>
inline GDALDataType primitiveToGdal() {
	return ErrorHandling::handleError(GDT_Unknown, "unsupported primitive type");
}

#define _MERGETIFF_P2G_SPECIALISATION(PrimitiveTy, GdalTy) template<> inline GDALDataType primitiveToGdal<PrimitiveTy>() { return GdalTy; }
_MERGETIFF_P2G_SPECIALISATION(uint8_t,  GDT_Byte)
_MERGETIFF_P2G_SPECIALISATION(uint16_t, GDT_UInt16)
_MERGETIFF_P2G_SPECIALISATION(int16_t,  GDT_Int16)
_MERGETIFF_P2G_SPECIALISATION(uint32_t, GDT_UInt32)
_MERGETIFF_P2G_SPECIALISATION(int32_t,  GDT_Int32)
_MERGETIFF_P2G_SPECIALISATION(float,    GDT_Float32)
_MERGETIFF_P2G_SPECIALISATION(double,   GDT_Float64)

} //End namespace DatatypeConversion
} //End namespace mergetiff

#endif
