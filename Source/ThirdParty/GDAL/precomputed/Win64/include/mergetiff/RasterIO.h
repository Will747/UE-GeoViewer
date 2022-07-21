#ifndef _MERGETIFF_RASTER_IO
#define _MERGETIFF_RASTER_IO

#include "DatatypeConversion.h"
#include "ErrorHandling.h"
#include "RasterData.h"
#include "SmartPointers.h"

#include <gdal_priv.h>
#include <gdal.h>
#include <algorithm>
#include <vector>

namespace mergetiff {

class RasterIO
{
	public:
		
		//Reads the raster data for an entire dataset
		template <typename PrimitiveTy>
		static inline RasterData<PrimitiveTy> readDataset(GDALDatasetRef& dataset, GDALDataType expectedType, std::vector<unsigned int> bands = std::vector<unsigned int>())
		{
			//Verify that a valid dataset was supplied
			if (!dataset || dataset->GetRasterCount() < 1) {
				return ErrorHandling::handleError< RasterData<PrimitiveTy> >("supplied dataset does not contain any raster bands");
			}
			
			//Verify that the dataset datatype matches the expected datatype
			if (dataset->GetRasterBand(1)->GetRasterDataType() != expectedType) {
				return ErrorHandling::handleError< RasterData<PrimitiveTy> >("supplied dataset datatype does not match expected datatype");
			}
			
			//Determine if a set of band indices were specified 
			if (!bands.empty())
			{
				//Verify that all of the requested band indices are valid
				unsigned int maxBand = *(std::max_element(bands.begin(), bands.end()));
				if (maxBand > (unsigned int)(dataset->GetRasterCount())) {
					return ErrorHandling::handleError< RasterData<PrimitiveTy> >("invalid band index " + std::to_string(maxBand));
				}
			}
			else
			{
				//Fill the vector with the indices of all bands present in the dataset
				for (unsigned int index = 1; index <= (unsigned int)(dataset->GetRasterCount()); ++index) {
					bands.push_back(index);
				}
			}
			
			//Determine the image dimensions
			uint64_t numChannels = bands.size();
			uint64_t numRows = dataset->GetRasterYSize();
			uint64_t numCols = dataset->GetRasterXSize();
			
			//Create the buffer to hold the raster data
			RasterData<PrimitiveTy> data(numChannels, numRows, numCols);
			
			//Read the data one raster band at a time
			unsigned int channelOffset = 0;
			for (auto bandIndex : bands)
			{
				//Retrieve the raster band for this channel
				GDALRasterBand* band = dataset->GetRasterBand(bandIndex);
				
				//Read the raster data into our buffer
				if (RasterIO::bandToBuffer<PrimitiveTy>(band, data, numChannels, numCols, numRows, channelOffset++) == false) {
					return ErrorHandling::handleError< RasterData<PrimitiveTy> >("failed to read data from GDAL raster band");
				}
			}
			
			return data;
		}
		
		//Reads the raster data for an entire dataset into an existing in-memory buffer
		template <typename PrimitiveTy>
		static inline bool readDataset(GDALDatasetRef& dataset, RasterData<PrimitiveTy>& data, std::vector<unsigned int> bands = std::vector<unsigned int>(), unsigned int destOffset = 0)
		{
			//Verify that a valid dataset was supplied
			if (!dataset || dataset->GetRasterCount() < 1) {
				return ErrorHandling::handleError<bool>("supplied dataset does not contain any raster bands");
			}
			
			//Determine the expected datatype based on the buffer datatype
			GDALDataType expectedType = DatatypeConversion::primitiveToGdal<PrimitiveTy>();
			
			//Verify that the dataset datatype matches the expected datatype
			if (dataset->GetRasterBand(1)->GetRasterDataType() != expectedType) {
				return ErrorHandling::handleError<bool>("supplied dataset datatype does not match expected datatype");
			}
			
			//Determine if a set of band indices were specified 
			if (!bands.empty())
			{
				//Verify that all of the requested band indices are valid
				unsigned int maxBand = *(std::max_element(bands.begin(), bands.end()));
				if (maxBand > (unsigned int)(dataset->GetRasterCount())) {
					return ErrorHandling::handleError<bool>("invalid band index " + std::to_string(maxBand));
				}
			}
			else
			{
				//Fill the vector with the indices of all bands present in the dataset
				for (unsigned int index = 1; index <= (unsigned int)(dataset->GetRasterCount()); ++index) {
					bands.push_back(index);
				}
			}
			
			//Determine the image dimensions
			uint64_t numRows = dataset->GetRasterYSize();
			uint64_t numCols = dataset->GetRasterXSize();
			
			//Verify that the buffer dimensions match the image dimensions
			if (destOffset + bands.size() > data.channels() || numRows != data.rows() || numCols != data.cols()) {
				return ErrorHandling::handleError<bool>("dataset raster dimensions do not match supplied buffer dimensions");
			}
			
			//Read the data one raster band at a time
			unsigned int channelOffset = destOffset;
			for (auto bandIndex : bands)
			{
				//Retrieve the raster band for this channel
				GDALRasterBand* band = dataset->GetRasterBand(bandIndex);
				
				//Read the raster data into our buffer
				if (RasterIO::bandToBuffer<PrimitiveTy>(band, data, data.channels(), numCols, numRows, channelOffset++) == false) {
					return ErrorHandling::handleError<bool>("failed to read data from GDAL raster band");
				}
			}
			
			return data;
		}
		
