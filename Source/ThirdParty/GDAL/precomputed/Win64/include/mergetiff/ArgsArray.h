#ifndef _MERGETIFF_ARGS_ARRAY
#define _MERGETIFF_ARGS_ARRAY

#include <initializer_list>
#include <memory>
#include <string>
#include <vector>

namespace mergetiff {

class ArgsArray
{
	public:
		
		//Creates an empty list
		inline ArgsArray() {}
		
		//Initialises the list with values
		inline ArgsArray(std::initializer_list<std::string> args)
		{
			for (auto arg : args) {
				this->add(arg);
			}
		}
		
		//Initialises the list with values
		inline ArgsArray(const std::vector<std::string>& args)
		{
			for (auto arg : args) {
				this->add(arg);
			}
		}
		
		//Adds an argument to the list
		inline void add(const std::string& arg)
		{
			this->args.push_back( std::vector<char>(arg.size()+1) );
			memcpy(this->args.back().data(), arg.c_str(), arg.size()+1);
		}
		
		//Returns an argv-style structure containing the arguments
		inline char** get()
		{
			//Clear the structure and add the pointers to each of the args
			this->structure.clear();
			for (auto& arg : this->args) {
				this->structure.push_back(arg.data());
			}
			
			//Add a nullptr at the end of the array and return the result
			this->structure.push_back(nullptr);
			return this->structure.data();
		}
		
		//Determines if the list is empty
		inline bool empty() const {
			return this->args.empty();
		}
		
	private:
		std::vector< std::vector<char> > args;
		std::vector<char*> structure;
};

} //End namespace mergetiff

#endif
