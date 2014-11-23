#include "nameobj.h"

using namespace std;

namespace dzy {

NameObj::NameObj() {
}

NameObj::NameObj(const string& name) : mName(name) {
}

NameObj::NameObj(const NameObj& other) : mName(other.mName) {
}

NameObj& NameObj::operator=(const NameObj& other) {
    if (this != &other) mName = other.mName;
    return *this;
}

void NameObj::setName(const string& name) {
    mName = name;
}

const string& NameObj::getName() {
    return mName;
}

const string& NameObj::getName() const {
    return mName;
}

}
