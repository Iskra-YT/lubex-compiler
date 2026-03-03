#ifndef ERROR_LUBEX_HPP
#define ERROR_LUBEX_HPP

#include "lexer.hpp"

class Error {
    private:
        PositionSpan span;
        std::string value;
    public:
        Error(PositionSpan position, std::string text): span(position), value(text) {} 
        std::string returnError() const;
};

#endif //ERROR_LUBEX_HPP