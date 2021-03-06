// Copyright 2017 The Fuchsia Authors
//
// Use of this source code is governed by a MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT

#include <inttypes.h>

#include <trace.h>

#include <object/process_dispatcher.h>
#include <zircon/types.h>

#include "priv.h"

#define LOCAL_TRACE 0

zx_status_t sys_futex_wait(user_in_ptr<const zx_futex_t> value_ptr, int32_t current_value, zx_time_t deadline) {
    LTRACEF("futex %p current %d\n", value_ptr.get(), current_value);

    return ProcessDispatcher::GetCurrent()->futex_context()->FutexWait(
        value_ptr, current_value, deadline);
}

zx_status_t sys_futex_wake(user_in_ptr<const zx_futex_t> value_ptr, uint32_t count) {
    LTRACEF("futex %p count %" PRIu32 "\n", value_ptr.get(), count);

    return ProcessDispatcher::GetCurrent()->futex_context()->FutexWake(
        value_ptr, count);
}

zx_status_t sys_futex_requeue(user_in_ptr<const zx_futex_t> wake_ptr, uint32_t wake_count, int32_t current_value,
                              user_in_ptr<const zx_futex_t> requeue_ptr, uint32_t requeue_count) {
    LTRACEF("futex %p wake_count %" PRIu32 "current_value %d "
           "requeue_futex %p requeue_count %" PRIu32 "\n",
           wake_ptr.get(), wake_count, current_value, requeue_ptr.get(), requeue_count);

    return ProcessDispatcher::GetCurrent()->futex_context()->FutexRequeue(
        wake_ptr, wake_count, current_value,
        requeue_ptr, requeue_count);
}
