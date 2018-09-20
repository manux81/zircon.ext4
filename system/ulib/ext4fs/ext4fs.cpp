// Copyright 2018 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <fcntl.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include <bitmap/raw-bitmap.h>
#include <fbl/algorithm.h>
#include <fbl/alloc_checker.h>
#include <fbl/auto_call.h>
#include <fbl/limits.h>
#include <fbl/unique_ptr.h>
#include <fs/block-txn.h>
#include <fs/trace.h>

#ifdef __Fuchsia__
#include <fbl/auto_lock.h>
#include <lib/async/cpp/task.h>
#include <lib/zx/event.h>

#endif

#include <ext4fs/ext4fs.h>
#include <ext4fs/types.h>


// #define DEBUG_PRINTF
#ifdef DEBUG_PRINTF
#define xprintf(args...) fprintf(stderr, args)
#else
#define xprintf(args...)
#endif

namespace ext4fs {

constexpr const char kFsName[] = "ext4fs";

zx_status_t ext4fs_mount(fbl::unique_fd blockfd, fbl::RefPtr<VnodeExt4>* root_out) {
    TRACE_DURATION("ext4fs", "ext4fs_mount");
    zx_status_t status;
    ext4fs_info_t *info = nullptr;

    fbl::unique_ptr<Ext4fs> fs;
    if ((status = Ext4fs::Create(fbl::move(blockfd), info, &fs)) != ZX_OK) {
        FS_TRACE_ERROR("ext4fs: mount failed\n");
        return status;
    }

    fbl::RefPtr<VnodeExt4> vn;
    if ((status = fs->VnodeGet(&vn, kExt4fsInodeRootIndex)) != ZX_OK) {
        FS_TRACE_ERROR("ext4fs: cannot find root inode\n");
        return status;
    }

    ZX_DEBUG_ASSERT(vn->IsDirectory());
    __UNUSED auto r = fs.release();
    *root_out = fbl::move(vn);

    return ZX_OK;
}


#ifdef __Fuchsia__
zx_status_t MountAndServe(fbl::unique_fd fd, async_dispatcher_t* dispatcher, zx::channel mount_channel,
                          fbl::Closure on_unmount) {
    TRACE_DURATION("ext4fs", "MountAndServe");

    fbl::RefPtr<VnodeExt4> vn;
    zx_status_t status = ext4fs_mount(fbl::move(fd), &vn);
    if (status != ZX_OK) {
        return status;
    }

    Ext4fs* vfs = vn->fs_;
    //vfs->SetReadonly(options->readonly);
    //vfs->SetMetrics(options->metrics);
    vfs->SetUnmountCallback(fbl::move(on_unmount));
    vfs->SetDispatcher(dispatcher);
    return vfs->ServeDirectory(fbl::move(vn), fbl::move(mount_channel));
}

#endif

VnodeExt4::~VnodeExt4() {
}

void VnodeExt4::fbl_recycle() {
    //ZX_DEBUG_ASSERT(fd_count_ == 0);
    if (!IsUnlinked()) {
        // If this node has not been purged already, remove it from the
        // hash map. If it has been purged; it will already be absent
        // from the map (and may have already been replaced with a new
        // node, if the inode has been re-used).
        fs_->VnodeRelease(this);
    }
    delete this;
}


bool VnodeExt4::IsDirectory() const
{
	return false;
}

bool VnodeExt4::IsUnlinked() const
{
	return false;
}

zx_status_t VnodeExt4::Open(uint32_t flags, fbl::RefPtr<Vnode>* out_redirect) {
    return ZX_OK;
}

zx_status_t VnodeExt4::Ioctl(uint32_t op, const void* in_buf, size_t in_len, void* out_buf,
                              size_t out_len, size_t* out_actual) {
    switch (op) {
        case IOCTL_VFS_QUERY_FS: {
            if (out_len < (sizeof(vfs_query_info_t) + strlen(kFsName))) {
                return ZX_ERR_INVALID_ARGS;
            }

            vfs_query_info_t* info = static_cast<vfs_query_info_t*>(out_buf);
            memset(info, 0, sizeof(*info));

            info->block_size = kExt4fsMinBlockSize;
            info->max_filename_size = kExt4fsMaxNameSize;
            info->fs_type = VFS_TYPE_EXT4FS;
#if 0
#ifdef __Fuchsia__
            info->fs_id = fs_->GetFsId();
#endif
            info->total_bytes = fs_->Info().block_count * fs_->Info().block_size;
            info->used_bytes = fs_->Info().alloc_block_count * fs_->Info().block_size;
            info->total_nodes = fs_->Info().inode_count;
            info->used_nodes = fs_->Info().alloc_inode_count;
#endif
            memcpy(info->name, kFsName, strlen(kFsName));
            *out_actual = sizeof(vfs_query_info_t) + strlen(kFsName);
            return ZX_OK;
        }
#ifdef __Fuchsia__
        case IOCTL_VFS_GET_DEVICE_PATH: {
        }
#endif
        default: {
            return ZX_ERR_NOT_SUPPORTED;
        }
    }
}

zx_status_t VnodeExt4::Close() {
	return ZX_OK;
}

zx_status_t VnodeExt4::ValidateFlags(uint32_t flags) {
            return ZX_OK;
}

zx_status_t VnodeExt4::Readdir(fs::vdircookie_t* cookie, void* dirents, size_t len,
		    size_t* out_actual) {
            return ZX_OK;
}

zx_status_t VnodeExt4::Read(void* data, size_t len, size_t off, size_t* out_actual) {
            return ZX_OK;
}

zx_status_t VnodeExt4::Write(const void* data, size_t len, size_t offset,
		    size_t* out_actual) {
            return ZX_OK;
}

zx_status_t VnodeExt4::Append(const void* data, size_t len, size_t* out_end,
		    size_t* out_actual) {
            return ZX_OK;
}

zx_status_t VnodeExt4::Lookup(fbl::RefPtr<fs::Vnode>* out, fbl::StringPiece name) {
            return ZX_OK;
}

zx_status_t VnodeExt4::Getattr(vnattr_t* a) {
            return ZX_OK;
}

zx_status_t VnodeExt4::Create(fbl::RefPtr<fs::Vnode>* out, fbl::StringPiece name,
		    uint32_t mode) {
            return ZX_OK;
}

zx_status_t VnodeExt4::Truncate(size_t len) {
            return ZX_OK;
}

zx_status_t VnodeExt4::Unlink(fbl::StringPiece name, bool must_be_dir) {
            return ZX_OK;
}

zx_status_t VnodeExt4::GetVmo(int flags, zx_handle_t* out) {
            return ZX_OK;
}

/* Class Ext4fs */
#if 0
zx_status_t readblk(blk_t bno, void* data) {

    off_t off = static_cast<off_t>(bno) * 4096;
    assert(off / 4096 == bno); // Overflow

#ifndef __Fuchsia__
    off += offset_;
#endif

    if (lseek(fd_.get(), off, SEEK_SET) < 0) {
        FS_TRACE_ERROR("minfs: cannot seek to block %u\n", bno);
        return ZX_ERR_IO;
    }
    if (read(fd_.get(), data, kMinfsBlockSize) != kMinfsBlockSize) {
        FS_TRACE_ERROR("minfs: cannot read block %u\n", bno);
        return ZX_ERR_IO;
    }
    return ZX_OK;
}
#endif

zx_status_t block_read_bytes_direct(int fd, off_t offset, uint32_t size,
				    void *data) {
	if (lseek(fd, offset, SEEK_SET) < 0) {
	//	FS_TRACE_ERROR("ext4fs: cannot seek to offset %u\n", offset);
		return ZX_ERR_IO;
	}

	if (read(fd, data, size) != size) {
	//	FS_TRACE_ERROR("ext4fs: cannot read bytes.\n");
		return ZX_ERR_IO;
	}

	return ZX_OK;
}

zx_status_t ext4_superblock_read_direct(int fd, ext4_superblock_t **sb)
{
	void *data = malloc(kExt4fsSuperblockSize);
	if (data == nullptr)
		return ZX_ERR_NO_MEMORY;

	/* Read data from block */
	zx_status_t rc = block_read_bytes_direct(fd, kExt4fsSuperblockOffset,
	    kExt4fsSuperblockSize, data);

	if (rc != ZX_OK) {
		free(data);
		goto exit;
	}

	(*sb) = (ext4_superblock_t *)data;

exit:
	return rc;
}

zx_status_t Ext4fs::Create(fbl::unique_fd blockfd, const ext4fs_info_t* info,
		    fbl::unique_ptr<Ext4fs>* out) {
	zx_status_t rc = ZX_OK;
	ext4_superblock_t *temp_superblock = nullptr;

	rc = ext4_superblock_read_direct(blockfd.get(), &temp_superblock);
	if (rc != ZX_OK)
		goto exit;	
	

exit:
	return rc;
}

zx_status_t Ext4fs::VnodeGet(fbl::RefPtr<VnodeExt4>* out, ino_t ino) {
	return ZX_OK;
}

#ifdef __Fuchsia__
void Ext4fs::Shutdown(fs::Vfs::ShutdownCallback cb) {
#if 0
    ManagedVfs::Shutdown([this, cb = fbl::move(cb)](zx_status_t status) mutable {
        Sync([this, cb = fbl::move(cb)](zx_status_t) mutable {
            async::PostTask(dispatcher(), [this, cb = fbl::move(cb)]() mutable {
                // Ensure writeback buffer completes before auxilliary structures
                // are deleted.
                writeback_ = nullptr;
                bc_->Sync();

                DumpMetrics();

                auto on_unmount = fbl::move(on_unmount_);

                // Explicitly delete this (rather than just letting the memory release when
                // the process exits) to ensure that the block device's fifo has been
                // closed.
                delete this;

                // Identify to the unmounting channel that teardown is complete.
                cb(ZX_OK);

                // Identify to the unmounting thread that teardown is complete.
                if (on_unmount) {
                    on_unmount();
                }
            });
        });
    });
#endif
}

void Ext4fs::VnodeRelease(VnodeExt4* vn) {

}

#endif

} // namespace ext4fs


