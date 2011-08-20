/* file QISH/lib/qigc.c */
/* emacs Time-stamp: <2005 May 14 17h53 CEST {qigc.c} Basile STARYNKEVITCH> */
/* prcsid $Id: qigc.c 36 2005-10-12 19:40:42Z basile $ */
/* prcsproj $ProjectHeader: Qish 1.9 Tue, 28 Dec 2004 12:08:57 +0100 basile $ */
//  Copyright © 2001-2005 Basile STARYNKEVITCH

/*
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General
 * Public License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place, Suite 330,
 * Boston, MA 02111-1307, USA.
 */
#define _GNU_SOURCE


#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <unistd.h>
#include <errno.h>
#include <setjmp.h>
#include <limits.h>
#include <sys/mman.h>
#include <string.h>
#include <assert.h>


#include "qish.h"



struct qishgc_birth_st qishgc_birth;
volatile int qish_need_full_gc;
/*****
   we are aiming a precise, copying, generational garbage collector

   at each safe point we need to know:
   a) the number of object parameters
   b) the number of object locals
   c) the first object parameter address
   d) the first object local address
   e) link to previous GC-ed "frame"
   f) other relevant information describing the call point (useful for
      introspection?)

   so each safe point has a constant (read-only) descriptor, 
      the short number NBPARAM of object parameters 
      the short number NBLOCAL of object locals
      an optional unique (nonzero) long number RANK of the safe point
      the C function name

   inside each GC-ed frame we have to maintain a frame descriptor
      the adress of relevant above descriptor
      the first object parameter
      the first object local
      the link to previous frame 

   the first word of an object is not zero, except for *forwarded* pointers 

   This GC is not mostly copying in the same sense as Bartlett's GC
   (1990); It copies every movable object (even if it is on the call
   stack - in that case the variable or argument should be passed to
   BEGIN_SIMPLE_FRAME or equivalent macro), and mark fixed objects
   (which have finalizers). Thanks to Gerd Moellmann for his comments.
****/

const int qishgc_nbroots = QISH_NB_ROOTS;


// minimal birth size is 8 MBytes - should be in a 1-100Mb range and
// no more than about a tenth of your available RAM
#ifndef QISH_MIN_MEGA
// 16 Mb on 64 bits machines, 8 Mb on 32 bits machines
#define QISH_MIN_MEGA  (QISH_POINTER_SIZE*2)
#endif

#if QISH_MIN_MEGA < 1 || QISH_MIN_MEGA > 256
#error QISH_MIN_MEGA should be > 1 and < 256
#endif

#ifndef MIN_BIRTH_SIZE
#define MIN_BIRTH_SIZE  ((QISH_MIN_MEGA)<<20)
#endif

// maximum birth size is 128 Mbytes - should be bigger than min size
#ifndef MAX_BIRTH_SIZE
#define MAX_BIRTH_SIZE (16*MIN_BIRTH_SIZE)
#endif

#if MAX_BIRTH_SIZE < 4 * MIN_BIRTH_SIZE
#error invalid min and max birth sizes
#endif

// threshold for making full gc - should be bigger than min size
#define FULL_GC_THRESHOLD (3*MIN_BIRTH_SIZE)

// period of forced full GC (0 to disable periodic full GC)
#define FULL_GC_PERIOD 256

/// counters for minor and full gc
int qish_nb_minor_collections;
int qish_nb_full_collections;

volatile void *qish_globconstab[QISH_MAXNBCONST + 1];
volatile char qish_globconstabwrbar[(QISH_MAXNBCONST >> 8) + 1];

const int qish_min_birth_size = MIN_BIRTH_SIZE;
const int qish_max_birth_size = MAX_BIRTH_SIZE;
const int qish_full_gc_threshold = FULL_GC_THRESHOLD;
const int qish_full_gc_period = FULL_GC_PERIOD;

#ifndef QISH_ROUTINE
/* copy source to destination and return the end of copy */
// src is the original object to be copied
// dst is the adress after which should go the copy
// *padr should be set to the new adress of the copy 
/// usually *padr=dst or the word after, if needed alignement 
// the return adress is the first word after the copied object
void *(*qish_gc_copy_p) (void **padr, void *dst, const void *src) = 0;

/* scan a movable object for minor GC, updating its pointers with
   QISHGC_MINOR_UPDATE, returning the next word after the scanned
   object */
void *(*qish_minor_scan_p) (void *ptr) = 0;

/* scan a movable object for full GC, updating its pointers with
   QISHGC_FULL_UPDATE, returning the next word after the scanned
   object */
void *(*qish_full_scan_p) (void *ptr) = 0;

/* minor scan a fixed object for full GC, updating its pointers */
void (*qish_fixedminor_scan_p) (void *ptr, int size) = 0;
/* full scan a fixed object for full GC, updating its pointers */
void (*qish_fixedfull_scan_p) (void *ptr, int size) = 0;

/* update other roots on minor GC, so call QISH_MINOR_UPDATE on extra
   GC roots, such as pointers inside the stack of an interpreter or
   virtual machine */
void (*qish_extra_minor_p) (void) = 0;

/* update other roots on full GC, so call QISH_FULL_UPDATE on extra
   GC roots, such as pointers inside the stack of an interpreter or
   virtual machine */
void (*qish_extra_full_p) (void) = 0;

/* prologue and epilogue hooks */
void (*qish_full_prehook_p) (void) = 0;
void (*qish_full_posthook_p) (void) = 0;
#endif

#ifndef NDEBUG
#define dbgmemap(Msg) {if (qish_debug) qishgc_dbgmemap_at((Msg),__LINE__);}
#else
#define dbgmemap(Msg) {}
#endif

int qish_debug;

// C++ hooks
void (*qishcpp_minor_p)(void);
void (*qishcpp_full_p)(void);


// actually we should protect this by a mutex, but we believe mutexes
// are too expensive to be checked at each allocation, and this flag
// is an optimization (because eventually every thread calls a GC) so
// we leave this as a volatile
volatile int qish_need_gc;

/* the old moving region has a low and high pointer */
void *qishgc_old_lo;
void *qishgc_old_hi;
void *qishgc_old_cur;

/* the fixed object region has a low and high pointer */
void *qishgc_fixed_lo;
void *qishgc_fixed_hi;

volatile void *qish_roots[QISH_NB_ROOTS];

struct qishmodule_st qish_moduletab[QISH_MAX_MODULE];


/* the rank up to which constants have to be updated on minor GC */
int qishgc_changedconstrank;

#ifdef TRACEMEM
FILE *qish_tracememfile;
#endif //TRACEMEM

static void
qishgc_dbgmemap_at (const char *msg, int lineno)
{
  FILE *f = fopen ("/proc/self/maps", "r");
  char linbuf[250];
  if (!f)
    qish_panic_at (errno, __FILE__, lineno, 0, "Cannot open mem.maps [%s]",
		   msg);
  fprintf (stderr,"\n%s:%d. *** memory map : %s ***\n", __FILE__, lineno, msg);
  while (!feof (f)) {
    memset (linbuf, 0, sizeof (linbuf));
    fgets (linbuf, sizeof (linbuf) - 1, f);
    if (!strchr (linbuf, '/'))
      fputs (linbuf, stderr);
  }
  fclose (f);
  putc ('\n', stderr);
}



// experimentally, this GC use a lot of system calls (mostly mmap &
// munmap) but it is not worth to optimze this by caching the latest
// munmap to reuse it instead of mmap-ing, because memory should be
// explicitly zeroed anyway!

