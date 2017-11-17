#ifndef _TYPES_H_
#define _TYPES_H_

// Type definitions

typedef unsigned int   uint;
typedef unsigned short ushort;
typedef unsigned char  uchar;
typedef uint pde_t;
#ifndef NULL
#define NULL (0)
#endif

typedef struct {
  uint locked;
} lock_t;

typedef struct {
  uint token;
  uint curr;
  lock_t qlock;
} cond_t;



#endif //_TYPES_H_
