#ifndef _MERGETIFF_DATASET_MANAGEMENT
#define _MERGETIFF_DATASET_MANAGEMENT

#include "DatatypeConversion.h"
#include "DriverOptions.h"
#include "ErrorHandling.h"
#include "LibrarySettings.h"
#include "RasterData.h"
#include "RasterIO.h"
#include "SmartPointers.h"

#include <algorithm>
#include <gdal.h>
#include <gdal_priv.h>
#include <cpl_conv.h>
#include <vrtdataset.h>
#include <string>
#include <vector>

namespace mergetiff {

class DatasetManagement
{
	public:
		
		//Opens a GDAL GeoTiff dataset
		static inline GDALDatasetRef openDataset(const std::string& filename)
		{
			//Register all GDAL drivers
			GDALAllRegister();
			
			//Attempt to open the input dataset
			ArgsArray drivers({"GTiff"});
			ArgsArray options({"NUM_THREADS=ALL_CPUS"});
			ArgsArray siblings;
			GDALDataset* dataset = (GDALDataset*)(GDALOpenEx(filename.c_str(), GDAL_OF_RASTER | GDAL_OF_READONLY | GDAL_OF_VERBOSE_ERROR, drivers.get(), options.get(), siblings.get()));
			
			//Verify that we were able to open the dataset
			if (!dataset) {
				return ErrorHandling::handleError<GDALDatasetRef>("failed to open input dataset \"" + filename + "\"");
			}
			
			return GDALDatasetRef(dataset);
		}
		
		//Retrieves the specified raster bands of a GDAL dataset
		static inline std::vector<GDALRasterBand*> getRasterBands(GDALDatasetRef& dataset, const std::vector<unsigned int>& bandIndices)
		{
			//Verify that all of the requested band indices are valid
			unsigned int maxBand = *(std::max_element(bandIndices.begin(), bandIndices.end()));
			if (maxBand > (unsigned int)(dataset->GetRasterCount())) {
				return ErrorHandling::handleError< std::vector<GDALRasterBand*> >("invalid band index " + std::to_string(maxBand));
			}
			
			//Retrieve each of the requested bands
			std::vector<GDALRasterBand*> bands;
			for (auto index : bandIndices) {
				bands.push_back(dataset->GetRasterBand(index));
			}
			
			return bands;
		}
		
		//Retrieves all of the raster bands of a GDAL dataset
		static inline std::vector<GDALRasterBand*> getAllRasterBands(GDALDatasetRef& dataset)
		{
			std::vector<GDALRasterBand*> bands;
			for (int index = 1; index <= dataset->GetRasterCount(); ++index) {
				bands.push_back(dataset->GetRasterBand(index));
			}
			
			return bands;
		}
		
		//Opens a dataset and reads all of its raster data into a RasterData object
		template <typename PrimitiveTy>
		static inline RasterData<PrimitiveTy> rasterFromFile(const std::string& filename, const std::vector<unsigned int>& bands = std::vector<unsigned int>())
		{
			//Attempt to open the dataset
			GDALDatasetRef dataset = DatasetManagement::openDataset(filename);
			
			//Perform the raster I/O
			return DatasetManagement::rasterFromDataset<PrimitiveTy>(dataset, bands);
		}
		
		//Writes the raster data from a RasterData object to an image file
		template <typename PrimitiveTy>
		static inline GDALDatasetRef rasterToFile(const std::string& filename, const RasterData<PrimitiveTy>& data)
		{
			GDALDataType dtype = DatatypeConversion::primitiveToGdal<PrimitiveTy>();
			ArgsArray options = DriverOptions::geoTiffOptions(dtype);
			return DatasetManagement::datasetFromRaster(data, false, "GTiff", filename, options);
		}
		
		//Reads all of the raster data from a dataset into a RasterData object
		template <typename PrimitiveTy>
		static inline RasterData<PrimitiveTy> rasterFromDataset(GDALDatasetRef& dataset, const std::vector<unsigned int>& bands = std::vector<unsigned int>())
		{
			GDALDataType expectedType = DatatypeConversion::primitiveToGdal<PrimitiveTy>();
			return RasterIO::readDataset<PrimitiveTy>(dataset, expectedType, bands);
		}
		
