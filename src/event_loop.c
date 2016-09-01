#include <event_loop.h>

#include <sys/epoll.h>

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <errno.h>
#include <stdbool.h>
#include <string.h>

#define AE_FILE_EVENTS  1
#define AE_TIME_EVENTS  2
#define AE_ALL_EVENTS   (AE_FILE_EVENTS|AE_TIME_EVENTS)
#define AE_DONT_WAIT    4

typedef struct epoll_state_t {
    int epfd;
    struct epoll_event* events;
} EpollState;

static int api_epoll_create(EventLoop* loop) {
    EpollState* self = malloc(sizeof(EpollState));

    self->events = malloc(sizeof(struct epoll_event) * loop->set_size);

    self->epfd = epoll_create(1024);/* 1024 is just an hint for the kernel */
    if (self->epfd == -1) {
        free(self->events);
        free(self);
        return -1;
    }
    loop->api_data = self;
    return 0;
}

static int api_epoll_add_event(EventLoop* loop, int fd, int mask) {
    EpollState* state = loop->api_data;
    struct epoll_event ee;
    /* If the fd was already monitored for some event, we need a MOD
     * operation. Otherwise we need an ADD operation. */
    // EPOLL_CTL_ADD == 1
    // EPOLL_CTL_MOD == 3
    int op = loop->events[fd].mask == AE_NONE ? EPOLL_CTL_ADD : EPOLL_CTL_MOD;
    ee.events = 0;

    mask |= loop->events[fd].mask; /* Merge old events */
    if (mask & AE_READABLE) {
        ee.events |= EPOLLIN;
    }
    if (mask & AE_WRITABLE) {
        ee.events |= EPOLLOUT;
    }
    ee.data.u64 = 0; /* avoid valgrind warning */
    ee.data.fd = fd;
    if (epoll_ctl(state->epfd, op, fd, &ee) == -1) {
        return -1;
    }
    return 0;
}

static int api_epoll_poll(EventLoop* loop, struct timeval* tvp) {
    EpollState* state = loop->api_data;
    int num_events = 0;
    int retval = epoll_wait(state->epfd, state->events, loop->set_size, tvp ? (tvp->tv_sec * 1000 + tvp->tv_usec / 1000) : -1);
    // 这里原来为-1，一直等待，直到有事件发生,但是如果用户创建了一个空的loop，程序就会陷入无限等待，无法退出
    //int retval = epoll_wait(state->epfd, state->events, loop->set_size, tvp ? (tvp->tv_sec * 1000 + tvp->tv_usec / 1000) : 10);

    if (retval > 0) {
        num_events = retval;
        for (int j = 0; j < num_events; j++) {
            int mask = 0;
            struct epoll_event *e = state->events + j;

            if (e->events & EPOLLIN)  mask |= AE_READABLE;
            if (e->events & EPOLLOUT) mask |= AE_WRITABLE;
            if (e->events & EPOLLERR) mask |= AE_WRITABLE;
            if (e->events & EPOLLHUP) mask |= AE_WRITABLE;

            loop->fired[j].fd = e->data.fd;
            loop->fired[j].mask = mask;
        }
    }
    return num_events;
}

static void api_epoll_del_event(EventLoop* loop, int fd, int delmask) {
    EpollState* state = loop->api_data;
    struct epoll_event ee;
    // 获取不删除的mask
    int mask = loop->events[fd].mask & (~delmask);

    ee.events = 0;
    // 如果仍然可读，events添加读事件
    if (mask & AE_READABLE) ee.events |= EPOLLIN;
    // 如果仍然可写，events添加写事件
    if (mask & AE_WRITABLE) ee.events |= EPOLLOUT;

    ee.data.u64 = 0;
    ee.data.fd = fd;

    if (mask == AE_NONE) {
        /* Note, Kernel < 2.6.9 requires a non null event pointer even for
         * EPOLL_CTL_DEL. 
         */
        epoll_ctl(state->epfd, EPOLL_CTL_DEL, fd, &ee);
    } else {
        epoll_ctl(state->epfd, EPOLL_CTL_MOD, fd, &ee);
    }
}

static void get_time(int32_t *seconds, int32_t *milliseconds) {
    struct timeval tv;

    gettimeofday(&tv, NULL);
    *seconds = tv.tv_sec;
    *milliseconds = tv.tv_usec / 1000;
}

static void add_milliseconds_to_now(
        int64_t milliseconds,
        int32_t* sec,
        int32_t* ms) {
    int32_t cur_sec = 0, cur_ms = 0, when_sec = 0, when_ms = 0;

    get_time(&cur_sec, &cur_ms);

    when_sec = cur_sec + milliseconds / 1000;
    when_ms = cur_ms + milliseconds % 1000;
    if (when_ms >= 1000) {
        when_sec++;
        when_ms -= 1000;
    }
    *sec = when_sec;
    *ms = when_ms;
}

