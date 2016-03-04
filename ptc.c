#include <pthread.h>
#include <stdlib.h>
#include <inttypes.h>
#include <stdio.h>
#include <xcb/xcb.h>

#include "chan.h"

/*chan_t* jobs;*/
/*chan_t* done;*/

/*void* worker()*/
/*{*/
    /*// Process jobs until channel is closed.*/
    /*void* job;*/
    /*while (chan_recv(jobs, &job) == 0)*/
    /*{*/
        /*printf("received job %d\n", (int) job);*/
    /*}*/

    /*// Notify that all jobs were received.*/
    /*printf("received all jobs\n");*/
    /*chan_send(done, "1");*/
    /*return NULL;*/
/*}*/

/*int main()*/
/*{*/
/*// Initialize channels.*/
/*jobs = chan_init(5);*/
/*done = chan_init(0);*/

/*pthread_t th;*/
/*pthread_create(&th, NULL, worker, NULL);*/

/*// Send 3 jobs over the jobs channel then close it.*/
/*int i;*/
/*for (i = 1; i <= 3; i++)*/
/*{*/
/*chan_send(jobs, (void*) (uintptr_t) i);*/
/*printf("sent job %d\n", i);*/
/*}*/
/*chan_close(jobs);*/
/*printf("sent all jobs\n");*/

/*// Wait for all jobs to be received.*/
/*chan_recv(done, NULL);*/

/*// Clean up channels.*/
/*chan_dispose(jobs);*/
/*chan_dispose(done);*/
/*}*/

typedef struct {
    xcb_connection_t *connection;
    chan_t *command_queue;
} event_listener_arg_t;

void *event_listener(void *ptr) {
    event_listener_arg_t *args = (event_listener_arg_t *)ptr;
    xcb_connection_t *connection = args->connection;
    chan_t *command_queue = args->command_queue;
    xcb_generic_event_t *event;

    while ((event = xcb_wait_for_event(connection))) {
        switch(event->response_type & ~0x80) {
            case XCB_EXPOSE: {
                xcb_expose_event_t *expose = (xcb_expose_event_t *)event;

                printf ("EXPOSE (x: %"PRIu16", y: %"PRIu16") (w: %"PRIu16", h: %"PRIu16")\n",
                        expose->x, expose->y, expose->width, expose->height);
                break;
            }
            default: {
                break;
            }
        }

        free(event);
    }

    return NULL;
}


int main() {
    pthread_t el;

    chan_t *command_queue = chan_init(20);
    xcb_connection_t *connection = xcb_connect(NULL, NULL);
    xcb_screen_t *screen = xcb_setup_roots_iterator (xcb_get_setup (connection)).data;
    xcb_window_t window = xcb_generate_id(connection);

    uint32_t     mask      = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
    uint32_t     values[2] = { screen->white_pixel
                             , XCB_EVENT_MASK_EXPOSURE
                             | XCB_EVENT_MASK_BUTTON_PRESS
                             | XCB_EVENT_MASK_BUTTON_RELEASE
                             | XCB_EVENT_MASK_POINTER_MOTION
                             | XCB_EVENT_MASK_ENTER_WINDOW
                             | XCB_EVENT_MASK_LEAVE_WINDOW
                             | XCB_EVENT_MASK_KEY_PRESS
                             | XCB_EVENT_MASK_KEY_RELEASE
                             };

    xcb_create_window ( connection
    /* depth       */ , 0
    /* win, parent */ , window, screen->root
    /* x, y        */ , 0, 0
    /* w, h        */ , 150, 150
    /* border w    */ , 10
    /* class       */ , XCB_WINDOW_CLASS_INPUT_OUTPUT
    /* visual      */ , screen->root_visual
    /* masks       */ , mask, values
                      );
    xcb_map_window (connection, window);
    xcb_flush (connection);

    event_listener_arg_t event_listener_arg = {connection, command_queue};
    pthread_create(&el, NULL, event_listener, &event_listener_arg);
    pthread_join(el, NULL);

    return 0;
}
