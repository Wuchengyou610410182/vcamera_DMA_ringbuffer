// SPDX-License-Identifier: GPL-2.0
#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/platform_device.h>
#include <linux/uaccess.h>
#include <linux/mm.h>
#include <linux/videodev2.h>
#include <linux/vmalloc.h>
#include <linux/slab.h>
#include <linux/kthread.h>
#include <linux/delay.h>

#include <media/v4l2-device.h>
#include <media/v4l2-ioctl.h>
#include <media/v4l2-fh.h>
#include <media/v4l2-mem2mem.h>
#include <media/videobuf2-v4l2.h>
#include <media/videobuf2-dma-contig.h>

#include "pattern_generator.h"

#define VCAM_NAME "vcam_dma_ring"
#define VCAM_WIDTH 640
#define VCAM_HEIGHT 480
#define VCAM_PIXEL_FORMAT V4L2_PIX_FMT_RGB24
#define VCAM_FRAME_SIZE (VCAM_WIDTH * VCAM_HEIGHT * 3)

struct vcam_dev {
    struct v4l2_device v4l2_dev;
    struct video_device vdev;
    struct vb2_queue vb_queue;
    struct mutex lock;

    struct task_struct *thread;
    bool streaming;

    unsigned int sequence;
};

static struct vcam_dev *g_vcam_dev;  // 全域變數，方便退出使用

static int vcam_queue_setup(struct vb2_queue *vq,
                            unsigned int *nbuffers,
                            unsigned int *nplanes,
                            unsigned int sizes[],
                            struct device *alloc_devs[])
{
    *nplanes = 1;
    sizes[0] = VCAM_FRAME_SIZE;
    return 0;
}

static int vcam_buf_prepare(struct vb2_buffer *vb)
{
    vb2_set_plane_payload(vb, 0, VCAM_FRAME_SIZE);
    return 0;
}

static void vcam_buf_queue(struct vb2_buffer *vb)
{
    struct vcam_dev *dev = vb2_get_drv_priv(vb->vb2_queue);
    void *vaddr = vb2_plane_vaddr(vb, 0);

    fill_frame_pattern(vaddr, VCAM_WIDTH, VCAM_HEIGHT);
    dev->sequence++;
    vb2_buffer_done(vb, VB2_BUF_STATE_DONE);
}

static int vcam_start_streaming(struct vb2_queue *vq, unsigned int count)
{
    struct vcam_dev *dev = vb2_get_drv_priv(vq);
    dev->streaming = true;
    return 0;
}

static void vcam_stop_streaming(struct vb2_queue *vq)
{
    struct vcam_dev *dev = vb2_get_drv_priv(vq);
    dev->streaming = false;
}

// queue_release 已經從新版 vb2_ops 移除，不要定義此函式

static const struct vb2_ops vcam_vb2_ops = {
    .queue_setup        = vcam_queue_setup,
    .buf_prepare        = vcam_buf_prepare,
    .buf_queue          = vcam_buf_queue,
    .start_streaming    = vcam_start_streaming,
    .stop_streaming     = vcam_stop_streaming,
    .wait_prepare       = vb2_ops_wait_prepare,
    .wait_finish        = vb2_ops_wait_finish,
};

static int vcam_open(struct file *file)
{
    return v4l2_fh_open(file);
}

static int vcam_release(struct file *file)
{
    return v4l2_fh_release(file);
}

static int vcam_mmap(struct file *file, struct vm_area_struct *vma)
{
    struct video_device *vdev = video_devdata(file);
    struct vcam_dev *dev = video_get_drvdata(vdev);
    return vb2_mmap(&dev->vb_queue, vma);
}

static __poll_t vcam_poll(struct file *file, struct poll_table_struct *wait)
{
    struct video_device *vdev = video_devdata(file);
    struct vcam_dev *dev = video_get_drvdata(vdev);
    return vb2_poll(&dev->vb_queue, file, wait);
}

static const struct v4l2_file_operations vcam_fops = {
    .owner = THIS_MODULE,
    .open = vcam_open,
    .release = vcam_release,
    .unlocked_ioctl = video_ioctl2,
    .mmap = vcam_mmap,
    .poll = vcam_poll,
};

static int vcam_vidioc_querycap(struct file *file, void *priv, struct v4l2_capability *cap)
{
    strscpy(cap->driver, VCAM_NAME, sizeof(cap->driver));
    strscpy(cap->card, "VCAM DMA Ring Device", sizeof(cap->card));
    strscpy(cap->bus_info, "platform:vcam", sizeof(cap->bus_info));
    cap->device_caps = V4L2_CAP_VIDEO_CAPTURE | V4L2_CAP_STREAMING;
    cap->capabilities = cap->device_caps | V4L2_CAP_DEVICE_CAPS;
    return 0;
}

