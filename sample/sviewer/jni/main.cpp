#include <memory>
#include "native_app.h"

using namespace dzy;
using namespace std;

class SViewApp : public NativeApp {
};

NativeApp *initSystem() {
    std::shared_ptr<NativeApp> sp;
    return new SViewApp();
}