		//Reads the raster data for an individual raster band
		template <typename PrimitiveTy>
		static inline RasterData<PrimitiveTy> readBand(GDALRasterBand* band, GDALDataType expectedType)
		{
			//Verify that the band datatype matches the expected datatype
			if (band->GetRasterDataType() != expectedType) {
				return ErrorHandling::handleError< RasterData<PrimitiveTy> >("supplied raster band datatype does not match expected datatype");
			}
			
			//Determine the image dimensions
			uint64_t numChannels = 1;
			uint64_t numRows = band->GetYSize();
			uint64_t numCols = band->GetXSize();
			
			//Create the buffer to hold the raster data
			RasterData<PrimitiveTy> data(numChannels, numRows, numCols);
			
			//Read the raster data into our buffer
			if (RasterIO::bandToBuffer<PrimitiveTy>(band, data, numChannels, numCols, numRows) == false) {
				return ErrorHandling::handleError< RasterData<PrimitiveTy> >("failed to read data from GDAL raster band");
			}
			
			return data;
		}
		
		//Reads the raster data for an individual raster band into an existing in-memory buffer, with an optional destination channel offset
		template <typename PrimitiveTy>
		static inline bool readBandWithOffset(GDALDatasetRef& dataset, RasterData<PrimitiveTy>& data, unsigned int bandIndex, unsigned int destOffset = 0)
		{
			//Verify that a valid dataset was supplied
			if (!dataset || dataset->GetRasterCount() < 1) {
				return ErrorHandling::handleError<bool>("supplied dataset does not contain any raster bands");
			}
			
			//Verify that the dataset datatype matches the expected datatype
			GDALDataType expectedType = DatatypeConversion::primitiveToGdal<PrimitiveTy>();
			if (dataset->GetRasterBand(1)->GetRasterDataType() != expectedType) {
				return ErrorHandling::handleError<bool>("supplied dataset datatype does not match expected datatype");
			}
			
			//Verify that the requested band index is valid
			if (bandIndex > dataset->GetRasterCount()) {
				return ErrorHandling::handleError<bool>("invalid band index " + std::to_string(bandIndex));
			}
			
			//Verify that the specified destination channel offset is valid
			if (destOffset >= data.channels()) {
				return ErrorHandling::handleError<bool>("destination channel offset " + std::to_string(destOffset));
			}
			
			//Determine the image dimensions
			uint64_t numChannels = data.channels();
			uint64_t numRows = dataset->GetRasterYSize();
			uint64_t numCols = dataset->GetRasterXSize();
			
			//Verify that the destination buffer dimensions match the image dimensions
			if (data.rows() != numRows || data.cols() != numCols) {
				return ErrorHandling::handleError<bool>("cannot copy raster data into buffer with different image dimensions");
			}
			
			//Retrieve the requested raster band
			GDALRasterBand* band = dataset->GetRasterBand(bandIndex);
			
			//Read the raster data into the buffer
			return RasterIO::bandToBuffer<PrimitiveTy>(band, data, numChannels, numCols, numRows, destOffset);
		}
		
