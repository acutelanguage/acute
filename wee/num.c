/*
 * Acute Programming Language
 * Copyright (c) 2011, Jeremy Tregunna, All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include <stdio.h>
#include <stdlib.h>
#include "num.h"

num_t* num_new_int(long i)
{
    num_t* num = (num_t*)obj_new_with_size(sizeof(*num));
    num->backing = kNumberBackingInteger;
    num->numerals.i = i;
    return num;
}

num_t* num_new_double(double d)
{
    num_t* num = (num_t*)obj_new_with_size(sizeof(*num));
    num->backing = kNumberBackingDouble;
    num->numerals.d = d;
    return num;
}

void num_destroy(num_t* num)
{
    free(num);
    num = NULL;
}

num_t* num_convert_to(num_t* num, num_backing_t backing)
{
    switch(backing)
    {
        case kNumberBackingInteger:
        {
            double d = num->numerals.d;
            num->numerals.i = (long)d;
            break;
        }
        case kNumberBackingDouble:
        {
            long i = num->numerals.i;
            num->numerals.d = (double)i;
            break;
        }
    }

    return num;
}