#ifndef _MERGETIFF_RASTER_DATA
#define _MERGETIFF_RASTER_DATA

#include "LibrarySettings.h"

#include <stdint.h>

namespace mergetiff {

//Stores raster pixel data in continuous row-major format
template <typename PrimitiveTy>
class RasterData
{
	public:
		
		//Creates an empty object
		RasterData() : _data(nullptr), _autoRelease(false), _channels(0), _rows(0), _cols(0) {}
		
		//Creates a buffer with the specified dimensions
		RasterData(uint64_t channels, uint64_t rows, uint64_t cols)
		{
			MERGETIFF_SMART_POINTER_RESET(this->_data, new PrimitiveTy[channels * rows * cols]);
			this->_autoRelease = false;
			this->_channels = channels;
			this->_rows = rows;
			this->_cols = cols;
		}
		
		//Takes ownership of an existing buffer with the specified dimensions, or simply wraps the buffer if autoRelease == true
		RasterData(PrimitiveTy* buffer, uint64_t channels, uint64_t rows, uint64_t cols, bool autoRelease = false)
		{
			MERGETIFF_SMART_POINTER_RESET(this->_data, buffer);
			this->_autoRelease = autoRelease;
			this->_channels = channels;
			this->_rows = rows;
			this->_cols = cols;
		}
		
		//RasterData objects cannot be copied
		RasterData(const RasterData& other) = delete;
		RasterData& operator=(const RasterData& other) = delete;
		
		//Move constructor
		RasterData(RasterData&& other) {
			this->moveFrom(std::move(other));
		}
		
		//Overloaded assignment operator for moving from another instance
		RasterData& operator=(RasterData&& other)
		{
			this->moveFrom(std::move(other));
			return *this;
		}
		
		//Destructor
		~RasterData()
		{
			//If we don't actually own the memory for the underlying buffer then relinquish it
			//(Note: this unconventional behaviour was chosen to avoid introducing breaking API changes to mergetiff, which would
			// have otherwise been necessary to accommodate variants of RasterData that use weak pointers / shared pointers / etc.)
			if (this->_autoRelease == true) {
				this->releaseBuffer();
			}
		}
		
		//Determines if the underlying buffer is valid
		operator bool() {
			return (bool)(this->_data);
		}
		
		//Returns the number of channels in the raster data
		uint64_t channels() const {
			return this->_channels;
		}
		
		//Returns the number of rows in the raster data
		uint64_t rows() const {
			return this->_rows;
		}
		
		//Returns the number of columns in the raster data
		uint64_t cols() const {
			return this->_cols;
		}
		
		//Retrieves a reference to the specified channel of the specified pixel
		PrimitiveTy& pixelComponent(uint64_t y, uint64_t x, uint64_t channel) {
			return this->_data[ this->index(y, x, channel) ];
		}
		
		//Retrieves a const reference to the specified channel of the specified pixel
		const PrimitiveTy& pixelComponent(uint64_t y, uint64_t x, uint64_t channel) const {
			return this->_data[ this->index(y, x, channel) ];
		}
		
		//Retrieves the pointer to the underlying buffer
		PrimitiveTy* getBuffer() {
			return MERGETIFF_SMART_POINTER_GET(this->_data);
		}
		
		//Retrieves a const pointer to the underlying buffer
		const PrimitiveTy* getBuffer() const {
			return MERGETIFF_SMART_POINTER_GET(this->_data);
		}
		
		//Relinquishes ownership of the underlying buffer and resets this object
		PrimitiveTy* releaseBuffer()
		{
			this->_autoRelease = false;
			this->_channels = 0;
			this->_rows = 0;
			this->_cols = 0;
			return MERGETIFF_SMART_POINTER_RELEASE(this->_data);
		}
		
	protected:
		
		//Moves data from another object instance
		void moveFrom(RasterData&& other)
		{
			this->_data = std::move(other._data);
			this->_autoRelease = other._autoRelease;
			this->_channels = other._channels;
			this->_rows = other._rows;
			this->_cols = other._cols;
			other.releaseBuffer();
		}
		
		//Computes the array index for the specified channel of the specified pixel
		uint64_t index(uint64_t y, uint64_t x, uint64_t channel) const {
			return (y * this->_cols * this->_channels) + (x * this->_channels) + channel;
		}
		
		MERGETIFF_SMART_POINTER_TYPE<PrimitiveTy[]> _data;
		bool _autoRelease;
		uint64_t _channels;
		uint64_t _rows;
		uint64_t _cols;
};

} //End namespace mergetiff

#endif
