#include "CPython.h"
#include "logger.h"

void EnableListener(EventCode evc) {
	enableEvent.emplace(evc, true);
}
