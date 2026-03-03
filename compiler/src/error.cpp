#include "error.hpp"
#include <sstream>
#include <string>

std::string Error::returnError() const {
    std::ostringstream oss;
    oss << "(" << span.start.line << ", " << span.start.column << "): " << value;
    return oss.str();
}