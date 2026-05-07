#ifndef __COMM_EVENT_H__
#define __COMM_EVENT_H__

#define EVENT_TYPE(event) ((event) & 0xffff0000)
#define EVENT_ID(event) ((event) & 0xffff)

#define INVALID_EVENT_TYPE 0x00000000

#define EV_CORE_TYPE 0x11110000
#define EV_ALIVE_CHECK (EV_CORE_TYPE + 1)
#define EV_ALIVE_ACK (EV_CORE_TYPE + 2)


#define EV_TEST_TYPE 0xffff0000
#define EV_TEST_EVENT1 (EV_TEST_TYPE + 1)

#endif