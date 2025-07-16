#include <linux/module.h>
#define INCLUDE_VERMAGIC
#include <linux/build-salt.h>
#include <linux/elfnote-lto.h>
#include <linux/export-internal.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

#ifdef CONFIG_UNWINDER_ORC
#include <asm/orc_header.h>
ORC_HEADER;
#endif

BUILD_SALT;
BUILD_LTO_INFO;

MODULE_INFO(vermagic, VERMAGIC_STRING);
MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(".gnu.linkonce.this_module") = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

#ifdef CONFIG_RETPOLINE
MODULE_INFO(retpoline, "Y");
#endif



static const struct modversion_info ____versions[]
__used __section("__versions") = {
	{ 0x4c03a563, "random_kmalloc_seed" },
	{ 0x29ac0981, "kmalloc_caches" },
	{ 0xdc327b3c, "kmalloc_trace" },
	{ 0xcefb0c9f, "__mutex_init" },
	{ 0x64fddeea, "v4l2_device_register" },
	{ 0x2083a0dd, "video_device_release_empty" },
	{ 0xb3df8977, "vb2_dma_contig_memops" },
	{ 0x9687efc, "vb2_queue_init" },
	{ 0xef34c52e, "__video_register_device" },
	{ 0x122c3a7e, "_printk" },
	{ 0x79424517, "vb2_ioctl_reqbufs" },
	{ 0xca713b39, "vb2_ioctl_querybuf" },
	{ 0x406ec47d, "vb2_ioctl_qbuf" },
	{ 0x16894676, "vb2_ioctl_dqbuf" },
	{ 0xec603ef6, "vb2_ioctl_streamon" },
	{ 0x12ac5d7c, "vb2_ioctl_streamoff" },
	{ 0xfb1e3992, "video_ioctl2" },
	{ 0x95f28ba7, "vb2_ops_wait_prepare" },
	{ 0x861a7d11, "vb2_ops_wait_finish" },
	{ 0xbdfb6dbb, "__fentry__" },
	{ 0x5b8239ca, "__x86_return_thunk" },
	{ 0xa2aabbda, "vb2_plane_vaddr" },
	{ 0xdacb000f, "fill_frame_pattern" },
	{ 0xd2e370f6, "vb2_buffer_done" },
	{ 0xbac5ec56, "v4l2_fh_release" },
	{ 0x123b36fb, "v4l2_fh_open" },
	{ 0x90f23f17, "video_devdata" },
	{ 0xfa44dfac, "vb2_mmap" },
	{ 0x7611fd59, "vb2_poll" },
	{ 0xc10b30a6, "video_unregister_device" },
	{ 0xca100865, "v4l2_device_unregister" },
	{ 0x37a0cba, "kfree" },
	{ 0xb2b23fc2, "module_layout" },
};

MODULE_INFO(depends, "videodev,videobuf2-dma-contig,videobuf2-v4l2,videobuf2-common,pattern_generator");


MODULE_INFO(srcversion, "774CB20E89CAC31310E18FF");
