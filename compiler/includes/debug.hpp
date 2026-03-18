#ifndef DEBUG_H
#define DEBUG_H

#include <iostream>

class DebugStream {
    public:
        template <typename T>
        DebugStream& operator<<(const T& value) {
#ifdef DEBUG
            std::cout << value;
# endif // DEBUG
            return *this;
        }
};

inline DebugStream DEBUG_OUTPUT;

#endif // DEBUG_H