#define qishgc_getmem(Msg,Sz) qishgc_getmem_at((Msg),(Sz),__LINE__)
static void *
qishgc_getmem_at (const char *msg, int sz, int lineno)
{
  char *mem;
  /* round up the size */
  if (sz & (QISH_PAGESIZE - 1)) {
    sz = sz | (QISH_PAGESIZE - 1);
    sz++;
  };
#ifdef TRACEMEM
  if (qish_tracememfile)
    fprintf (qish_tracememfile, "GETMEM sz=%06dk; lin=%03d; msg=%s\n",
	     (sz) >> 10, lineno, msg);
#endif /*TRACEMEM*/
    /* allocate with extra page before and after */
    mem = mmap ((void *) 0, sz + 2 * QISH_PAGESIZE,
		PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);
  if (mem == MAP_FAILED)
    qish_panic_at (errno, __FILE__, lineno, 0,
		   "Cannot get memory [%s] of %dKBytes", msg, sz >> 10);
  if (munmap (mem, QISH_PAGESIZE))
    qish_panic_at (errno, __FILE__, lineno, 0,
		   "Cannot release previous page at %p [%s] while  getting %dKbytes",
		   mem, msg, sz >> 10);
  mem += QISH_PAGESIZE;
  if (munmap (mem + sz, QISH_PAGESIZE))
    qish_panic_at (errno, __FILE__, lineno, 0,
		   "Cannot release next page at %p [%s] while  getting %dKbytes",
		   mem + sz, msg, sz >> 10);
#ifndef NDEBUG
  if (qish_debug)
    fprintf (stderr,"%s:%d:: getmem [%s] %dKbytes => %p-%p\n", __FILE__, lineno, msg,
	    sz >> 10, mem, mem + sz);
#endif
  return mem;
}				/* end of qishgc_getmem_at */


// we need to allocate executable memory

#define qishgc_getexecmem(Msg,Sz) qishgc_getexecmem_at((Msg),(Sz),__LINE__)
static void *
qishgc_getexecmem_at (const char *msg, int sz, int lineno)
{
  char *mem;
  /* round up the size */
  if (sz & (QISH_PAGESIZE - 1)) {
    sz = sz | (QISH_PAGESIZE - 1);
    sz++;
  };
#ifdef TRACEMEM
  if (qish_tracememfile)
    fprintf (qish_tracememfile, "GETMEM sz=%06dk; lin=%03d; msg=%s\n",
	     (sz) >> 10, lineno, msg);
#endif /*TRACEMEM*/
    /* allocate with extra page before and after */
    mem = mmap ((void *) 0, sz + 2 * QISH_PAGESIZE,
		PROT_READ | PROT_WRITE | PROT_EXEC,
		MAP_ANON | MAP_PRIVATE, -1, 0);
  if (mem == MAP_FAILED)
    qish_panic_at (errno, __FILE__, lineno, 0,
		   "Cannot get memory [%s] of %dKBytes", msg, sz >> 10);
  if (munmap (mem, QISH_PAGESIZE))
    qish_panic_at (errno, __FILE__, lineno, 0,
		   "Cannot release previous page at %p [%s] while  getting %dKbytes",
		   mem, msg, sz >> 10);
  mem += QISH_PAGESIZE;
  if (munmap (mem + sz, QISH_PAGESIZE))
    qish_panic_at (errno, __FILE__, lineno, 0,
		   "Cannot release next page at %p [%s] while  getting %dKbytes",
		   mem + sz, msg, sz >> 10);
#ifndef NDEBUG
  if (qish_debug)
    fprintf (stderr,"%s:%d:: getexecmem [%s] %dKbytes => %p-%p\n", __FILE__, lineno,
	    msg, sz >> 10, mem, mem + sz);
#endif
  return mem;
}				/* end of qishgc_getexecmem_at */



#define qishgc_releasemem(Msg,Mem,Siz) qishgc_releasemem_at((Msg),(Mem),(Siz),__LINE__)
static void
qishgc_releasemem_at (const char *msg, void *mem, int sz, int lineno)
{
  /* round up the size */
  if (sz & (QISH_PAGESIZE - 1)) {
    sz = sz | (QISH_PAGESIZE - 1);
    sz++;
  };
#ifdef TRACEMEM
  if (qish_tracememfile)
    fprintf (stderr,qish_tracememfile, "RELEASEMEM sz=%06dk; lin=%03d; msg=%s\n",
	     (sz) >> 10, lineno, msg);
#endif /*TRACEMEM*/
    if (munmap (mem, sz))
    qish_panic_at (errno, __FILE__, lineno, 0,
		   "Cannot unmap released memory [%s] at %p of %dKbytes", msg,
		   mem, sz >> 10);
#ifndef NDEBUG
  if (qish_debug)
    fprintf (stderr,"%s:%d:: releasemem [%s] %dKbytes %p-%p\n", __FILE__, lineno, msg,
	    sz >> 10, mem, (char *) mem + sz);
#endif
}				/* end of qishgc_releasemem_at */


