find compiler runtime standard \
  \( -path "*/extern/*" -o -path "*/build/*" \) -prune -o \
  \( -name '*.cpp' -o -name '*.hpp' -o -name '*.c' -o -name '*.h' \) \
  -print | xargs -n 1 -P 4 clang-format-15 -i

