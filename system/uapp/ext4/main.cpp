// Copyright 2017 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <libgen.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include <fbl/unique_free_ptr.h>
#include <fbl/unique_ptr.h>
#include <fs/trace.h>
#include <lib/async-loop/cpp/loop.h>
#include <ext4fs/ext4fs.h>
#include <trace-provider/provider.h>
#include <zircon/compiler.h>
#include <zircon/process.h>
#include <zircon/processargs.h>

namespace {

int do_ext4fs_mount(fbl::unique_fd fd) {
    zx_status_t status;
    zx_handle_t h = zx_take_startup_handle(PA_HND(PA_USER0, 0));
    if (h == ZX_HANDLE_INVALID) {
        FS_TRACE_ERROR("ext4fs: Could not access startup handle to mount point\n");
        return ZX_ERR_BAD_STATE;
    }

    async::Loop loop(&kAsyncLoopConfigNoAttachToThread);
    trace::TraceProvider trace_provider(loop.dispatcher());

    auto loop_quit = [&loop]() { loop.Quit(); };
    if ((status = ext4fs::MountAndServe(fbl::move(fd), loop.dispatcher(), zx::channel(h),
                                fbl::move(loop_quit)) != ZX_OK)) {
        return -1;
    }

    loop.Run();
    return ZX_OK;
}

off_t get_size(int fd) {
    block_info_t info;
    if (ioctl_block_get_info(fd, &info) != sizeof(info)) {
        fprintf(stderr, "error: minfs could not find size of device\n");
        return 0;
    }
    return info.block_size * info.block_count;
}
}//namespace

int main(int argc, char** argv) {
    fbl::unique_fd fd;
    fd.reset(FS_FD_BLOCKDEVICE);

    off_t size = get_size(fd.get());
    if (size == 0) {
        fprintf(stderr, "ext4fs: failed to access block device\n");
        return -1;
    }

    return do_ext4fs_mount(fbl::move(fd));
}