static void
qishgc_minor (int siz)
{
  struct qishgc_framedescr_st *fram = 0;
  int n = 0;
  void *birthlo = (void *) qishgc_birth.bt_lo;
  void *birthhi = (void *) qishgc_birth.bt_hi;
  void *scanptr = (void *) qishgc_old_cur;
  void *newad = 0;
  void **storead = 0;
#ifdef __sparc__
  asm ("ta 3");			// flush register on sparc
#endif //__sparc__
  if (siz < 0)
    siz = 0;
  else if (siz >= MAX_BIRTH_SIZE)
    qish_panic
      ("QISH: huge birth memory size requested (%dKbytes, MAX_BIRTH_SIZE is %dKbytes)\n",
       siz >> 10, MAX_BIRTH_SIZE >> 10);
  qish_dbgprintf ("qishgc_minor siz=%dK birth=%p-%p", siz >> 10, birthlo, birthhi);
  dbgmemap ("before minor gc");
  ///
#define MINOR_UPDATE(Ptr) {				\
  if ((qish_uaddr_t)(Ptr)>=(qish_uaddr_t)birthlo	\
      && ((qish_uaddr_t)(Ptr) & 3) == 0			\
      && (qish_uaddr_t)(Ptr)<=(qish_uaddr_t)birthhi)	\
    QISHGC_FORWARD(&(Ptr)); }
  // we might use QISHGC_MINOR_UPDATE but we can use MINOR_UPDATE
  // instead here because it should be faster
  for (n = 0; n < QISH_NB_ROOTS; n++)
    MINOR_UPDATE (((void **) qish_roots)[n]);
  if (qishgc_changedconstrank > 0) {
    assert (qishgc_changedconstrank <= QISH_MAX_MODULE);
    for (n = 0; n < qishgc_changedconstrank; n++)
      MINOR_UPDATE (qish_moduletab[n].km_constant);
    qishgc_changedconstrank = 0;
  };
  for (n = 0; n < QISH_MAXNBCONST >> 8; n++)
    if (qish_globconstabwrbar[n]) {
      int i = 0, j = 0;
      j = n >> 8;
      for (i = j; i < j + 256; i++)
	MINOR_UPDATE (qish_globconstab[i]);
      qish_globconstabwrbar[n] = 0;
    };
  //.qish_dbgprintf("minor nbroots=%d qishgcf=%p", n, qishgcf);
  // constants are only scanned with full GC, so changing them always
  // require adding to the store vector; we do not scan constants on
  // minor GC!
  for (fram = (struct qishgc_framedescr_st *) qishgc_birth.bt_qishgcf; fram;
       fram = (struct qishgc_framedescr_st *) (fram->gcf_prev)) {
    int i;
    int nbparams = fram->gcf_point->gcd_nbparam;
    int nblocals = fram->gcf_point->gcd_nblocal;
    void **args = fram->gcf_args;
    void **locals = fram->gcf_locals;
    //.qish_dbgprintf("minorgc fram=%p args=%p nbparams=%d locals=%p nblocals=%d",
    //.       fram, args, nbparams, locals, nblocals);
    assert (args || locals);
    assert (nbparams >= 0 && nblocals >= 0);
#if QISH_ARGS_UP
    /* actual arguments have increasing adresses from args; optimize
       for few arguments */
    switch (nbparams) {
    default:
      for (i = nbparams - 1; i >= 7; i--)
	MINOR_UPDATE (args[i]);
    case 7:
      MINOR_UPDATE (args[6]);	/*fallthru */
    case 6:
      MINOR_UPDATE (args[5]);	/*fallthru */
    case 5:
      MINOR_UPDATE (args[4]);	/*fallthru */
    case 4:
      MINOR_UPDATE (args[3]);	/*fallthru */
    case 3:
      MINOR_UPDATE (args[2]);	/*fallthru */
    case 2:
      MINOR_UPDATE (args[1]);	/*fallthru */
    case 1:
      MINOR_UPDATE (args[0]);	/*fallthru */
    case 0:
      break;
    };
#endif //QISH_ARGS_UP
    ///////
#if QISH_ARGS_DOWN
    /* actual arguments have decreasing adresses from args; optimize
       for few arguments */
    switch (nbparams) {
    default:
      for (i = nbparams - 1; i >= 7; i--)
	MINOR_UPDATE (args[-i]);
    case 7:
      MINOR_UPDATE (args[-6]);	/*fallthru */
    case 6:
      MINOR_UPDATE (args[-5]);	/*fallthru */
    case 5:
      MINOR_UPDATE (args[-4]);	/*fallthru */
    case 4:
      MINOR_UPDATE (args[-3]);	/*fallthru */
    case 3:
      MINOR_UPDATE (args[-2]);	/*fallthru */
    case 2:
      MINOR_UPDATE (args[-1]);	/*fallthru */
    case 1:
      MINOR_UPDATE (args[0]);	/*fallthru */
    case 0:;
    };
#endif //QISH_ARGS_DOWN
    /* optimize for few locals */
    switch (nblocals) {
    default:
      for (i = nblocals - 1; i >= 16; i--)
	MINOR_UPDATE (locals[i]);
    case 16:
      MINOR_UPDATE (locals[15]);	/*fallthru */
    case 15:
      MINOR_UPDATE (locals[14]);	/*fallthru */
    case 14:
      MINOR_UPDATE (locals[13]);	/*fallthru */
    case 13:
      MINOR_UPDATE (locals[12]);	/*fallthru */
    case 12:
      MINOR_UPDATE (locals[11]);	/*fallthru */
    case 11:
      MINOR_UPDATE (locals[10]);	/*fallthru */
    case 10:
      MINOR_UPDATE (locals[9]);	/*fallthru */
    case 9:
      MINOR_UPDATE (locals[8]);	/*fallthru */
    case 8:
      MINOR_UPDATE (locals[7]);	/*fallthru */
    case 7:
      MINOR_UPDATE (locals[6]);	/*fallthru */
    case 6:
      MINOR_UPDATE (locals[5]);	/*fallthru */
    case 5:
      MINOR_UPDATE (locals[4]);	/*fallthru */
    case 4:
      MINOR_UPDATE (locals[3]);	/*fallthru */
    case 3:
      MINOR_UPDATE (locals[2]);	/*fallthru */
    case 2:
      MINOR_UPDATE (locals[1]);	/*fallthru */
    case 1:
      MINOR_UPDATE (locals[0]);	/*fallthru */
    case 0:;
    };
  };
  /// update extra minor  roots if applicable

  qish_dbgprintf ("before calling qish_extra_minor");
  qish_extra_minor ();
  qish_dbgprintf ("after calling qish_extra_minor");

  /// C++ hook
  qishcpp_minor();

#undef MINOR_UPDATE
  /* scan the stored object vector (write barrier) */
  for (storead = ((void **) qishgc_birth.bt_storeptr) - 1;
       storead < (void **) qishgc_birth.bt_hi; storead++) {
    void *written = *storead;
    qish_dbgprintf("minor scan stored storead=%p written=%p", storead, written);
    if (QISH_IS_MOVING_PTR(written)) {
      qish_dbgprintf ("old stored object %p in storeptr%p", written,
		 (void *) storead);
      qish_minor_scan (written);
      qish_dbgprintf ("updated stored object %p in storeptr%p",
		 (void *) (*storead), (void *) storead);
    }
    else if (QISH_IS_FIXED_PTR(written)) {
      qish_dbgprintf("written fixed object %p", written);
      qishgc_minormarkscan(written);
    }
  }
  /* Chesney loop */
  qish_dbgprintf("minor chesney scanptr=%p", scanptr);
  while ((qish_uaddr_t) scanptr < (qish_uaddr_t) qishgc_old_cur) {
    qish_dbgprintf("minor chesney loop scanptr=%p oldcur=%p", scanptr, qishgc_old_cur);
    scanptr = qish_minor_scan (scanptr);
    // skip to next non zero word
    while ((qish_uaddr_t) scanptr < (qish_uaddr_t) qishgc_old_cur
	   && (*(void **) scanptr) == 0)
      scanptr = ((void **) scanptr) + 1;
  };
  // allocate the new birth region
  siz += 2 * QISH_PAGESIZE;
  if (siz < MIN_BIRTH_SIZE)
    siz = MIN_BIRTH_SIZE;
  // round up size to page
  siz |= (QISH_PAGESIZE - 1);
  siz++;
  newad = qishgc_getmem ("new birth region -minor GC-", siz);
  qishgc_releasemem ("previous birth region -minor GC-",
		     (void *) qishgc_birth.bt_lo,
		     (char *) qishgc_birth.bt_hi -
		     (char *) qishgc_birth.bt_lo);
  qishgc_birth.bt_lo = newad;
  qishgc_birth.bt_cur = (void *) ((char *) newad + 2 * sizeof (void *));
  qishgc_birth.bt_hi = (void *) ((char *) newad + siz);
  // the offset of qishgc_birth.bt_storeptr is related to offsets in
  // qish_write_notify
  qishgc_birth.bt_storeptr = (void *) (((void **) qishgc_birth.bt_hi) - 5);
  qish_nb_minor_collections++;
  dbgmemap ("after minor gc");
}				/* end of qishgc_minor */



/* at most 128Megabytes of fixed address objects */
#define FIXEDHEAP_MAXLEN   (128*1024*1024)

/* initial fixed space is 8Mb */
#define FIXEDHEAP_INITLEN (8*1024*1024)

struct qishfixedheader_st *qishgc_fixedhigh_head;

#define QISHNBFIXEDSIZE 52
const size_t qishgc_fixed_sizetab[QISHNBFIXEDSIZE] = {
  /* array of total size, header included */
  0,
  /* we should have power of 2 or 3*power of 2 here, all multiple of 32 */
  2 << 4, 2 << 5, 3 << 5, 2 << 6, 3 << 6, 2 << 7, 3 << 7, 2 << 8,
  3 << 8, 2 << 9, 3 << 9, 2 << 10, 3 << 10,
  2 << 11, 3 << 11, 2 << 12, 3 << 12, 2 << 13, 3 << 13, 2 << 14, 3 << 14,
  2 << 15, 3 << 15, 2 << 16, 3 << 16,
  2 << 17, 3 << 17, 2 << 18, 3 << 18, 2 << 19, 3 << 19, 2 << 20, 3 << 20,
  2 << 21, 3 << 21, 2 << 22, 3 << 22,
  2 << 23, 3 << 23, 2 << 24, 3 << 24, 2 << 25, 3 << 25, 2 << 26, 3 << 26,
  2 << 27, 3 << 27, 2 << 28, 3 << 28,
  0
};