		//Creates a GDAL dataset from the supplied raster data (defaults to an in-memory dataset containing a copy of the raster data)
		template <typename PrimitiveTy>
		static inline GDALDatasetRef datasetFromRaster(const RasterData<PrimitiveTy>& data, bool forceGrayInterp = false, const std::string& driver = "MEM", const std::string& filename = "", ArgsArray options = ArgsArray())
		{
			//Register all GDAL drivers
			GDALAllRegister();
			
			//Attempt to retrieve a reference to the requested GDAL driver
			GDALDriver* gdalDriver = ((GDALDriver*)GDALGetDriverByName(driver.c_str()));
			if (gdalDriver == nullptr) {
				return ErrorHandling::handleError<GDALDatasetRef>("failed to retrieve the GDAL \"" + driver + "\" driver handle");
			}
			
			//Attempt to create the output dataset
			GDALDataType gdalType = DatatypeConversion::primitiveToGdal<PrimitiveTy>();
			GDALDataset* datasetPtr = gdalDriver->Create(
				filename.c_str(),
				data.cols(),
				data.rows(),
				data.channels(),
				gdalType,
				options.get()
			);
			
			//Verify that we were able to create the dataset
			if (datasetPtr == nullptr) {
				return ErrorHandling::handleError<GDALDatasetRef>("failed to create dataset from raster using \"" + driver + "\" driver");
			}
			
			//Attempt to copy the raster data into the dataset
			GDALDatasetRef dataset(datasetPtr);
			if (RasterIO::writeDataset(dataset, data) == false) {
				return ErrorHandling::handleError<GDALDatasetRef>("failed to copy raster data into dataset");
			}
			
			//Set the colour interpretation for each of the raster bands
			for (int index = 1; index < data.channels(); ++index)
			{
				GDALRasterBand* band = dataset->GetRasterBand(index+1);
				DatasetManagement::setColourInterpretation(band, index, data.channels(), forceGrayInterp);
			}
			
			return dataset;
		}
		
		//Creates a GDAL in-memory dataset that wraps the supplied raster data without copying it
		template <typename PrimitiveTy>
		static inline GDALDatasetRef wrapRasterData(const RasterData<PrimitiveTy>& data, bool forceGrayInterp = false)
		{
			//Register all GDAL drivers
			GDALAllRegister();
			
			//Build the "filename" that will specify the options for the MEM driver
			GDALDataType dtype = DatatypeConversion::primitiveToGdal<PrimitiveTy>();
			std::string dtypeName = GDALGetDataTypeName(dtype);
			char ptrStrBuf[256];
			int ptrStrLen = CPLPrintPointer(ptrStrBuf, (void*)(data.getBuffer()), 256);
			std::string ptrStr = std::string(ptrStrBuf, ptrStrLen);
			std::string filename = std::string("MEM:::") +
				"DATAPOINTER=" + ptrStr +
				",PIXELS=" + std::to_string(data.cols()) +
				",LINES=" + std::to_string(data.rows()) +
				",BANDS=" + std::to_string(data.channels()) +
				",DATATYPE=" + dtypeName +
				",PIXELOFFSET=" + std::to_string(data.channels() * sizeof(PrimitiveTy)) +
				",LINEOFFSET=" + std::to_string(data.cols() * data.channels() * sizeof(PrimitiveTy)) +
				",BANDOFFSET=" + std::to_string(sizeof(PrimitiveTy));
			
			//Attempt to open the dataset
			GDALDataset* dataset = (GDALDataset*)(GDALOpen(filename.c_str(), GA_ReadOnly));
			
			//Verify that we were able to open the dataset
			if (dataset == nullptr) {
				return ErrorHandling::handleError<GDALDatasetRef>("failed to open in-memory dataset wrapping raster data");
			}
			
			//Set the colour interpretation for each of the raster bands
			for (int index = 1; index < data.channels(); ++index)
			{
				GDALRasterBand* band = dataset->GetRasterBand(index+1);
				DatasetManagement::setColourInterpretation(band, index, data.channels(), forceGrayInterp);
			}
			
			return GDALDatasetRef(dataset);
		}
		
