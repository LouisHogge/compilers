#ifndef UTILS_HPP
#define UTILS_HPP

#include <iostream>
#include <string>

inline void error(const std::string& filename, const int line, const int column, const std::string& m) {

    std::cerr << filename << ":"
         << line << ":" 
         << column << ": "
         << " semantic error: "
         << m
         << std::endl;

         exit(1);
}

#endif // UTILS_HPP