// free and used fixed headers 
struct qishfixedheader_st *qishgc_free_fixed[QISHNBFIXEDSIZE];
struct qishfixedheader_st *qishgc_used_fixed[QISHNBFIXEDSIZE];

// free and used executable headers
struct qishfixedheader_st *qishgc_free_exfix[QISHNBFIXEDSIZE];
struct qishfixedheader_st *qishgc_used_exfix[QISHNBFIXEDSIZE];

/* values of kfx_magic */
#define KFX_MAGIC_FREE 0x7ae	/* free chunk */
#define KFX_MAGIC_USED 0x2e8f	/* used chunk */
#define KFX_MAGIC_EXEC 0x2e9d	/* used-executable chunk */

static void
free_fixed_chunk (struct qishfixedheader_st *phyprev, void *start, int sz)
{
  int szix = 0;
  int i = 0;
  size_t freesz = 0;
  struct qishfixedheader_st *r = 0;
  do {
#if 0
    qish_dbgprintf ("free fixed loop phyprev=%p start=%p->%p sz=%dK", phyprev,
	       start, start + sz, sz >> 10);
    qish_dbgprintf ("*start=%x", *(char *) start);
    qish_dbgprintf ("*end-1=%x", *((((char *) start)) + sz - 1));
#endif
    assert ((char *) phyprev < (char *) start);
    szix = 0;
    for (i = 1; i < QISHNBFIXEDSIZE; i++)
      if ((int) qishgc_fixed_sizetab[i] <= sz) {
	szix = i;
	freesz = qishgc_fixed_sizetab[i];
      } else
	break;
    //.qish_dbgprintf("szix=%d freesz=%d", szix, freesz);
    if (sz > (int) sizeof (struct qishfixedheader_st) + 128)
      memset (start, 0, sizeof (struct qishfixedheader_st) + 128);
    else
      memset (start, 0, sz);
    if (szix <= 0)
      return;			// no index found - space is lost...
    r = start;
    r->kfx_magic = KFX_MAGIC_FREE;
    r->kfx_sizix = szix;
    r->kfx_mark = 0;
    r->kfx_destr = 0;
    r->kfx_phyprev = phyprev;
    r->kfx_next = qishgc_free_fixed[szix];
    qishgc_free_fixed[szix] = r;
    phyprev = r;
    start = (char *) r + qishgc_fixed_sizetab[szix];
    sz -= qishgc_fixed_sizetab[szix];
  } while (sz > 0);
}				/* end of free_fixed_chunk */

static void
free_exec_chunk (struct qishfixedheader_st *phyprev, void *start, int sz)
{
  int szix = 0;
  int i = 0;
  size_t freesz = 0;
  struct qishfixedheader_st *r = 0;
  do {
#if 0
    qish_dbgprintf ("free fixed loop phyprev=%p start=%p->%p sz=%dK", phyprev,
	       start, start + sz, sz >> 10);
    qish_dbgprintf ("*start=%x", *(char *) start);
    qish_dbgprintf ("*end-1=%x", *((((char *) start)) + sz - 1));
#endif
    assert ((char *) phyprev < (char *) start);
    szix = 0;
    for (i = 1; i < QISHNBFIXEDSIZE; i++)
      if ((int) qishgc_fixed_sizetab[i] <= sz) {
	szix = i;
	freesz = qishgc_fixed_sizetab[i];
      } else
	break;
    //.qish_dbgprintf("szix=%d freesz=%d", szix, freesz);
    if (sz > (int) sizeof (struct qishfixedheader_st) + 128)
      memset (start, 0, sizeof (struct qishfixedheader_st) + 128);
    else
      memset (start, 0, sz);
    if (szix <= 0)
      return;			// no index found - space is lost...
    r = start;
    r->kfx_magic = KFX_MAGIC_FREE;
    r->kfx_sizix = szix;
    r->kfx_mark = 0;
    r->kfx_destr = 0;
    r->kfx_phyprev = phyprev;
    r->kfx_next = qishgc_free_exfix[szix];
    qishgc_free_fixed[szix] = r;
    phyprev = r;
    start = (char *) r + qishgc_fixed_sizetab[szix];
    sz -= qishgc_fixed_sizetab[szix];
  } while (sz > 0);
}				/* end of free_exec_chunk */


/* called when no fixed chunk of required is found - either allocate
   memory or break a bigger free chunk into pieces */

static struct qishfixedheader_st *
get_fixed_chunk (int sizix)
{
  int ix = 0;
  struct qishfixedheader_st *r = 0;
  size_t allocsz = 0;
  void *ad = 0;
  assert (sizix > 0 && sizix < QISHNBFIXEDSIZE);
  for (ix = sizix + 1; ix < QISHNBFIXEDSIZE; ix++)
    if ((r = qishgc_free_fixed[ix]) != 0) {
      assert (r->kfx_magic == KFX_MAGIC_FREE);
      qishgc_free_fixed[ix] = r->kfx_next;
      r->kfx_next = qishgc_free_fixed[sizix];
      r->kfx_sizix = sizix;
      r->kfx_magic = KFX_MAGIC_USED;
      free_fixed_chunk (r, (char *) r + qishgc_fixed_sizetab[sizix],
			qishgc_fixed_sizetab[ix] -
			qishgc_fixed_sizetab[sizix]);
      return r;
    };
  /* no bigger chunk found, allocate one... */
  allocsz = 15 * QISH_PAGESIZE + qishgc_fixed_sizetab[sizix];
  allocsz |= (QISH_PAGESIZE - 1);
  allocsz++;
  //.qish_dbgprintf("allocsz=%d", allocsz);
  if ((char *) qishgc_fixed_hi + allocsz >
      (char *) qishgc_fixed_lo + FIXEDHEAP_MAXLEN)
    qish_panic ("overflowing memory for fixed objects (%dKb requested)\n",
		(int)(allocsz >> 10));
  ad =
    mmap (qishgc_fixed_hi, (size_t) allocsz,
	  PROT_READ | PROT_WRITE | PROT_EXEC,
	  MAP_ANON | MAP_FIXED | MAP_PRIVATE, -1, 0);
  if (ad == MAP_FAILED)
    qish_epanic ("cannot allocate memory for fixed objects (%dKb requested)",
		 (int)(allocsz >> 10));
  qish_dbgprintf ("mmap fixed %p-%p %dK", qishgc_fixed_hi,
	     (char *) qishgc_fixed_hi + allocsz, (int)(allocsz >> 10));
  //.qish_dbgprintf("mmaped ad=%p", ad);
  r = ad;
  r->kfx_magic = KFX_MAGIC_USED;
  r->kfx_sizix = sizix;
  r->kfx_mark = 0;
  r->kfx_destr = 0;
  r->kfx_phyprev = qishgc_fixedhigh_head;
  r->kfx_next = 0;
  qishgc_fixedhigh_head = r;
  free_fixed_chunk (r, r + qishgc_fixed_sizetab[sizix],
		    allocsz - qishgc_fixed_sizetab[sizix]);
  qishgc_fixed_hi = (char *) ad + allocsz;
  return r;
}				/* end of get_fixed_chunk */


/* called when no fixed-exec chunk of required is found - either allocate
   memory or break a bigger free chunk into pieces */