		//Creates a copy of the supplied dataset using the CreateCopy() method of the specified driver
		static inline GDALDatasetRef createDatasetCopy(GDALDatasetRef& dataset, const std::string& driver, const std::string& filename)
		{
			//Register all GDAL drivers
			GDALAllRegister();
			
			//Attempt to retrieve a reference to the requested GDAL driver
			GDALDriver* gdalDriver = ((GDALDriver*)GDALGetDriverByName(driver.c_str()));
			if (gdalDriver == nullptr) {
				return ErrorHandling::handleError<GDALDatasetRef>("failed to retrieve the GDAL \"" + driver + "\" driver handle");
			}
			
			//Attempt to create the copy dataset
			GDALDataset* copyDataset = gdalDriver->CreateCopy(
				filename.c_str(),
				MERGETIFF_SMART_POINTER_GET(dataset),
				false,
				nullptr,
				nullptr,
				nullptr
			);
			
			//Verify that we were able to create the copy
			if (copyDataset == nullptr) {
				return ErrorHandling::handleError<GDALDatasetRef>("failed to create dataset copy");
			}
			
			return GDALDatasetRef(copyDataset);
		}
		
		//Creates a merged dataset containing all of the supplied raster bands along with the metadata from the specified dataset
		template <typename PrimitiveTy>
		static inline GDALDatasetRef createMergedDatasetForType(const std::string& filename, GDALDatasetRef& metadataDataset, std::vector<GDALRasterBand*> rasterBands, GDALProgressFunc progressCallback = nullptr)
		{
			//Register all GDAL drivers
			GDALAllRegister();
			
			//Verify that all of the supplied raster bands have the correct datatype
			GDALDataType expectedType = DatatypeConversion::primitiveToGdal<PrimitiveTy>();
			for (auto band : rasterBands)
			{
				if (band->GetRasterDataType() != expectedType) {
					return ErrorHandling::handleError<GDALDatasetRef>("invalid datatype in one or more raster bands");
				}
			}
			
			//Attempt to retrieve a reference to the GeoTiff VRT driver
			GDALDriver* vrtDriver = ((GDALDriver*)GDALGetDriverByName("VRT"));
			if (vrtDriver == nullptr) {
				return ErrorHandling::handleError<GDALDatasetRef>("failed to retrieve the GDAL VRT driver handle");
			}
			
			//Attempt to retrieve a reference to the GeoTiff GDAL driver
			GDALDriver* tiffDriver = ((GDALDriver*)GDALGetDriverByName("GTiff"));
			if (tiffDriver == nullptr) {
				return ErrorHandling::handleError<GDALDatasetRef>("failed to retrieve the GDAL GeoTiff driver handle");
			}
			
			//Attempt to create a virtual dataset
			int width  = rasterBands[0]->GetXSize();
			int height = rasterBands[0]->GetYSize();
			GDALDataset* virtualDataset = vrtDriver->Create("", width, height, 0, expectedType, nullptr);
			
			//Verify that we were able to create the virtual dataset
			if (virtualDataset == nullptr) {
				return ErrorHandling::handleError<GDALDatasetRef>("failed to create virtual dataset");
			}
			
			//Ensure the virtual dataset is closed when the function ends
			GDALDatasetRef virtualWrapper(virtualDataset);
			
			//If a dataset was specified to copy metadata from, do so
			if (metadataDataset)
			{
				//Extract the list of metadata domains
				char** domains = metadataDataset->GetMetadataDomainList();
				
				//Check if there are any metadata domains
				if (domains == nullptr)
				{
					//No domains, simply copy the metadata for the default domain
					virtualDataset->SetMetadata(metadataDataset->GetMetadata());
				}
				else
				{
					//Copy the metadata for each domain
					char** currDomain = domains;
					while (*currDomain != nullptr)
					{
						virtualDataset->SetMetadata(metadataDataset->GetMetadata(*currDomain), *currDomain);
						currDomain++;
					}
					
					//Free the domain list
					CSLDestroy(domains);
				}
				
				//Copy projection
				virtualDataset->SetProjection(metadataDataset->GetProjectionRef());
				
				//Copy affine GeoTransform
				double padfTransform[6];
				if (metadataDataset->GetGeoTransform(padfTransform) != CE_Failure) {
					virtualDataset->SetGeoTransform(padfTransform);
				}
				
				//Copy GCPs
				if (metadataDataset->GetGCPCount() > 0)
				{
					virtualDataset->SetGCPs(
						metadataDataset->GetGCPCount(),
						metadataDataset->GetGCPs(),
						metadataDataset->GetGCPProjection()
					);
				}
			}
			
			//Assign each of the input raster bands as the source for the corresponding virtual band
			for (unsigned int index = 0; index < rasterBands.size(); ++index)
			{
				//Retrieve the input band
				GDALRasterBand* inputBand = rasterBands[index];
				
				//Create and retrieve the output band
				virtualDataset->AddBand(expectedType, nullptr);
				VRTSourcedRasterBand* outputBand = (VRTSourcedRasterBand*)(virtualDataset->GetRasterBand(index+1));
				
				//Add the input band as the source for the output band
				outputBand->AddSimpleSource(inputBand, 0, 0, width, height, 0, 0, width, height);
				
				//Copy the "no data" sentinel value, if any
				int hasNoDataValue = 0;
				double noDataValue = inputBand->GetNoDataValue(&hasNoDataValue);
				if (hasNoDataValue) {
					outputBand->SetNoDataValue(noDataValue);
				}
				
				//Copy the colour interpretation value, if any
				GDALColorInterp colourInterp = inputBand->GetColorInterpretation();
				if (colourInterp != GCI_Undefined) {
					outputBand->SetColorInterpretation(colourInterp);
				}
			}
			
			//Attempt to create the output dataset as a copy of the virtual dataset
			ArgsArray options = DriverOptions::geoTiffOptions(expectedType);
			GDALDataset* dataset = tiffDriver->CreateCopy(
				filename.c_str(),
				virtualDataset,
				false,
				options.get(),
				progressCallback,
				nullptr
			);
			
			//Verify that we were able to create the dataset
			if (dataset == nullptr) {
				return ErrorHandling::handleError<GDALDatasetRef>("failed to open output dataset \"" + filename + "\"");
			}
			
			return GDALDatasetRef(dataset);
		}
		
