#include <memory>
#include "native_app.h"

using namespace dzy;
using namespace std;

class SViewApp : public NativeApp {
};

namespace dzy {

NativeApp* nativeInit() {
    //shared_ptr<NativeApp> napp(new SViewApp);
    //return napp.get();
    return new SViewApp;
}

} // namespace dzy