EventLoop* event_loop_create(int set_size) {
    EventLoop* self = malloc(sizeof(EventLoop));

    size_t events_size = sizeof(FileEvent) * set_size;
    self->events = malloc(events_size);
    bzero(self->events, events_size);

    size_t fired_size = sizeof(FiredEvent) * set_size;
    self->fired = malloc(fired_size);
    bzero(self->fired, fired_size);

    self->set_size = set_size;
    self->last_time = time(NULL);
    self->time_event_head = NULL;
    self->time_event_next_id = 0;
    self->stop = false;
    self->max_fd = -1;
    self->before = NULL;
    self->after = NULL;

    if (api_epoll_create(self) == -1) {
        goto error;
    }
    /* Events with mask == AE_NONE are not set. So let's initialize the
     * vector with it. */
    for (int i = 0; i < set_size; i++) {
        self->events[i].mask = AE_NONE;
    }
    return self;

    error:
    free(self->events);
    free(self->fired);
    free(self);
    return NULL;
}

void event_loop_set_before(EventLoop* loop, BeforeFunc* before) {
    loop->before = before;
}

void event_loop_set_after(EventLoop* loop, AfterFunc* after) {
    loop->after = after;
}

int event_loop_add_file_event(EventLoop* loop, int fd, int mask, FileFunc* func,
        void* user_data) {
    if (fd >= loop->set_size) {
        errno = ERANGE;
        return AE_ERR;
    }

    FileEvent* fevent = &loop->events[fd];
    if (api_epoll_add_event(loop, fd, mask) == -1) {
        return AE_ERR;
    }

    fevent->mask |= mask;
    if (mask & AE_READABLE) {
        fevent->read = func;
    }
    if (mask & AE_WRITABLE) {
        fevent->write = func;
    }
    fevent->user_data = user_data;
    if (fd > loop->max_fd) {
        loop->max_fd = fd;
    }
    return AE_OK;
}

void event_loop_del_file_event(EventLoop* loop, int fd, int mask) {
    if(fd >= loop->set_size) {
        return;
    }

    FileEvent* fe = &loop->events[fd];
    if(fe->mask == AE_NONE) {
        return;
    }

    api_epoll_del_event(loop, fd, mask);
    fe->mask = fe->mask & (~mask);
    if (fd == loop->max_fd && fe->mask == AE_NONE) {
        /*Update the max fd*/
        for(int j = loop->max_fd - 1; j >= 0; j--) {
            if (loop->events[j].mask != AE_NONE){
                break;
            }
            loop->max_fd = j;
        }
    }
}

int64_t event_loop_add_time_event(
        EventLoop* loop, 
        int64_t milliseconds,
        TimeFunc* func, 
        void* user_data, 
        FinalizeFunc* finalize) {
    int64_t id = loop->time_event_next_id++;
    TimeEvent* tevent = malloc(sizeof(TimeEvent));

    tevent->id = id;
    add_milliseconds_to_now(milliseconds, &tevent->when_sec, &tevent->when_ms);
    tevent->func = func;
    tevent->finalize = finalize;
    tevent->user_data = user_data;
    tevent->next = loop->time_event_head;
    loop->time_event_head = tevent;
    return id;
}

int event_loop_del_time_event(EventLoop* loop, int64_t id) {
    TimeEvent *te, *prev = NULL;

    te = loop->time_event_head;
    while (te) {
        if (te->id == id) {
            if (prev == NULL) {
                loop->time_event_head = te->next;
            } else {
                prev->next = te->next;
            }
            if (te->finalize) {
                te->finalize(loop, te->user_data);
            }
            free(te);
            return AE_OK;
        }
        prev = te;
        te = te->next;
    }
    return AE_ERR; /* NO event with the specified ID found */
}

static TimeEvent* search_nearest_timer(EventLoop* loop) {
    TimeEvent* te = loop->time_event_head;
    TimeEvent* nearest = NULL;

    while (te != NULL) {
        if (!nearest
                || te->when_sec < nearest->when_sec
                || (te->when_sec == nearest->when_sec && te->when_ms < nearest->when_ms)) {
            nearest = te;
        }
        te = te->next;
    }
    return nearest;
}