		//Writes the raster data for an entire dataset
		template <typename PrimitiveTy>
		static inline bool writeDataset(GDALDatasetRef& dataset, const RasterData<PrimitiveTy>& data)
		{
			//If an invalid dataset was supplied, signal failure
			if (!dataset || dataset->GetRasterCount() < 1) {
				return false;
			}
			
			//If the dataset datatype does not match the expected datatype, signal failure
			GDALDataType expectedType = DatatypeConversion::primitiveToGdal<PrimitiveTy>();
			if (dataset->GetRasterBand(1)->GetRasterDataType() != expectedType) {
				return false;
			}
			
			//Determine the image dimensions
			uint64_t numChannels = dataset->GetRasterCount();
			uint64_t numRows = dataset->GetRasterYSize();
			uint64_t numCols = dataset->GetRasterXSize();
			
			//If the image dimensions do not match the dimensions of the supplied raster data, signal failure
			if (numChannels != data.channels() || numRows != data.rows() || numCols != data.cols()) {
				return false;
			}
			
			//Write the data one channel at a time
			for (int channel = 0; channel < numChannels; ++channel)
			{
				//Retrieve the raster band for this channel
				GDALRasterBand* band = dataset->GetRasterBand(channel + 1);
				
				//Write the raster data from our buffer
				if (RasterIO::bufferToBand<PrimitiveTy>(band, data, numChannels, numCols, numRows, channel) == false) {
					return false;
				}
			}
			
			return true;
		}
		
		//Writes the raster data for an individual raster band
		template <typename PrimitiveTy>
		static inline bool writeBand(GDALRasterBand* band, const RasterData<PrimitiveTy>& data)
		{
			//If the dataset datatype does not match the expected datatype, signal failure
			GDALDataType expectedType = DatatypeConversion::primitiveToGdal<PrimitiveTy>();
			if (band->GetRasterDataType() != expectedType) {
				return false;
			}
			
			//Determine the image dimensions
			uint64_t numChannels = 1;
			uint64_t numRows = band->GetYSize();
			uint64_t numCols = band->GetXSize();
			
			//If the image dimensions do not match the dimensions of the supplied raster data, signal failure
			if (numChannels != data.channels() || numRows != data.rows() || numCols != data.cols()) {
				return false;
			}
			
			//Write the raster data from our buffer
			return RasterIO::bufferToBand<PrimitiveTy>(band, data, numChannels, numCols, numRows);
		}
		
		//Performs the GDALRasterBand::RasterIO() call to read data from the band into an in-memory buffer
		template <typename PrimitiveTy>
		static inline bool bandToBuffer(GDALRasterBand* band, RasterData<PrimitiveTy>& data, uint64_t numChannels, uint64_t numCols, uint64_t numRows, uint64_t channelOffset = 0)
		{
			GDALDataType dtype = DatatypeConversion::primitiveToGdal<PrimitiveTy>();
			CPLErr result = band->RasterIO(
				GF_Read,
				0,
				0,
				numCols,
				numRows,
				data.getBuffer() + channelOffset,
				numCols,
				numRows,
				dtype,
				sizeof(PrimitiveTy) * numChannels,
				sizeof(PrimitiveTy) * numChannels * numCols
			);
			
			return (result != CE_Failure);
		}
		
		//Performs the GDALRasterBand::RasterIO() call to write data to the band from an in-memory buffer
		template <typename PrimitiveTy>
		static inline bool bufferToBand(GDALRasterBand* band, const RasterData<PrimitiveTy>& data, uint64_t numChannels, uint64_t numCols, uint64_t numRows, uint64_t channelOffset = 0)
		{
			GDALDataType dtype = DatatypeConversion::primitiveToGdal<PrimitiveTy>();
			CPLErr result = band->RasterIO(
				GF_Write,
				0,
				0,
				numCols,
				numRows,
				(void*)(data.getBuffer() + channelOffset),
				numCols,
				numRows,
				dtype,
				sizeof(PrimitiveTy) * numChannels,
				sizeof(PrimitiveTy) * numChannels * numCols
			);
			
			return (result != CE_Failure);
		}
};

} //End namespace mergetiff

#endif