static struct qishfixedheader_st *
get_exec_chunk (int sizix)
{
  int ix = 0;
  struct qishfixedheader_st *r = 0;
  size_t allocsz = 0;
  void *ad = 0;
  assert (sizix > 0 && sizix < QISHNBFIXEDSIZE);
  for (ix = sizix + 1; ix < QISHNBFIXEDSIZE; ix++)
    if ((r = qishgc_free_exfix[ix]) != 0) {
      assert (r->kfx_magic == KFX_MAGIC_FREE);
      qishgc_free_fixed[ix] = r->kfx_next;
      r->kfx_next = qishgc_free_exfix[sizix];
      r->kfx_sizix = sizix;
      r->kfx_magic = KFX_MAGIC_EXEC;
      free_exec_chunk (r, (char *) r + qishgc_fixed_sizetab[sizix],
		       qishgc_fixed_sizetab[ix] -
		       qishgc_fixed_sizetab[sizix]);
      return r;
    };
  /* no bigger chunk found, allocate one... */
  allocsz = 15 * QISH_PAGESIZE + qishgc_fixed_sizetab[sizix];
  allocsz |= (QISH_PAGESIZE - 1);
  allocsz++;
  //.qish_dbgprintf("allocsz=%d", allocsz);
  if ((char *) qishgc_fixed_hi + allocsz >
      (char *) qishgc_fixed_lo + FIXEDHEAP_MAXLEN)
    qish_panic ("overflowing memory for execfixed objects (%dKb requested)\n",
		(int)(allocsz >> 10));
  ad = qishgc_getexecmem ("exec fixed alloc", allocsz);
  qish_dbgprintf ("mmap fixed %p-%p %dK", qishgc_fixed_hi,
	     (char *) qishgc_fixed_hi + allocsz, (int)(allocsz >> 10));
  //.qish_dbgprintf("mmaped ad=%p", ad);
  r = ad;
  r->kfx_magic = KFX_MAGIC_EXEC;
  r->kfx_sizix = sizix;
  r->kfx_mark = 0;
  r->kfx_destr = 0;
  r->kfx_phyprev = qishgc_fixedhigh_head;
  r->kfx_next = 0;
  qishgc_fixedhigh_head = r;
  free_fixed_chunk (r, r + qishgc_fixed_sizetab[sizix],
		    allocsz - qishgc_fixed_sizetab[sizix]);
  qishgc_fixed_hi = (char *) ad + allocsz;
  return r;
}				/* end of get_exec_chunk */


void *
qish_fixed_alloc (size_t sz, void (*destr) (void *))
{
  size_t rawsz = sz + sizeof (struct qishfixedheader_st);
  int szix = 0;
  int i = 0;
  struct qishfixedheader_st *r = 0;
  if (rawsz <= 2 << 7) {
    if (rawsz <= 2 << 4)
      szix = 1;
    else if (rawsz <= 2 << 5)
      szix = 2;
    else if (rawsz <= 3 << 5)
      szix = 3;
    else if (rawsz <= 2 << 6)
      szix = 4;
    else if (rawsz <= 3 << 6)
      szix = 5;
    else if (rawsz <= 2 << 7)
      szix = 6;
  } else
    for (i = 5; i < QISHNBFIXEDSIZE; i++)
      if (rawsz <= qishgc_fixed_sizetab[i]) {
	szix = i;
	break;
      };
  if ((r = qishgc_free_fixed[szix]) != 0) {
    /* reserve the r and unlink it from the free list */
    assert (r->kfx_magic == KFX_MAGIC_FREE);
    assert (r->kfx_sizix == szix);
    qishgc_free_fixed[szix] = r->kfx_next;
    r->kfx_mark = 1;
    r->kfx_magic = KFX_MAGIC_USED;
    r->kfx_destr = destr;
    r->kfx_next = qishgc_used_fixed[szix];
    qishgc_used_fixed[szix] = r;
    memset (r->kfx_data, 0, rawsz - sizeof (struct qishfixedheader_st));
    return r->kfx_data;
  } else {
    /* should make a free chunk of size index szix */
    r = get_fixed_chunk (szix);
    r->kfx_mark = 1;
    r->kfx_magic = KFX_MAGIC_USED;
    r->kfx_destr = destr;
    qishgc_used_fixed[szix] = r;
    return r->kfx_data;
  };
}				/* end of qish_fixed_alloc */



void *
qish_fixed_exec_alloc (size_t sz, void (*destr) (void *))
{
  size_t rawsz = sz + sizeof (struct qishfixedheader_st);
  int szix = 0;
  int i = 0;
  struct qishfixedheader_st *r = 0;
  if (rawsz <= 2 << 7) {
    if (rawsz <= 2 << 4)
      szix = 1;
    else if (rawsz <= 2 << 5)
      szix = 2;
    else if (rawsz <= 3 << 5)
      szix = 3;
    else if (rawsz <= 2 << 6)
      szix = 4;
    else if (rawsz <= 3 << 6)
      szix = 5;
    else if (rawsz <= 2 << 7)
      szix = 6;
  } else
    for (i = 5; i < QISHNBFIXEDSIZE; i++)
      if (rawsz <= qishgc_fixed_sizetab[i]) {
	szix = i;
	break;
      };
  if ((r = qishgc_free_exfix[szix]) != 0) {
    /* reserve the r and unlink it from the free list */
    assert (r->kfx_magic == KFX_MAGIC_FREE);
    assert (r->kfx_sizix == szix);
    qishgc_free_fixed[szix] = r->kfx_next;
    r->kfx_mark = 1;
    r->kfx_magic = KFX_MAGIC_EXEC;
    r->kfx_destr = destr;
    r->kfx_next = qishgc_used_exfix[szix];
    qishgc_used_exfix[szix] = r;
    memset (r->kfx_data, 0, rawsz - sizeof (struct qishfixedheader_st));
    return r->kfx_data;
  } else {
    /* should make a free chunk of size index szix */
    r = get_exec_chunk (szix);
    r->kfx_mark = 1;
    r->kfx_magic = KFX_MAGIC_EXEC;
    r->kfx_destr = destr;
    qishgc_used_exfix[szix] = r;
    return r->kfx_data;
  };
}				/* end of qish_fixed_exec_alloc */

static void
clear_all_fixed_marks (void)
{
  struct qishfixedheader_st *hd = 0;
  int szix = 0;
  for (szix = 1; szix < QISHNBFIXEDSIZE; szix++) {
    for (hd = qishgc_used_fixed[szix]; hd; hd = hd->kfx_next) {
      assert (hd->kfx_magic == KFX_MAGIC_USED
	      || hd->kfx_magic == KFX_MAGIC_EXEC);
      assert (hd->kfx_sizix == szix);
      //.qish_dbgprintf("clear mark hd=%p data %p szix=%d", hd, hd->kfx_data, szix);
      hd->kfx_mark = 0;
    };
    for (hd = qishgc_used_exfix[szix]; hd; hd = hd->kfx_next) {
      assert (hd->kfx_magic == KFX_MAGIC_USED
	      || hd->kfx_magic == KFX_MAGIC_EXEC);
      assert (hd->kfx_sizix == szix);
      //.qish_dbgprintf("clear mark hd=%p data %p szix=%d", hd, hd->kfx_data, szix);
      hd->kfx_mark = 0;
    };
  };
}				// end of clear_all_fixed_marks

static void
destroy_unmarked_fixed (void)
{
  struct qishfixedheader_st *hd = 0;
  struct qishfixedheader_st *nexthd = 0;
  struct qishfixedheader_st **prevptr = 0;
  int szix = 0;
  void (*destr) (void *);
  for (szix = 1; szix < QISHNBFIXEDSIZE; szix++) {
    prevptr = qishgc_used_fixed + szix;
    for (hd = qishgc_used_fixed[szix]; hd; hd = nexthd) {
      assert (hd->kfx_magic == KFX_MAGIC_USED);
      nexthd = hd->kfx_next;
      if (!hd->kfx_mark) {
	//.qish_dbgprintf("destroy unmarked hd=%p data=%p", hd, hd->kfx_data);
	if ((destr = hd->kfx_destr) != 0) {
	  hd->kfx_destr = 0;
	  (*destr) ((void *) (hd->kfx_data));
	  memset (hd + 1, 0, qishgc_fixed_sizetab[szix] - sizeof (*hd));
	};
	free_fixed_chunk (hd->kfx_phyprev, hd, qishgc_fixed_sizetab[szix]);
      };
      *prevptr = nexthd;
      prevptr = &hd->kfx_next;
    };
  }
}				// end of destroy_unmarked_fixed

