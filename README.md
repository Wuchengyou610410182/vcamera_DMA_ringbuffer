# vcamera_DMA_ringbuffer
This project is a Linux kernel module implementing a virtual camera driver based on the V4L2 framework. It supports mmap/userptr/dmabuf via videobuf2 and simulates 640x480 YUYV video at ~30FPS using a DMA ring buffer and kernel thread. Compatible with ffmpeg/OpenCV, it aids V4L2 testing and Camera HAL prototyping.
vcam-dma-ring
A virtual camera Linux kernel module using a DMA ring buffer architecture to simulate video frame output.
Implemented on top of the Linux V4L2 framework with videobuf2 buffer management.

Project Overview
vcam-dma-ring is a Linux kernel module that simulates a virtual camera device using contiguous DMA memory and a ring buffer to manage video buffers.
It is designed for multimedia driver learning, virtual device testing, and video data simulation.

Features
Video buffer management based on videobuf2-v4l2 and videobuf2-dma-contig

Supports V4L2 standard video format: RGB24, 640x480 resolution

Provides mmap interface for user-space access to frame buffers

Ring buffer mechanism to ensure smooth data streaming

Generates test video frames via the included pattern_generator module

Requirements
Ubuntu 22.04 LTS (Recommended kernel version 5.15 LTS or any kernel with videobuf2 support)

Linux kernel headers matching the running kernel version

GCC 12.x (matching the compiler used for building the kernel)

Root privileges for building and loading kernel modules


Build and Installation

# Verify current kernel version
uname -r

# Install build dependencies and headers
sudo apt update
sudo apt install build-essential linux-headers-$(uname -r)

# Compile the modules
make

# Load dependent modules (if available)
sudo modprobe videobuf2_core
sudo modprobe videobuf2_dma_contig
sudo modprobe videodev

# Insert the modules
sudo insmod pattern_generator.ko
sudo insmod vcam_dma_ring.ko

Testing
After loading the module, a video device /dev/videoX will be created (where X is the device number)

Use v4l2-ctl, VLC, ffplay, or similar tools to capture and display video from the virtual camera

Verify the output video frames are in RGB24 format at 640x480 resolution

Notes
Ensure the kernel includes videobuf2 modules to avoid “Unknown symbol” errors

Using an official stable LTS kernel and installing the matching linux-modules-extra package is highly recommended

This driver is for learning and demonstration purposes and is not optimized for production hardware performance
