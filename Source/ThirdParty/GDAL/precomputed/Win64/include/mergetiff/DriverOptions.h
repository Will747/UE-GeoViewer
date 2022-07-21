#ifndef _MERGETIFF_DRIVER_OPTIONS
#define _MERGETIFF_DRIVER_OPTIONS

#include "ArgsArray.h"
#include <gdal.h>

namespace mergetiff {

class DriverOptions
{
	public:
		
		//Returns the driver options for creating datasets with the GeoTiff driver
		static inline ArgsArray geoTiffOptions(GDALDataType dtype)
		{
			//Use LZW compresion/decompression with all CPU cores
			ArgsArray options;
			options.add("NUM_THREADS=ALL_CPUS");
			options.add("COMPRESS=LZW");
			
			//Use predictor=2 for integer types and predictor=3 for floating-point types
			if (dtype == GDT_Float32 || dtype == GDT_Float64) {
				options.add("PREDICTOR=3");
			}
			else {
				options.add("PREDICTOR=2");
			}
			
			return options;
		}
};

} //End namespace mergetiff

#endif
