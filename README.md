# vcamera_DMA_ringbuffer
This project is a Linux kernel module implementing a virtual camera driver based on the V4L2 framework. It supports mmap/userptr/dmabuf via videobuf2 and simulates 640x480 YUYV video at ~30FPS using a DMA ring buffer and kernel thread. Compatible with ffmpeg/OpenCV, it aids V4L2 testing and Camera HAL prototyping.
