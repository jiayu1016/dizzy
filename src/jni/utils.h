#ifndef UTILS_H
#define UTILS_H

namespace dzy {

struct RawBufferDeleter {
    void operator ()(char *p) { delete[] p; }
};

}

#endif