static void
destroy_unmarked_execfix (void)
{
  struct qishfixedheader_st *hd = 0;
  struct qishfixedheader_st *nexthd = 0;
  struct qishfixedheader_st **prevptr = 0;
  int szix = 0;
  void (*destr) (void *);
  for (szix = 1; szix < QISHNBFIXEDSIZE; szix++) {
    prevptr = qishgc_used_exfix + szix;
    for (hd = qishgc_used_exfix[szix]; hd; hd = nexthd) {
      assert (hd->kfx_magic == KFX_MAGIC_EXEC);
      nexthd = hd->kfx_next;
      if (!hd->kfx_mark) {
	//.qish_dbgprintf("destroy unmarked hd=%p data=%p", hd, hd->kfx_data);
	if ((destr = hd->kfx_destr) != 0) {
	  hd->kfx_destr = 0;
	  (*destr) ((void *) (hd->kfx_data));
	  memset (hd + 1, 0, qishgc_fixed_sizetab[szix] - sizeof (*hd));
	};
	free_exec_chunk (hd->kfx_phyprev, hd, qishgc_fixed_sizetab[szix]);
      };
      *prevptr = nexthd;
      prevptr = &hd->kfx_next;
    };
  }
}				// end of destroy_unmarked_execfix

void qish_fixed_forget(void* ob) {
  struct qishfixedheader_st *hd = 0;
  int sz=0;
  assert (ob != 0 && ob >= qishgc_fixed_lo && ob <= qishgc_fixed_hi);
  hd = (struct qishfixedheader_st *) ob - 1;
  sz = qishgc_fixed_sizetab[hd->kfx_sizix];
  memset(ob, 0, sz);
  hd->kfx_destr = (qish_voidfun_t*)-1;
} /* end of qish_fixed_forget */

void
qishgc_minormark (void *ob)
{
  struct qishfixedheader_st *hd = 0;
  assert (ob != 0 && ob >= qishgc_fixed_lo && ob <= qishgc_fixed_hi);
  hd = (struct qishfixedheader_st *) ob - 1;
  assert (hd->kfx_magic == KFX_MAGIC_USED);
  if (hd->kfx_mark)
    return;
  hd->kfx_mark = 1;
  qish_fixedminor_scan (ob, qishgc_fixed_sizetab[hd->kfx_sizix]);
}				// end of qishgc_minormark

void
qishgc_fullmark (void *ob)
{
  struct qishfixedheader_st *hd = 0;
  assert (ob != 0 && ob >= qishgc_fixed_lo && ob <= qishgc_fixed_hi);
  hd = (struct qishfixedheader_st *) ob - 1;
  assert (hd->kfx_magic == KFX_MAGIC_USED);
  if (hd->kfx_mark)
    return;
  hd->kfx_mark = 1;
  qish_fixedfull_scan (ob, qishgc_fixed_sizetab[hd->kfx_sizix]);
}				// end of qishgc_fullmark

void
qishgc_minormarkscan (void *ob)
{
  struct qishfixedheader_st *hd = 0;
  assert (ob != 0 && ob >= qishgc_fixed_lo && ob <= qishgc_fixed_hi);
  hd = (struct qishfixedheader_st *) ob - 1;
  assert (hd->kfx_magic == KFX_MAGIC_USED);
  hd->kfx_mark = 1;
  qish_fixedminor_scan (ob, qishgc_fixed_sizetab[hd->kfx_sizix]);
}				// end of qishgc_minormark

void
qishgc_fullmarkscan (void *ob)
{
  struct qishfixedheader_st *hd = 0;
  assert (ob != 0 && ob >= qishgc_fixed_lo && ob <= qishgc_fixed_hi);
  hd = (struct qishfixedheader_st *) ob - 1;
  assert (hd->kfx_magic == KFX_MAGIC_USED);
  hd->kfx_mark = 1;
  qish_fixedfull_scan (ob, qishgc_fixed_sizetab[hd->kfx_sizix]);
}				// end of qishgc_fullmark


static void
qishgc_full_stackscan (void)
{
  struct qishgc_framedescr_st *fram = 0;
  //.qish_dbgprintf("scanptr=%p oldcur=%p after %d constants", scanptr, qishgc_old_cur, n);
  for (fram = (struct qishgc_framedescr_st *) qishgc_birth.bt_qishgcf; fram;
       fram = (struct qishgc_framedescr_st *) (fram->gcf_prev)) {
    int i;
    int nbparams = fram->gcf_point->gcd_nbparam;
    int nblocals = fram->gcf_point->gcd_nblocal;
    void **args = fram->gcf_args;
    void **locals = fram->gcf_locals;
    //.qish_dbgprintf("fram=%p nbparams=%d args=%p nblocals=%d locals=%p", fram, nbparams, args, nblocals, locals);
#if QISH_ARGS_UP
    /* actual arguments have increasing adresses from args; optimize
       for few arguments */
    switch (nbparams) {
    default:
      for (i = nbparams - 1; i >= 7; i--)
	QISHGC_FULL_UPDATE (args[i]);
    case 7:
      QISHGC_FULL_UPDATE (args[6]);	/*fallthru */
    case 6:
      QISHGC_FULL_UPDATE (args[5]);	/*fallthru */
    case 5:
      QISHGC_FULL_UPDATE (args[4]);	/*fallthru */
    case 4:
      QISHGC_FULL_UPDATE (args[3]);	/*fallthru */
    case 3:
      QISHGC_FULL_UPDATE (args[2]);	/*fallthru */
    case 2:
      QISHGC_FULL_UPDATE (args[1]);	/*fallthru */
    case 1:
      QISHGC_FULL_UPDATE (args[0]);	/*fallthru */
    case 0:
      break;
    };
#endif //QISH_ARGS_UP
    ///////
#if QISH_ARGS_DOWN
    /* actual arguments have decreasing adresses from args; optimize
       for few arguments */
    switch (nbparams) {
    default:
      for (i = nbparams - 1; i >= 7; i--)
	QISHGC_FULL_UPDATE (args[-i]);
    case 7:
      QISHGC_FULL_UPDATE (args[-6]);	/*fallthru */
    case 6:
      QISHGC_FULL_UPDATE (args[-5]);	/*fallthru */
    case 5:
      QISHGC_FULL_UPDATE (args[-4]);	/*fallthru */
    case 4:
      QISHGC_FULL_UPDATE (args[-3]);	/*fallthru */
    case 3:
      QISHGC_FULL_UPDATE (args[-2]);	/*fallthru */
    case 2:
      QISHGC_FULL_UPDATE (args[-1]);	/*fallthru */
    case 1:
      QISHGC_FULL_UPDATE (args[0]);	/*fallthru */
    case 0:;
    };
#endif //QISH_ARGS_DOWN
    /* optimize for few locals */
    switch (nblocals) {
    default:
      for (i = nblocals - 1; i >= 16; i--)
	QISHGC_FULL_UPDATE (locals[i]);
    case 16:
      QISHGC_FULL_UPDATE (locals[15]);	/*fallthru */
    case 15:
      QISHGC_FULL_UPDATE (locals[14]);	/*fallthru */
    case 14:
      QISHGC_FULL_UPDATE (locals[13]);	/*fallthru */
    case 13:
      QISHGC_FULL_UPDATE (locals[12]);	/*fallthru */
    case 12:
      QISHGC_FULL_UPDATE (locals[11]);	/*fallthru */
    case 11:
      QISHGC_FULL_UPDATE (locals[10]);	/*fallthru */
    case 10:
      QISHGC_FULL_UPDATE (locals[9]);	/*fallthru */
    case 9:
      QISHGC_FULL_UPDATE (locals[8]);	/*fallthru */
    case 8:
      QISHGC_FULL_UPDATE (locals[7]);	/*fallthru */
    case 7:
      QISHGC_FULL_UPDATE (locals[6]);	/*fallthru */
    case 6:
      QISHGC_FULL_UPDATE (locals[5]);	/*fallthru */
    case 5:
      QISHGC_FULL_UPDATE (locals[4]);	/*fallthru */
    case 4:
      QISHGC_FULL_UPDATE (locals[3]);	/*fallthru */
    case 3:
      QISHGC_FULL_UPDATE (locals[2]);	/*fallthru */
    case 2:
      QISHGC_FULL_UPDATE (locals[1]);	/*fallthru */
    case 1:
      QISHGC_FULL_UPDATE (locals[0]);	/*fallthru */
    case 0:;
    };
  }				// end for fram;
  /// update extra full  roots if applicable
  qish_dbgprintf ("before calling qish_extra_full_p");
  qish_extra_full ();
  qish_dbgprintf ("after calling qish_extra_full_p");
}				// end of qishgc_full_stackscan