static int process_time_events(EventLoop* loop){
    int processed = 0;
    TimeEvent *te;
    time_t now = time(NULL);
    /* If the system clock is moved to the future, and then set back to the
     * right value, time events may be delayed in a random way. Often this
     * means that scheduled operations will not be performed soon enough.
     *
     * Here we try to detect system clock skews, and force all the time
     * events to be processed ASAP when this happens: the idea is that
     * processing events earlier is less dangerous than delaying them
     * indefinitely, and practice suggests it is. */
    if (now < loop->last_time) {
        te = loop->time_event_head;
        while (te) {
            te->when_sec = 0;
            te = te->next;
        }
    }
    loop->last_time = now;

    te = loop->time_event_head;

    int max_id = loop->time_event_next_id;

    while(te) {
        int32_t now_sec, now_ms;
        int64_t id;
        if (te->id > max_id)
        {
            te = te->next;
            continue;
        }
        get_time(&now_sec, &now_ms);
        if (now_sec > te->when_sec
                || (now_sec == te->when_sec && now_ms >= te->when_ms)) {
            int retval;

            id = te->id;
            retval = te->func(loop, id, te->user_data);
            processed++;
            /* After an event is processed our time event list may
             * no longer be the same, so we restart from head.
             * Still we make sure to don't process events registered
             * by event handlers itself in order to don't loop forever.
             * To do so we saved the max ID we want to handle.
             *
             * FUTURE OPTIMIZATIONS:
             * Note that this is NOT great algorithmically. Redis uses
             * a single time event so it's not a problem but the right
             * way to do this is to add the new elements on head, and
             * to flag deleted elements in a special way for later
             * deletion (putting references to the nodes to delete into
             * another linked list). */
            if (retval != AE_NOMORE) {
                add_milliseconds_to_now(retval, &te->when_sec, &te->when_ms);
            } else {
                event_loop_del_time_event(loop, id);
            }
            te = loop->time_event_head;
        } else {
            te = te->next;
        }
    }
    return processed;
}

int event_loop_proccess_events(EventLoop* loop, int flags) {
    int processed = 0, num_events;

    /* Nothing to do? return ASAP */
    if (!(flags & AE_TIME_EVENTS) && !(flags & AE_FILE_EVENTS)) {
        return 0;
    }
    if (loop->max_fd != -1 || ((flags & AE_TIME_EVENTS) && !(flags & AE_DONT_WAIT))) {
        TimeEvent* shortest = NULL;
        struct timeval tv, *tvp;
        if ((flags & AE_TIME_EVENTS) && !(flags & AE_DONT_WAIT)) {
            shortest = search_nearest_timer(loop);
        }
        if (shortest) {
            int32_t now_sec, now_ms;
            /* Calculate the time missing for the nearest timer to fire. */
            get_time(&now_sec, &now_ms);
            tvp = &tv;
            tvp->tv_sec = shortest->when_sec - now_sec;
            if (shortest->when_ms < now_ms) {
                tvp->tv_usec = ((shortest->when_ms + 1000) - now_ms) * 1000;
                tvp->tv_sec--;
            } else {
                tvp->tv_usec = (shortest->when_ms - now_ms) * 1000;
            }
            if (tvp->tv_sec < 0) {
                tvp->tv_sec = 0;
            }
            if (tvp->tv_usec < 0) {
                tvp->tv_usec = 0;
            }
        } else {
            /* If we have to check for events but need to return
             * ASAP because of AE_DONT_WAIT we need to set the timeout
             * to zero */
            if (flags & AE_DONT_WAIT) {
                tv.tv_sec = tv.tv_usec = 0;
                tvp = &tv;
            } else {
                /* Otherwise we can block */
                tvp = NULL; /* wait forever */
            }
        }
        
        num_events = api_epoll_poll(loop, tvp);

        for (int j = 0; j < num_events; j++) {
            FiredEvent* fired = &loop->fired[j];
            FileEvent* fe = &loop->events[fired->fd];
            int rfired = 0;
            /* note the fe->mask & mask & ... code: maybe an already processed
             * event removed an element that fired and we still didn't
             * processed, so we check if the event is still valid. */
            if (fe->mask & fired->mask & AE_READABLE) {
                rfired = 1;
                int ret = fe->read(loop, fired->fd, fe->user_data, fired->mask);
                if (ret == AE_NOMORE) {
                    event_loop_del_file_event(loop, fired->fd, AE_READABLE);    
                }
            }
            if (fe->mask & fired->mask & AE_WRITABLE) {
                if (!rfired || fe->write != fe->read) {
                    int ret = fe->write(loop, fired->fd, fe->user_data, fired->mask);
                    if (ret == AE_NOMORE) {
                       event_loop_del_file_event(loop, fired->fd, AE_WRITABLE);    
                    }
                }
            }
            processed++;
        }
    }
    /* Check time events */
    if (flags & AE_TIME_EVENTS) {
        processed += process_time_events(loop);
    }
    return processed;
}

void event_loop_run(EventLoop* loop) {
    loop->stop = false;
    while (!loop->stop) {
        if (loop->before != NULL) {
            loop->before(loop);
        }
        event_loop_proccess_events(loop, AE_ALL_EVENTS);
        if (loop->after != NULL) {
            loop->after(loop);
        }
    }
}

void event_loop_stop(EventLoop* loop) {
    loop->stop = true;    
}
