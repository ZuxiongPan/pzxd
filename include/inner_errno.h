#ifndef __INNER_ERRNO_H__
#define __INNER_ERRNO_H__

#define SUCCESS 0
#define CFUNC_CALLFAIL -1
#define INVALID_ARGUMENT -2
#define OPERATION_FORBIDDEN -3

#define ERRNO_TO_PTR(err) ((void *)(err))
#define PTR_TO_ERRNO(ptr) ((int)(ptr))

#endif