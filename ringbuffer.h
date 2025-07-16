#ifndef VCAM_RINGBUFFER_H
#define VCAM_RINGBUFFER_H

#include <linux/types.h>
#include <linux/errno.h>

#define RBUF_SIZE 16

struct ringbuffer {
    void *bufs[RBUF_SIZE];
    u32 head;
    u32 tail;
    u32 count;
};

static inline void ringbuffer_init(struct ringbuffer *rb) {
    rb->head = 0;
    rb->tail = 0;
    rb->count = 0;
}

static inline bool ringbuffer_is_empty(struct ringbuffer *rb) {
    return rb->count == 0;
}

static inline bool ringbuffer_is_full(struct ringbuffer *rb) {
    return rb->count == RBUF_SIZE;
}

static inline int ringbuffer_enqueue(struct ringbuffer *rb, void *item) {
    if (ringbuffer_is_full(rb))
        return -ENOMEM;

    rb->bufs[rb->tail] = item;
    rb->tail = (rb->tail + 1) % RBUF_SIZE;
    rb->count++;
    return 0;
}

static inline void *ringbuffer_dequeue(struct ringbuffer *rb) {
    void *item;
    if (ringbuffer_is_empty(rb))
        return NULL;

    item = rb->bufs[rb->head];
    rb->head = (rb->head + 1) % RBUF_SIZE;
    rb->count--;
    return item;
}

#endif // VCAM_RINGBUFFER_H

