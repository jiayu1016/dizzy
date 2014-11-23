#ifndef NAMEOBJ_H
#define NAMEOBJ_H

#include <string>

namespace dzy {

class NameObj {
public:
    NameObj();
    NameObj(const std::string& name);
    NameObj(const NameObj& other);
    NameObj& operator=(const NameObj& other);

    void setName(const std::string& name);
    const std::string& getName();
    const std::string& getName() const;
protected:
    std::string mName;
};

}
#endif
