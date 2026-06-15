#include "emiter/emiter.hpp"
#include <vector>

std::string mangleName(const std::string& name) {
    if (mangleVisitor != "") {
        return mangleVisitor;
    }

    std::vector<std::string> parts;
    size_t start = 0;
    size_t end = name.find('.');

    while (end != std::string::npos) {
        parts.push_back(name.substr(start, end - start));
        start = end + 1;
        end = name.find('.', start);
    }

    parts.push_back(name.substr(start));

    std::string mangledName;
    if (parts.size() > 0) mangledName += "_M" + std::to_string(parts[0].length()) + parts[0];
    if (parts.size() > 1) mangledName += "_C" + std::to_string(parts[1].length()) + parts[1];
    if (parts.size() > 2) mangledName += "_F" + std::to_string(parts[2].length()) + parts[2];
    if (parts.size() > 3) mangledName += "_" + std::to_string(parts[3].length()) + parts[3];

    return mangledName;
}

std::string mangleName(Symbol* sym) {
    if (!sym->forcedMangle.empty()) {
        return sym->forcedMangle;
    }

    return mangleName(sym->mangledName);
}

std::string getMethodName(const std::string& fn) {
    size_t pos = fn.rfind("_F");
    if (pos == std::string::npos) return fn;

    pos += 2;
    size_t lenEnd = pos;

    while (lenEnd < fn.size() && isdigit(fn[lenEnd])) {
        lenEnd++;
    }

    int len = std::stoi(fn.substr(pos, lenEnd - pos));
    return fn.substr(lenEnd, len);
}
