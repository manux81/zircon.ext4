// Copyright 2018 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <inttypes.h>

#include <fbl/algorithm.h>
#include <fbl/function.h>
#include <fbl/intrusive_hash_table.h>
#include <fbl/intrusive_single_list.h>
#include <fbl/macros.h>
#include <fbl/ref_ptr.h>
#include <fbl/unique_ptr.h>
#include <fbl/unique_fd.h>

#ifdef __Fuchsia__
#include <lib/async/dispatcher.h>
#endif

#include <fs/queue.h>
#include <fs/vfs.h>
#include <fs/block-txn.h>
#include <lib/fzl/mapped-vmo.h>
#include <fs/ticker.h>
#include <fs/trace.h>
#include <fs/vfs.h>
#include <fs/vnode.h>

#include "ext4fs/types.h"

namespace ext4fs {

class Ext4fs;
class VnodeExt4;

#ifdef __Fuchsia__

zx_status_t MountAndServe(fbl::unique_fd fd, async_dispatcher_t* dispatcher,
                          zx::channel mount_channel, fbl::Closure on_unmount);

#endif

class VnodeExt4 final : public fs::Vnode,
			public fbl::SinglyLinkedListable<VnodeExt4*>,
			public fbl::Recyclable<VnodeExt4> {
public:
	// Constructs the "directory" ext4
	VnodeExt4(Ext4fs* es);
	virtual ~VnodeExt4();
	void fbl_recycle() final;

    	bool IsDirectory() const;
	bool IsUnlinked() const;
	// TODO(rvargas): Make private.
	Ext4fs* const fs_;

private:
	// Vnode I/O operations

	zx_status_t Open(uint32_t flags, fbl::RefPtr<Vnode>* out_redirect) final;

	zx_status_t Close() final;

	zx_status_t Ioctl(uint32_t op, const void* in_buf, size_t in_len, void* out_buf,
			    size_t out_len, size_t* out_actual) final;

	zx_status_t ValidateFlags(uint32_t flags) final;

	zx_status_t Readdir(fs::vdircookie_t* cookie, void* dirents, size_t len,
			    size_t* out_actual) final;

	zx_status_t Read(void* data, size_t len, size_t off, size_t* out_actual) final;

	zx_status_t Write(const void* data, size_t len, size_t offset,
			    size_t* out_actual) final;

	zx_status_t Append(const void* data, size_t len, size_t* out_end,
			    size_t* out_actual) final;

	zx_status_t Lookup(fbl::RefPtr<fs::Vnode>* out, fbl::StringPiece name) final;

	zx_status_t Getattr(vnattr_t* a) final;

	zx_status_t Create(fbl::RefPtr<fs::Vnode>* out, fbl::StringPiece name,
			    uint32_t mode) final;

	zx_status_t Truncate(size_t len) final;

	zx_status_t Unlink(fbl::StringPiece name, bool must_be_dir) final;

	zx_status_t GetVmo(int flags, zx_handle_t* out) final;
};

class Ext4fs final : public fs::Vfs, public fbl::RefCounted<Ext4fs> {
public:
	DISALLOW_COPY_ASSIGN_AND_MOVE(Ext4fs);
	friend class VnodeExt4;

	static zx_status_t Create(fbl::unique_fd blockfd, const ext4fs_info_t* info,
				  fbl::unique_ptr<Ext4fs>* out);

	zx_status_t VnodeGet(fbl::RefPtr<VnodeExt4>* out, ino_t ino);
	void VnodeRelease(VnodeExt4* vn);

#ifdef __Fuchsia__
	void Shutdown(fs::Vfs::ShutdownCallback cb);
	void SetUnmountCallback(fbl::Closure closure) { on_unmount_ = fbl::move(closure); }

private:
	fbl::Closure on_unmount_{};
#endif
};

} // namespace ext4fs