		//Helper function for createMergedDatasetForType() to automatically provide the correct template argument
		static inline GDALDatasetRef createMergedDataset(const std::string& filename, GDALDatasetRef& metadataDataset, std::vector<GDALRasterBand*> rasterBands, GDALProgressFunc progressCallback = nullptr)
		{
			GDALDataType dtype = rasterBands[0]->GetRasterDataType();
			
			#define _CREATE_MERGED(GdalTy, PrimitiveTy) case GdalTy: return DatasetManagement::createMergedDatasetForType<PrimitiveTy>(filename, metadataDataset, rasterBands, progressCallback)
			switch (dtype)
			{
				_CREATE_MERGED(GDT_Byte,    uint8_t);
				_CREATE_MERGED(GDT_Int16,   int16_t);
				_CREATE_MERGED(GDT_UInt16,  uint16_t);
				_CREATE_MERGED(GDT_Int32,   int32_t);
				_CREATE_MERGED(GDT_UInt32,  uint32_t);
				_CREATE_MERGED(GDT_Float32, float);
				_CREATE_MERGED(GDT_Float64, double);
				
				default:
					return ErrorHandling::handleError<GDALDatasetRef>("unsupported GDAL datatype");
			}
			#undef _CREATE_MERGED
		}
		
	protected:
		
		//Helper function to set the colour interpretation for a raster band
		static inline void setColourInterpretation(GDALRasterBand* band, int bandIndex, int totalChannels, bool forceGrayInterp)
		{
			if (forceGrayInterp == false && totalChannels >= 3 && bandIndex <= 3)
			{
				static const std::vector<GDALColorInterp> interps = {
					GCI_RedBand,
					GCI_GreenBand,
					GCI_BlueBand,
					GCI_AlphaBand
				};
				
				band->SetColorInterpretation(interps[bandIndex]);
			}
			else
			{
				//Use grayscale for all bands if forceGrayInterp is true, as
				//well as all bands beyond the fourth raster band regardless
				band->SetColorInterpretation(GCI_GrayIndex);
			}
		}
};

} //End namespace mergetiff

#endif
