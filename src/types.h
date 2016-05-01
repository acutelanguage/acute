// Refuse
// Copyright © 2016, Jeremy Tregunna, All Rights Reserved.

#ifndef __REFUSE__TYPES_H__
#define __REFUSE__TYPES_H__

typedef struct refhdr_s
{
    unsigned int retainCount:30;
    unsigned int new:1;
    unsigned int dirty:1;
} refhdr_t;

#endif // !__REFUSE__TYPES_H__