/**************************************************************
 * major (or full) garbage collection, with some preforwarding
 **************************************************************/
static void
qishgc_full (int siz, int nbforw, void**oldforw, void**newforw)
{
  int n = 0;
  void *prevoldlo = qishgc_old_lo;
  void *prevoldhi = qishgc_old_hi;
  void *prevoldcur = (void *) qishgc_old_cur;
  void *newold = 0;
  void *scanptr = 0;
  void *newbirth = 0;
  int birthsiz = 0;
  int newoldsiz = 0;
  int oldsiz = 0;
#ifdef __sparc__
  asm ("ta 3");			// flush register on sparc
#endif //__sparc__
  qish_dbgprintf ("qishgc_full prev old=%p-%p oldcur=%p siz=%d", prevoldlo,
		  prevoldhi, prevoldcur, siz);
  dbgmemap ("before full gc");
  qish_full_prehook ();
  qishgc_old_cur = 0;
  if (siz < 0)
    siz = 0;
  else if (siz >= MAX_BIRTH_SIZE)
    qish_panic ("huge birth memory size requested [full gc] (%dKbytes)",
		siz >> 10);
  qish_dbgprintf ("gc_full prev birth=%p-%p cur=%p", qishgc_birth.bt_lo,
		  qishgc_birth.bt_hi, qishgc_birth.bt_cur);
  newoldsiz =
    ((char *) prevoldcur - (char *) prevoldlo) + siz + MIN_BIRTH_SIZE +
    FULL_GC_THRESHOLD + 16 * QISH_PAGESIZE;
  newoldsiz += ((char *) qishgc_birth.bt_cur - (char *) qishgc_birth.bt_lo);
  newoldsiz |= (QISH_PAGESIZE - 1);
  newoldsiz++;
  clear_all_fixed_marks ();
  // allocate the new old region 
  newold = qishgc_getmem ("new old region", newoldsiz);
  qishgc_old_cur = scanptr = (char *) newold + 32 * sizeof (void *);
  /// preforwarding
  for (n = 0; n<nbforw; n++) {
    if (newforw[n]) 
      QISHGC_FULL_PTR_UPDATE(newforw[n]);;
    if (QISH_IS_MOVING_PTR(oldforw[n])) {
      ((void**)(oldforw[n]))[0] = 0;
      ((void**)(oldforw[n]))[1] = newforw[n];
    }
  };
  ////
  for (n = 0; n < QISH_NB_ROOTS; n++)
    QISHGC_FULL_UPDATE (((void **) qish_roots)[n]);
  for (n = 0; n < QISH_MAX_MODULE; n++)
    QISHGC_FULL_UPDATE (qish_moduletab[n].km_constant);
  memset ((void *) qish_globconstabwrbar, 0, sizeof (qish_globconstabwrbar));
  for (n = 0; n < QISH_MAXNBCONST; n++)
    if (qish_globconstab[n])
      QISHGC_FULL_UPDATE (qish_globconstab[n]);
  qishgc_changedconstrank = 0;
  qishgc_full_stackscan ();
  /// C++ hook
  qishcpp_full();
  /* Chesney loop */
  while ((qish_uaddr_t) scanptr < (qish_uaddr_t) qishgc_old_cur) {
    void *scanob = scanptr;
    scanptr = qish_full_scan (scanob);
    qish_dbgprintf("scanob=%p scanptr=%p", scanob, scanptr);
    assert (scanptr != 0 && scanob != 0);
    assert ((((qish_uaddr_t) scanptr) % sizeof (void *)) == 0);
    // skip to next non zero word
    while ((qish_uaddr_t) scanptr < (qish_uaddr_t) qishgc_old_cur
	   && (*(void **) scanptr) == 0)
      scanptr = ((void **) scanptr) + 1;
  };
  destroy_unmarked_fixed ();
  destroy_unmarked_execfix ();
  qishgc_releasemem ("previous old region (full GC)", prevoldlo,
		     (char *) prevoldhi - (char *) prevoldlo);
  /* map a new birth region */
  birthsiz = siz + MIN_BIRTH_SIZE;
  birthsiz |= (QISH_PAGESIZE - 1);
  birthsiz++;
  newbirth = qishgc_getmem ("new birth region", birthsiz);
  /* unmap previous birth region */
  qishgc_releasemem ("previous birth region (full GC)",
		     (void *) qishgc_birth.bt_lo,
		     (char *) qishgc_birth.bt_hi -
		     (char *) qishgc_birth.bt_lo);
  qishgc_birth.bt_lo = newbirth;
  qishgc_birth.bt_cur = (char *) newbirth + 2 * sizeof (void *);
  qishgc_birth.bt_hi = (char *) newbirth + birthsiz;
  // offset to  qishgc_birth.bt_storeptr is related to those in qish_write_notify 
  qishgc_birth.bt_storeptr = (void *) (((void **) qishgc_birth.bt_hi) - 5);
  /* unmap excess part of old region */
  qish_dbgprintf ("oldcur=%p newold=%p", qishgc_old_cur, newold);
  oldsiz = (char *) qishgc_old_cur - (char *) newold;
  oldsiz += birthsiz + 2 * MIN_BIRTH_SIZE + 4 * QISH_PAGESIZE;
  oldsiz |= (QISH_PAGESIZE - 1);
  oldsiz++;
  qish_dbgprintf ("oldsiz= %dK newoldsiz %dK newold=%p", oldsiz >> 10,
		  newoldsiz >> 10, newold);
  assert (oldsiz <= newoldsiz);
  if (oldsiz < newoldsiz) {
    qish_dbgprintf ("oldsiz=%dK newoldsiz=%dK unmapping %p-%p", oldsiz << 10,
		    newoldsiz << 10, (char *) newold + oldsiz,
		    (char *) newold + newoldsiz);
    if (munmap ((char *) newold + oldsiz, newoldsiz - oldsiz))
      qish_epanic ("cannot unmap excess %dKbytes of old region",
		   (newoldsiz - oldsiz) >> 10);
    qishgc_old_hi = (char *) qishgc_old_lo + oldsiz;
    qish_dbgprintf ("munmap excess old %p-%p %dK", (char *) newold + oldsiz,
		    (char *) newold + newoldsiz, (newoldsiz - oldsiz) >> 10);
  };
  qishgc_old_lo = newold;
  qishgc_old_hi = (char *) newold + oldsiz;
  qish_dbgprintf ("after fullgc old=%p-%p %dK", (char *) qishgc_old_lo,
		  (char *) qishgc_old_hi, oldsiz >> 10);
  qish_full_posthook ();
  qish_nb_full_collections++;
}				// end of qishgc_full


