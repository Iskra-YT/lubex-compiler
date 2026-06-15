#ifndef ERROR_LUBEX_HPP
#define ERROR_LUBEX_HPP

#include "lexer.hpp"

class Error {
    private:
    PositionSpan span;
    std::string value;
    std::string file;

    public:
    Error(PositionSpan position, std::string text, std::string file) : span(position), value(text), file(file) {
    }
    std::string returnError() const;
};

#endif // ERROR_LUBEX_HPP