static int vcam_vidioc_enum_fmt_vid_cap(struct file *file, void *priv, struct v4l2_fmtdesc *f)
{
    if (f->index > 0)
        return -EINVAL;
    f->pixelformat = VCAM_PIXEL_FORMAT;
    strscpy(f->description, "RGB24", sizeof(f->description));
    return 0;
}

static int vcam_vidioc_g_fmt_vid_cap(struct file *file, void *priv, struct v4l2_format *f)
{
    f->fmt.pix.width = VCAM_WIDTH;
    f->fmt.pix.height = VCAM_HEIGHT;
    f->fmt.pix.pixelformat = VCAM_PIXEL_FORMAT;
    f->fmt.pix.field = V4L2_FIELD_NONE;
    f->fmt.pix.bytesperline = VCAM_WIDTH * 3;
    f->fmt.pix.sizeimage = VCAM_FRAME_SIZE;
    return 0;
}

static const struct v4l2_ioctl_ops vcam_ioctl_ops = {
    .vidioc_querycap       = vcam_vidioc_querycap,
    .vidioc_enum_fmt_vid_cap = vcam_vidioc_enum_fmt_vid_cap,
    .vidioc_g_fmt_vid_cap  = vcam_vidioc_g_fmt_vid_cap,
    .vidioc_reqbufs        = vb2_ioctl_reqbufs,
    .vidioc_querybuf       = vb2_ioctl_querybuf,
    .vidioc_qbuf           = vb2_ioctl_qbuf,
    .vidioc_dqbuf          = vb2_ioctl_dqbuf,
    .vidioc_streamon       = vb2_ioctl_streamon,
    .vidioc_streamoff      = vb2_ioctl_streamoff,
};

static int __init vcam_init(void)
{
    int ret;

    g_vcam_dev = kzalloc(sizeof(*g_vcam_dev), GFP_KERNEL);
    if (!g_vcam_dev)
        return -ENOMEM;

    mutex_init(&g_vcam_dev->lock);

    ret = v4l2_device_register(NULL, &g_vcam_dev->v4l2_dev);
    if (ret)
        goto free_dev;

    strscpy(g_vcam_dev->vdev.name, VCAM_NAME, sizeof(g_vcam_dev->vdev.name));
    g_vcam_dev->vdev.v4l2_dev = &g_vcam_dev->v4l2_dev;
    g_vcam_dev->vdev.fops = &vcam_fops;
    g_vcam_dev->vdev.ioctl_ops = &vcam_ioctl_ops;
    g_vcam_dev->vdev.release = video_device_release_empty;
    g_vcam_dev->vdev.lock = &g_vcam_dev->lock;
    g_vcam_dev->vdev.device_caps = V4L2_CAP_VIDEO_CAPTURE | V4L2_CAP_STREAMING;

    g_vcam_dev->vb_queue.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    g_vcam_dev->vb_queue.io_modes = VB2_MMAP | VB2_USERPTR | VB2_DMABUF;
    g_vcam_dev->vb_queue.drv_priv = g_vcam_dev;
    g_vcam_dev->vb_queue.buf_struct_size = sizeof(struct vb2_v4l2_buffer);
    g_vcam_dev->vb_queue.ops = &vcam_vb2_ops;
    g_vcam_dev->vb_queue.mem_ops = &vb2_dma_contig_memops;
    g_vcam_dev->vb_queue.timestamp_flags = V4L2_BUF_FLAG_TIMESTAMP_MONOTONIC;
    g_vcam_dev->vb_queue.lock = &g_vcam_dev->lock;
    g_vcam_dev->vb_queue.dev = NULL;

    ret = vb2_queue_init(&g_vcam_dev->vb_queue);
    if (ret)
        goto unregister_dev;

    g_vcam_dev->vdev.queue = &g_vcam_dev->vb_queue;

    ret = video_register_device(&g_vcam_dev->vdev, VFL_TYPE_VIDEO, -1);
    if (ret)
        goto unregister_dev;

    video_set_drvdata(&g_vcam_dev->vdev, g_vcam_dev);

    pr_info("%s: virtual camera device registered\n", VCAM_NAME);
    return 0;

unregister_dev:
    v4l2_device_unregister(&g_vcam_dev->v4l2_dev);
free_dev:
    kfree(g_vcam_dev);
    return ret;
}

static void __exit vcam_exit(void)
{
    video_unregister_device(&g_vcam_dev->vdev);
    v4l2_device_unregister(&g_vcam_dev->v4l2_dev);
    kfree(g_vcam_dev);
}

module_init(vcam_init);
module_exit(vcam_exit);

MODULE_DESCRIPTION("Virtual Camera with DMA Ring Buffer");
MODULE_AUTHOR("WU");
MODULE_LICENSE("GPL");

