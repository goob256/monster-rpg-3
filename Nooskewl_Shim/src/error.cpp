#include "Nooskewl_Shim/error.h"

using namespace noo;

namespace noo {

namespace util {

Error::Error()
{
}

Error::Error(std::string error_message) : error_message(error_message)
{
}

MemoryError::MemoryError(std::string error_message)
{
	this->error_message = "Memory error: " + error_message;
}

LoadError::LoadError(std::string error_message)
{
	this->error_message = "Load error: " + error_message;
}

FileNotFoundError::FileNotFoundError(std::string error_message)
{
	this->error_message = "File not found: " + error_message;
}

GLError::GLError(std::string error_message)
{
	this->error_message = "OpenGL error: " + error_message;
}

} // End namespace util

} // End namespace noo
