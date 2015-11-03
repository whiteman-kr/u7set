#pragma once

#define AUTO_LOCK(mutex) QMutexLocker m(&mutex);

