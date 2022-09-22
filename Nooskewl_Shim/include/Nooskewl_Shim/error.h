#ifndef NOO_ERROR_H
#define NOO_ERROR_H

#include "Nooskewl_Shim/main.h"

namespace noo {

namespace util {

class NOOSKEWL_SHIM_EXPORT Error {
public:
	Error();
	Error(std::string error_message);
	
	std::string error_message;
};

class NOOSKEWL_SHIM_EXPORT MemoryError : public Error {
public:
	MemoryError(std::string error_message);
};

class NOOSKEWL_SHIM_EXPORT LoadError : public Error {
public:
	LoadError(std::string error_message);
};

class NOOSKEWL_SHIM_EXPORT FileNotFoundError : public Error {
public:
	FileNotFoundError(std::string error_message);
};

class NOOSKEWL_SHIM_EXPORT GLError : public Error {
public:
	GLError(std::string error_message);
};

} // End namespace util

} // End namespace noo

#endif // NOO_ERROR_H
