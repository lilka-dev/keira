#include "keira/service.h"
Service::Service() {
    setktStackSize(4096);
    setktPriority(KT_PRIO_IDLE);
    setktType(KT_SERVICE);
}
