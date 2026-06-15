#ifndef OBJECT_LUBEX_HPP
#define OBJECT_LUBEX_HPP

#include "emiter.hpp"
#include "config.hpp"
#include <filesystem>

bool emitObjectFile(llvm::Module& module,
                    const std::string& triple,
                    const std::filesystem::path& output,
                    ProjectConfig config);

#endif // OBJECT_LUBEX_HPP