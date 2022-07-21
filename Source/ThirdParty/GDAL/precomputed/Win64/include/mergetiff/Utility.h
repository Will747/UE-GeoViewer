#ifndef _MERGETIFF_UTILITY
#define _MERGETIFF_UTILITY

#include <string>
#include <vector>

namespace mergetiff {

class Utility
{
	public:
		
		//Splits a string based on the specified delimiter
		static inline std::vector<std::string> strSplit(const std::string& str, const std::string& delim, unsigned int limit = 0)
		{
			size_t uDelimLen = delim.length();
			size_t captureCount = 0;
			size_t prevPos = 0;
			size_t currPos = 0;
			
			std::vector<std::string> result;
			
			//If looking for an empty string, simply return a vector containing the subject
			if (uDelimLen == 0)
			{
				result.push_back(str);
				return result;
			}
			
			//Loop through and find all instances of the delimiter
			while (((currPos = str.find(delim, currPos)) != std::string::npos) && (!limit || captureCount < limit-1))
			{
				result.push_back(str.substr(prevPos, currPos - prevPos));
				captureCount++;
				currPos += uDelimLen;
				prevPos = currPos;
			}
			
			//Retrieve the remaining characters (if any)
			currPos = str.length();
			if (prevPos <= currPos) {
				result.push_back(str.substr(prevPos, currPos - prevPos));
			}
			
			return result;
		}
};

} //End namespace mergetiff

#endif
