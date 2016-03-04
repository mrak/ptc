#include <pthread.h>
#include <stdlib.h>
#include <inttypes.h>
#include <stdio.h>
#include <xcb/xcb.h>

#include "deps/chan/chan.h"

enum pt_control_scheme { S7C1T, S8C1T };
enum color_type { XTERM_256, TRUECOLOR };

/* either one of xterm's 256 colors or RGB */
struct color {
    enum color_type type;
    union {
        unsigned char index;
        struct {
            float r, g, b;
        };
    };
};

struct event_listener_arg {
    xcb_connection_t *connection;
    chan_t *command_queue;
};

void *event_listener(void *ptr) {
    struct event_listener_arg *args = (struct event_listener_arg *)ptr;
    xcb_connection_t *connection = args->connection;
    /*chan_t *command_queue = args->command_queue;*/
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
    chan_t *command_queue        = chan_init(20);

    // Window creation
    xcb_connection_t *connection = xcb_connect(NULL, NULL);
    xcb_screen_t *screen         = xcb_setup_roots_iterator (xcb_get_setup (connection)).data;
    xcb_window_t window          = xcb_generate_id(connection);
    uint32_t mask                = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
    uint32_t values[2]           = { screen->white_pixel
                                   , XCB_EVENT_MASK_EXPOSURE
                                   | XCB_EVENT_MASK_BUTTON_PRESS
                                   | XCB_EVENT_MASK_BUTTON_RELEASE
                                   | XCB_EVENT_MASK_POINTER_MOTION
                                   | XCB_EVENT_MASK_ENTER_WINDOW
                                   | XCB_EVENT_MASK_LEAVE_WINDOW
                                   | XCB_EVENT_MASK_KEY_PRESS
                                   | XCB_EVENT_MASK_KEY_RELEASE
                                   };

    xcb_create_window( connection
    /* depth      */ , 0
    /* win        */ , window
    /* parent     */ , screen->root
    /* x, y       */ , 0, 0
    /* w, h       */ , 150, 150
    /* border w   */ , 10
    /* class      */ , XCB_WINDOW_CLASS_INPUT_OUTPUT
    /* visual     */ , screen->root_visual
    /* masks      */ , mask, values
                     );
    xcb_map_window(connection, window);
    xcb_flush(connection);

    // Listen for window events
    struct event_listener_arg event_listener_arg = {connection, command_queue};
    pthread_create(&el, NULL, event_listener, &event_listener_arg);
    pthread_join(el, NULL);

    // Cleanup
    chan_dispose(command_queue);
    return 0;
}