/* the public interface to the garbage collector has to decide between
minor and full (major) GC */
void
qish_garbagecollect (int size, int needfull)
{
  static int gc_count;
  gc_count++;
#ifndef NDEBUG
  if (qish_debug)
    putchar ('\n');
  qish_dbgprintf ("begin %d GC ***** [old=%p-%p] [birth=%p-%p]", gc_count,
	     qishgc_old_lo, qishgc_old_hi, qishgc_birth.bt_lo,
	     qishgc_birth.bt_hi);
#endif

#ifndef QISH_ROUTINE
  if (!qish_gc_copy_p)
    qish_panic ("no qish_gc_copy_p function (gc%d)", gc_count);
  if (!qish_minor_scan_p)
    qish_panic ("no qish_minor_scan_p function (gc%d)", gc_count);
  if (!qish_full_scan_p)
    qish_panic ("no qish_full_scan_p function (gc%d)", gc_count);
#endif
  assert (qishgc_birth.bt_in_gc == 0);
  qishgc_birth.bt_in_gc = 1;
  assert ((qish_uaddr_t) qishgc_old_lo <= (qish_uaddr_t) qishgc_old_cur);
  assert ((qish_uaddr_t) qishgc_old_cur <= (qish_uaddr_t) qishgc_old_hi);
  assert ((qish_uaddr_t) qishgc_birth.bt_lo <=
	  (qish_uaddr_t) qishgc_birth.bt_cur);
  assert ((qish_uaddr_t) qishgc_birth.bt_cur <=
	  (qish_uaddr_t) qishgc_birth.bt_storeptr);
  assert ((qish_uaddr_t) qishgc_birth.bt_storeptr <=
	  (qish_uaddr_t) qishgc_birth.bt_hi);
  if ((char *) qishgc_old_cur + size + MIN_BIRTH_SIZE +
      ((char *) qishgc_birth.bt_hi - (char *) qishgc_birth.bt_lo)
      > (char *) qishgc_old_hi)
    needfull = 1;
#if FULL_GC_PERIOD>0
  else if (gc_count % FULL_GC_PERIOD == 0)
    needfull = 1;
#endif
  if (needfull)
    qishgc_full (size, 0, 0, 0);
  else
    qishgc_minor (size);
  qish_need_gc = 0;
  assert ((qish_uaddr_t) qishgc_old_lo <= (qish_uaddr_t) qishgc_old_cur);
  assert ((qish_uaddr_t) qishgc_old_cur <= (qish_uaddr_t) qishgc_old_hi);
  assert ((qish_uaddr_t) qishgc_birth.bt_lo <=
	  (qish_uaddr_t) qishgc_birth.bt_cur);
  assert ((qish_uaddr_t) qishgc_birth.bt_cur <=
	  (qish_uaddr_t) qishgc_birth.bt_storeptr);
  assert ((qish_uaddr_t) qishgc_birth.bt_storeptr <=
	  (qish_uaddr_t) qishgc_birth.bt_hi);
  qishgc_birth.bt_in_gc = 0;
  qish_dbgprintf ("end %d GC [%s] old=%p-%p birth=%p-%p *****", gc_count,
	     needfull ? "full" : "minor", qishgc_old_lo, qishgc_old_hi,
	     qishgc_birth.bt_lo, qishgc_birth.bt_hi);
}				// end of qish_garbagecollect



void qish_preforward_garbagecollect(int nbforw, void*oldforw[], void*newforw[]) {
  int i;
  if (nbforw<=0) 
    return;
  assert(oldforw != 0);
  assert(newforw != 0);
  for (i=0; i<nbforw; i++) {
    if (newforw[i] && !QISH_IS_MOVING_PTR(newforw[i])) 
      qish_panic("invalid new forward #%d = %p", i, newforw[i]);
    if (oldforw[i] && !QISH_IS_MOVING_PTR(oldforw[i])) 
      qish_panic("invalid old forward #%d = %p", i, oldforw[i]);
  }
  qishgc_full (2*MIN_BIRTH_SIZE, nbforw, oldforw, newforw);
} /* end of qish_preforward_garbagecollect */


void
qishgc_init (void)
{
  int oldsiz = 0;
  int birthsiz = 0;
  static int gcinitialized;
  if (gcinitialized)
    return;
  gcinitialized = 1;
  //
#ifdef TRACEMEM
  {
    char *tracnam = getenv ("QISHTRACEMEM");
    time_t now = 0;
    if (tracnam)
      qish_tracememfile = fopen (tracnam, "w");
    if (qish_tracememfile) {
      time (&now);
      fprintf (qish_tracememfile, "#Tracing Qish memory pid %d on %s\n",
	       (int) getpid (), ctime (&now));
      fflush (qish_tracememfile);
    }
  }
#endif /*TRACEMEM*/
    //
    if ((qishgc_fixed_lo =
	 mmap ((void *) 0, (size_t) FIXEDHEAP_MAXLEN, PROT_NONE,
	       MAP_ANON | MAP_NORESERVE | MAP_PRIVATE, -1, 0)) == MAP_FAILED)
    qish_epanic (" cannot initialize fixed heap range of %d MBytes",
		 FIXEDHEAP_MAXLEN >> 20);
  qish_dbgprintf ("mmap fixed adrzon %p-%p %dK", qishgc_fixed_lo,
	     (char *) qishgc_fixed_lo + FIXEDHEAP_MAXLEN,
	     FIXEDHEAP_MAXLEN >> 10);
  if (mmap
      (qishgc_fixed_lo, (size_t) FIXEDHEAP_INITLEN,
       PROT_READ | PROT_WRITE | PROT_EXEC, MAP_ANON | MAP_PRIVATE | MAP_FIXED,
       -1, 0) == MAP_FAILED)
    qish_epanic ("cannot initialize fixed heap of %d MBytes",
		 FIXEDHEAP_INITLEN >> 20);
  qishgc_fixed_hi = (void *) ((char *) qishgc_fixed_lo + FIXEDHEAP_INITLEN);
  qish_dbgprintf ("mmap fixed %p-%p %dK", qishgc_fixed_lo, qishgc_fixed_hi,
	     FIXEDHEAP_INITLEN >> 10);
  free_fixed_chunk (0, qishgc_fixed_lo, FIXEDHEAP_INITLEN);
  oldsiz = 4 * MIN_BIRTH_SIZE;
  qishgc_old_lo = qishgc_getmem ("initial old region", oldsiz);
  qishgc_old_cur = (char *) qishgc_old_lo + 2 * sizeof (void *);
  qishgc_old_hi = (char *) qishgc_old_lo + oldsiz;
  birthsiz = 2 * MIN_BIRTH_SIZE;
  assert (birthsiz + MIN_BIRTH_SIZE < oldsiz);
  qishgc_birth.bt_lo = qishgc_getmem ("initial birth region", birthsiz);
  qishgc_birth.bt_cur = (char *) qishgc_birth.bt_lo + 2 * sizeof (void *);
  qishgc_birth.bt_hi = (char *) qishgc_birth.bt_lo + birthsiz;
  // offset to   qishgc_birth.bt_storeptr is related to those in qish_write_notify
  qishgc_birth.bt_storeptr = (void *) (((void **) qishgc_birth.bt_hi) - 4);
  qishgc_fixedhigh_head = 0;
}				/* end of qishgc_init */


/* eof $Id: qigc.c 36 2005-10-12 19:40:42Z basile $ */
