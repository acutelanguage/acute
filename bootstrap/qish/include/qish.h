#ifndef __QISH__INCLUDED__
/* file QISH/include/qish.h */
/* emacs Time-stamp: <2005 Aug 17 11h06 CEST {qish.h} Basile STARYNKEVITCH> */
#define __QISH__INCLUDED__
/* prcsid $Id: qish.h 35 2005-08-17 09:44:19Z basile $ */
/* prcsproj $ProjectHeader: Qish 1.9 Tue, 28 Dec 2004 12:08:57 +0100 basile $ */

//  Copyright © 2002-2005 Basile STARYNKEVITCH

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

#ifdef __cplusplus
extern "C" {
#endif


#include <assert.h>
#include <stdlib.h>
#include <stdlib.h>

/* include a generated include file defining important system
   dependent constants like QISH_PAGESIZE etc...*/
#include "_qishgen.h"

#ifdef _REENTRANT		/* warn that qish is not reentrant */
#warning qish is not yet multithreadable
#endif

#ifndef STRICT_C99
#define FLEXIBLE_ARRAY_DIM 0
#else
#define FLEXIBLE_ARRAY_DIM
#endif

// defined in generated _date.c file
  extern const char qishlib_date[];
  extern const long qishlib_time;	/*actually a time_t */
  extern const char qishlib_srcdir[];
  extern const char qishlib_hostname[];
  extern const char qishlib_system[];
  extern const char qishlib_version[];
  extern const int qishlib_minorvnum;
  extern const char qishlib_majorvers[];
  extern const char qishlib_prcsdate[];


/// counters for minor and full gc
  extern int qish_nb_minor_collections;
  extern int qish_nb_full_collections;

// sensitive compile time constants in lib/qigc.c 
  extern const int qish_min_birth_size /*minimal birth size, eg 8Mb */ ;
  extern const int qish_max_birth_size	/*maximal birth & object size, eg 64Mb */
   ;
  extern const int qish_full_gc_threshold
    /*threshold for full collection, eg 24Mb */ ;


  extern const int qish_nil[];	// this is such that &qish_nil == 0 for BEGIN_SIMPLE_FRAME

// minimal address
#define QISH_MIN_ADDR     ((qish_uaddr_t)QISH_PAGESIZE)

#if defined(__GNUC__) && !defined(_QISH_NO_BUILTIN_EXPECT)
  /* compiler hints - but simple benches dont show them as significant */
#define QISH_LIKELY(A) __builtin_expect((A),1)
#define QISH_UNLIKELY(A) __builtin_expect((A),0)
#else
#define QISH_LIKELY(A) (A)
#define QISH_UNLIKELY(A) (A)
#endif
  
  extern int qish_debug;
#ifndef NDEBUG
#define qish_dbgprintf(Fmt, ...) do {if (qish_debug) \
  fprintf(stderr,"%s:%d!! " Fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__); \
  fflush(stderr);} while(0)
#else
#define qish_dbgprintf(Fmt, Args...) do {} while(0)
#endif


#ifndef QISH_ROUTINE

/******************* routine pointers to be set by the application *****************/


/* copy source to destination and return the end of copy */
// src is the original object to be copied
// dst is the adress after which should go the copy
// *padr should be set to the new adress of the copy 
/// usually *padr=dst or the word after, if needed alignement 
// the return adress is the first word after the copied object
  extern void *(*qish_gc_copy_p) (void **padr, void *dst, const void *src);
#define qish_gc_copy(Padr,Dst,Src) (*qish_gc_copy_p)((Padr),(Dst),(Src))
/* scan a movable object for minor GC, updating its pointers with
   QISHGC_MINOR_UPDATE, returning the next word after the scanned
   object */

  extern void *(*qish_minor_scan_p) (void *ptr);
#define qish_minor_scan(Ptr) (*qish_minor_scan_p)((Ptr))
/* scan a movable object for full GC, updating its pointers with
   QISHGC_FULL_UPDATE, returning the next word after the scanned
   object */
  extern void *(*qish_full_scan_p) (void *ptr);
#define qish_full_scan(Ptr) (*qish_full_scan_p)((Ptr))

/* scan a fixed object for minor GC, updating its pointers */
  extern void (*qish_fixedminor_scan_p) (void *ptr, int size);
#define qish_fixedminor_scan(Ptr,Sz) (*qish_fixedminor_scan_p)((Ptr),(Sz))
/* scan a fixed object for full GC, updating its pointers */
  extern void (*qish_fixedfull_scan_p) (void *ptr, int size);
#define qish_fixedfull_scan(Ptr,Sz) (*qish_fixedfull_scan_p)((Ptr),(Sz))

/* update other roots on minor GC, so call QISHGC_MINOR_UPDATE on
   extra GC roots, such as pointers inside the stack of an interpreter
   or virtual machine */
  extern void (*qish_extra_minor_p) (void);
#define qish_extra_minor() do {if (qish_extra_minor_p) (*qish_extra_minor_p)();} while(0)
/* update other roots on full GC, so call QISHGC_FULL_UPDATE on
   extra GC roots, such as pointers inside the stack of an interpreter
   or virtual machine */
  extern void (*qish_extra_full_p) (void);
#define qish_extra_full() do {if(qish_extra_full_p) (*qish_extra_full_p)();} while(0)

  /* we provide application hooks for full garbage collections; these
     hooks might give some user feedback on full GC; they cannot
     allocate or even access any Qish-collected object! */
  /* function called at start of full garbage collection */
  extern void (*qish_full_prehook_p) (void);
#define qish_full_prehook() do{if (qish_full_prehook_p) (*qish_full_prehook_p)();}while(0)
  /* function called at end of full garbage collection */
  extern void (*qish_full_posthook_p) (void);
#define qish_full_posthook() do{if (qish_full_posthook_p) (*qish_full_posthook_p)();}while(0)
#else				/* QISH_ROUTINE */

  /* the application may be compide with -DQISH_ROUTINE; in that case,
     the qishrout_* functions below should be provided by the application */

/* see comments above for the meaning of the routines */
  extern void *qishrout_gc_copy (void **padr, void *dst, const void *src);
#define qish_gc_copy(Ad,Dst,Src) qishrout_gc_copy((Ad),(Dst),(Src))
  extern void *qishrout_minor_scan (void *ptr);
#define qish_minor_scan(Ptr) qishrout_minor_scan((Ptr))
  extern void *qishrout_full_scan (void *ptr);
#define qish_full_scan(Ptr) qishrout_full_scan((Ptr))
  extern void qishrout_fixedminor_scan (void *ptr, int siz);
#define qish_fixedminor_scan(Ptr,Sz) qishrout_fixedminor_scan((Ptr),(Sz))
  extern void qishrout_fixedfull_scan (void *ptr, int siz);
#define qish_fixedfull_scan(Ptr,Sz) qishrout_fixedfull_scan((Ptr),(Sz))
  extern void qishrout_extra_minor (void);
#define qish_extra_minor() qishrout_extra_minor()
  extern void qishrout_extra_full (void);
#define qish_extra_full() qishrout_extra_full()
  extern void qishrout_full_prehook (void);
#define qish_full_prehook() qishrout_full_prehook()
  extern void qishrout_full_posthook (void);
#define qish_full_posthook() qishrout_full_posthook()
#endif				/*no QISH_ROUTINE */

  // C++ support is always done thru a routine pointer (even with QISH_ROUTINE)
  extern void (*qishcpp_minor_p)(void);
  extern void (*qishcpp_full_p)(void);
#define qishcpp_minor() do{if(qishcpp_minor_p) (*qishcpp_minor_p)(); } while(0)
#define qishcpp_full() do{if(qishcpp_full_p) (*qishcpp_full_p)(); } while(0)

//////////////////////////////////////////////////////////////////////////////




  struct qishgc_safepointdescr_st {
    unsigned short gcd_nbparam;
    unsigned short gcd_nblocal;
    unsigned long gcd_rank;
    void *gcd_data;
    const char *gcd_cname;
  };

  struct qishgc_framedescr_st {
    const struct qishgc_safepointdescr_st *gcf_point;
    volatile struct qishgc_framedescr_st *gcf_prev;
    void **gcf_args;
    void **gcf_locals;
  };

/* header before fixed address objects */
  struct qishfixedheader_st {
    unsigned short kfx_magic;	/* magic number */
    unsigned char kfx_sizix;	/* size index */
    unsigned char kfx_mark;	/* GC mark */
    void (*kfx_destr) (void *);	/* destructor (called with the object) */
    struct qishfixedheader_st *kfx_phyprev;	/* link to *physically* previous object header (by address) */
    struct qishfixedheader_st *kfx_next;	/* link to logically next object (of same size) */
    char kfx_data[FLEXIBLE_ARRAY_DIM]
      __attribute__ ((__aligned__ (2 * sizeof (double))));
  };




  struct qishgc_birth_st {	/* in multithreaded each thread has its own such structure */
#define QISHGC_BIRTH_MAGIC 0xc26fbf
    int bt_magic;
    /* the birth region has a low and a high pointer */
    volatile void *bt_lo;
    volatile void *bt_hi;
    /* growing downwards is the store pointer (for GC write barrier) */
    volatile void **bt_storeptr;
    /* the current allocation pointer points to free zone */
    volatile void *bt_cur;
    /* the rank up to which constants have to be updated on minor GC */
    volatile int bt_changedconstrank;
    /* the current GC frame is */
    volatile struct qishgc_framedescr_st *bt_qishgcf;
    /* nonzero if inside GC */
    volatile int bt_in_gc;
  };

  extern struct qishgc_birth_st qishgc_birth;

  extern volatile int qish_need_full_gc;

/* the old moving region has a low and high pointer */
  extern void *qishgc_old_lo;
  extern void *qishgc_old_hi;
/* current pointer in old generation */
  extern void *qishgc_old_cur;

/* the fixed object region has a low and high pointer */
  extern void *qishgc_fixed_lo;
  extern void *qishgc_fixed_hi;

  extern volatile int qish_need_gc;

  enum { QISH_GC_MINOR = 0, QISH_GC_FULL };

  /// call the garbage collector; first argument is new required
  /// birthsize, eg wanted size; second argument is a boolean flag to
  /// force full garbage collection
  void qish_garbagecollect (int size, int needfull);

  /// preforward some application pointers from a set of old pointers
  /// to be forwarded into a set of new pointers; costly, since
  /// require a full garbage collection; 
  void qish_preforward_garbagecollect(int nbforw, void*oldforw[], void*newforw[]);

// test if a pointer is moving (outside of GC routines!)
#define QISH_IS_MOVING_PTR(P) ((((qish_uaddr_t)P)&3)==0 &&		\
  ( ((void*)(P)>=qishgc_birth.bt_lo && (void*)(P)<=qishgc_birth.bt_hi)	\
    || ((void*)(P)>=qishgc_old_lo && (void*)(P)<=qishgc_old_hi) ))

  // test if a pointer is fixed (outside of GC routines!)
#define QISH_IS_FIXED_PTR(P)  ((((qish_uaddr_t)P)&3)==0 &&		\
  ( ((void*)(P)>=qishgc_fixed_lo && (void*)(P)<=qishgc_fixed_hi)))

#ifndef NO_QISH_INLINE
  static inline void
    qish_write_notify (void *ob) {
    if ((qish_uaddr_t) ob < (qish_uaddr_t) qishgc_old_lo
	|| (qish_uaddr_t) ob > (qish_uaddr_t) qishgc_old_hi
	|| ob == qishgc_birth.bt_storeptr[1]
	|| ob == qishgc_birth.bt_storeptr[2]
	|| ob == qishgc_birth.bt_storeptr[3])
      return;
    *(qishgc_birth.bt_storeptr--) = ob;
    if (QISH_UNLIKELY(((void **) qishgc_birth.bt_storeptr - 4 <=
		       (void **) qishgc_birth.bt_cur)))
      qish_garbagecollect (QISH_PAGESIZE, 0);
  }				// end of qish_write_notify
#endif				/*NO_QISH_INLINE */

#define QISH_WRITE_NOTIFY(Ob) do {					\
    void* _qiwr= (void*)(Ob);						\
      if ( _qiwr == qishgc_birth.bt_storeptr[1]				\
          || _qiwr == qishgc_birth.bt_storeptr[2]			\
          || _qiwr == qishgc_birth.bt_storeptr[3]) { }			\
  else if (((qish_uaddr_t) _qiwr >= (qish_uaddr_t) qishgc_old_lo	\
      && (qish_uaddr_t) _qiwr < (qish_uaddr_t) qishgc_old_hi)		\
|| QISH_IS_FIXED_PTR(_qiwr))  {						\
  *(qishgc_birth.bt_storeptr--) = _qiwr;				\
  if (QISH_UNLIKELY((void**) qishgc_birth.bt_storeptr - 4		\
      <= (void **) qishgc_birth.bt_cur))				\
    qish_garbagecollect (QISH_PAGESIZE, 0);				\
  }									\
} while(0)

#ifndef NO_QISH_INLINE
  /* tagged int (i.e. unaligned pointers) */ static inline int
    qish_is_tagged_int (const void *ptr) {
    return (((qish_uaddr_t) ptr) & 1);
  }				// end of qish_is_tagged_int
#endif /*NO_QISH_INLINE */

#define QISH_IS_TAGGED_INT(P) (((qish_uaddr_t) (P)) & 1)

// convert an integer to a "tagged pointer"
#define QISH_INT2TAGGED(I) ((void*)(((I)<<1)|1))
#define QISH_LONG2TAGGED(L) QISH_INT2TAGGED(L)

// convert a tagged pointer to an integer 
#define QISH_TAGGED2INT(P)  ((int)(((qish_uaddr_t)(P))>>1))
#define QISH_TAGGED2LONG(P) ((long)(((qish_uaddr_t)(P))>>1))


#ifndef NO_QISH_INLINE
  static inline int
    qish_tagged_to_int (void *ptr) {
    if (qish_is_tagged_int (ptr))
      return QISH_TAGGED2INT (ptr);
    else
      return 0;
  }				// end of qish_tagged_to_int

  static inline void *qish_int_to_tagged (int i) {
    return QISH_LONG2TAGGED ((long)i);
  }				// end of qish_int_to_tagged
#endif

  extern void qish_panic_at (int err, const char *fil, int lin,
			     const char *fct, const char *fmt, ...)
    __attribute__ ((noreturn, format (printf, 5, 6)));

    extern void qish_abort(void)
       __attribute__ ((noreturn));

#define qish_panic(Fmt,...) \
  qish_panic_at(0, __FILE__, __LINE__, __FUNCTION__, Fmt, ##__VA_ARGS__)

#define qish_epanic(Fmt,...) \
  qish_panic_at(errno, __FILE__, __LINE__, __FUNCTION__, Fmt, ##__VA_ARGS__)


/* the first word of an object should be non-zero (except for fowarded
   pointers) */

#define QISHGC_FORWARD(Pptr) do {					\
  void **_qipptr = *(void ***) (Pptr);					\
  if (*_qipptr == 0) {							\
    /* already forwarded */						\
    *(Pptr) = _qipptr[1];						\
  } else {								\
    void *_qiad = 0;							\
    void *_qiend =  (void*)qishgc_old_cur;				\
    _qiend = qish_gc_copy (&_qiad, _qiend, (const void *) _qipptr);	\
    _qipptr[0] = 0;							\
    _qipptr[1] = _qiad;							\
    *(Pptr) = _qiad;							\
    qishgc_old_cur = _qiend;						\
  }									\
}  while(0)


#define QISHGC_MINOR_UPDATE(Ptr) do {		\
  if (((qish_uaddr_t)(Ptr) & 3) == 0)		\
       QISHGC_MINOR_PTR_UPDATE(Ptr);		\
} while(0)

#define QISH_FOLLOW_FORWARD(Ptr) do {		\
  if (((qish_uaddr_t)(Ptr) & 3) == 0)		\
       QISH_FOLLOW_FORWARD_PTR(Ptr);		\
} while(0)

#define QISH_FOLLOW_FORWARD_PTR(Ptr) 	do	{	\
while ((qish_uaddr_t)(Ptr)>(qish_uaddr_t)QISH_MIN_ADDR	\
       && *((qish_uaddr_t*)(Ptr)) == 0)			\
  (Ptr) = (void*)(((qish_uaddr_t*)(Ptr))[1]);} while(0)



// same as QISHGC_MINOR_UPDATE but the pointer is known to not be a tagged int
#define QISHGC_MINOR_PTR_UPDATE(Ptr) do {	\
  qish_uaddr_t _qip;				\
  /* qishgc minor ptr update */                 \
  QISH_FOLLOW_FORWARD_PTR(Ptr);			\
  _qip = (qish_uaddr_t)(Ptr);			\
  if (_qip>=(qish_uaddr_t)qishgc_birth.bt_lo	\
      && _qip<(qish_uaddr_t)qishgc_birth.bt_hi)	\
    {QISHGC_FORWARD((void**)&(Ptr)); } 		\
 else if (QISH_IS_FIXED_PTR(_qip)) 		\
    qishgc_minormark((void*)_qip);} while(0)



#define QISHGC_FULL_UPDATE(Ptr) do { if (((qish_uaddr_t)(Ptr)&3) == 0) 	\
 QISHGC_FULL_PTR_UPDATE(Ptr); } while(0)

// same as QISHGC_FULL_UPDATE but the pointer is known to not be a tagged int
#define QISHGC_FULL_PTR_UPDATE(Ptr) do {				\
  qish_uaddr_t _qip;							\
  QISH_FOLLOW_FORWARD_PTR(Ptr);						\
  _qip = (qish_uaddr_t)(Ptr);						\
  if ((_qip>=(qish_uaddr_t)qishgc_birth.bt_lo				\
       && _qip<(qish_uaddr_t)qishgc_birth.bt_hi)			\
      || (_qip>=(qish_uaddr_t)qishgc_old_lo				\
          && _qip<(qish_uaddr_t)qishgc_old_hi))				\
    {QISHGC_FORWARD((void**)&(Ptr));}					\
  else if (_qip>=(qish_uaddr_t)qishgc_fixed_lo				\
       && _qip<=(qish_uaddr_t)qishgc_fixed_hi) {			\
       if (((qish_word_t)((struct qishfixedheader_st*)			\
			  (_qip))[-1].kfx_destr)==(qish_word_t)-1) 	\
	 *(void**)&(Ptr) = 0;						\
        else if (!((struct qishfixedheader_st*)				\
	    (_qip))[-1].kfx_mark)					\
    qishgc_fullmark((void*)(_qip)); }} while(0)

#define QIGC__SET_CUR_FRAME(F) qishgc_birth.bt_qishgcf = &(F)

#define QISH_ENTER_FRAME(Point, Params, Locals)	\
  struct qishgc_framedescr_st __qcf_cur;	\
  __qcf_cur.gcf_point = &Point;			\
  __qcf_cur.gcf_args = (void**)&Params;		\
  __qcf_cur.gcf_locals = (void**)&Locals;	\
  __qcf_cur.gcf_prev = qishgc_birth.bt_qishgcf;	\
  QIGC__SET_CUR_FRAME(__qcf_cur);

#define ENTER_FRAME QISH_ENTER_FRAME	/*compatibility */

#define QISH_ENTER_FRAME_WITHOUT_ARGS(Point,Locals)	\
  struct qishgc_framedescr_st __qcf_cur;	\
  __qcf_cur.gcf_point = &Point;			\
  __qcf_cur.gcf_args = 0;			\
  __qcf_cur.gcf_locals = (void**)&Locals;	\
  __qcf_cur.gcf_prev = qishgc_birth.bt_qishgcf;	\
  QIGC__SET_CUR_FRAME(__qcf_cur);

#define QISH_ENTER_FRAME_WITHOUT_LOCALS(Point, Params)	\
  struct qishgc_framedescr_st __qcf_cur;		\
  __qcf_cur.gcf_point = &Point;				\
  __qcf_cur.gcf_args = (void**)&Params;			\
  __qcf_cur.gcf_locals = 0;				\
  __qcf_cur.gcf_prev = qishgc_birth.bt_qishgcf;	       	\
  QIGC__SET_CUR_FRAME(__qcf_cur);

#define ENTER_FRAME_WITHOUT_LOCALS QISH_ENTER_FRAME_WITHOUT_LOCALS

#define QISH_EXIT_FRAME() \
  qishgc_birth.bt_qishgcf = __qcf_cur.gcf_prev
#define EXIT_FRAME QISH_EXIT_FRAME

#define QISH_DESCR_FRAME(Point, Nbparam, Nblocal, Rank)	\
   static struct qishgc_safepointdescr_st const Point	\
   = { Nbparam, Nblocal, Rank, (void*)0, __FUNCTION__ }
#define DESCR_FRAME QISH_DESCR_FRAME

#define QISH_DESCR_SIMPLE_FRAME(Nbparam, Nblocal)		\
   QISH_DESCR_FRAME(__gcd_curpt, Nbparam, Nblocal, 0)
#define DESCR_SIMPLE_FRAME QISH_DESCR_SIMPLE_FRAME

#define QISH_ENTER_SIMPLE_FRAME(Params, Locals)	\
   QISH_ENTER_FRAME(__gcd_curpt, Params, Locals)
#define ENTER_SIMPLE_FRAME QISH_ENTER_SIMPLE_FRAME

// deprecated macro, use one of the three macros below and declare a
// volatile struct { } _locals_
#define QISH_BEGIN_SIMPLE_FRAME(Nbparam, Params, Nblocal, Locals) \
   QISH_DESCR_SIMPLE_FRAME(Nbparam, Nblocal); \
   QISH_ENTER_SIMPLE_FRAME(Params, Locals)

#define BEGIN_SIMPLE_FRAME QISH_BEGIN_SIMPLE_FRAME

#define QISH_BEGIN_FRAME_WITHOUT_LOCALS(Nparam,Param1) \
   QISH_DESCR_SIMPLE_FRAME(Nparam, 0); \
   QISH_ENTER_FRAME_WITHOUT_LOCALS(__gcd_curpt, Param1)
#define BEGIN_FRAME_WITHOUT_LOCALS   QISH_BEGIN_FRAME_WITHOUT_LOCALS

#define QISH_BEGIN_LOCAL_FRAME_WITHOUT_ARGS()			\
   QISH_DESCR_SIMPLE_FRAME(0, sizeof(_locals_)/sizeof(void*));	\
   QISH_ENTER_FRAME_WITHOUT_ARGS(__gcd_curpt, _locals_)
#define BEGIN_LOCAL_FRAME_WITHOUT_ARGS  QISH_BEGIN_LOCAL_FRAME_WITHOUT_ARGS

#define QISH_BEGIN_LOCAL_FRAME(Nparam,Param1) \
  QISH_BEGIN_SIMPLE_FRAME(Nparam,Param1,sizeof(_locals_)/sizeof(void*),_locals_)
#define BEGIN_LOCAL_FRAME QISH_BEGIN_LOCAL_FRAME

  struct qish_excframe_st {
    unsigned spareheader;
    void *obj;
    volatile struct qishgc_framedescr_st *savedfram;
    int codexc;
    void *prevptr;
    char setjmpbuffer[QISH_SETJMP_SIZE];
  };

#define BEGIN_EXCEPT_BLOCK(Var) {				\
  struct qish_excframe_st _qhcf_excfram = { 0 };		\
  _qhcf_excfram.savedfram = qishgc_birth.bt_qishgcf;   		\
  _qhcf_excfram.codexc = 0;					\
  _qhcf_excfram.prevptr = Var;					\
  Var = (void*)&_qhcf_excfram;					\
  if (!(_qhcf_excfram.codexc 					\
	= setjmp(*(jmp_buf*)_qhcf_excfram.setjmpbuffer))) {

#define CATCH_EXCEPT_BLOCK(Cod,Exc)		\
  } else {					\
    Cod = _qhcf_excfram.codexc;			\
    Exc = _qhcf_excfram.obj;			\
    qishgc_birth.bt_qishgcf = _qhcf_excfram.savedfram;

#define END_EXCEPT_BLOCK(Var)					\
   Var = _qhcf_excfram.prevptr;		       			\
   memset(&_qhcf_excfram, 0, sizeof(struct qish_excframe_st));	\
} }

#define THROW_EXCEPTION(Var,Cod,Exobj) do {				\
  if (!(Var)) 								\
     qish_panic("nowhere to throw exception code %d", (Cod));		\
  ((struct qish_excframe_st*)(Var))->obj = (Exobj);			\
  longjmp(*(jmp_buf*)((struct qish_excframe_st*)(Var))->setjmpbuffer, 	\
	  (Cod));							\
} while (0)


  void qishgc_fullmark (void *);
  void qishgc_minormark (void *);
  void qishgc_minormarkscan(void*);
  void qishgc_fullmarkscan(void*);

/* the global roots - there should be a small number of them,
   typically less than the number of pointers in a rather shallow call
   stack; changing them can be done without notification  */
#define QISH_NB_ROOTS 64
  extern volatile void *qish_roots[QISH_NB_ROOTS];

#define QISH_MAX_MODULE 8192
  extern struct qishmodule_st {
    char km_name[100];		// module basename
    void *km_handle;		// handle from dlopen
    void *km_constant;		// garbage-collected constant
    void *km_data;		// extra (non-GCed) data
  } qish_moduletab[QISH_MAX_MODULE];

  /// return module rank iff ok (ie loaded module successfully) or else
  /// -1 the dirpath is a colon separated list of directories, like
  /// LD_LIBRARY_PATH; if modrank<0 find the module rank of given
  /// name, or a fresh one
  int qish_load_module (char *modname, char *dirpath, int modrank);
  /// get a symbol in a module (or find it in all modules if modrank < 0)
  void *qish_get_symbol (char *name, int modrank);
  /// unload a module (will be closed by qish_postponed_dlclose)
  void qish_unload_module (int modrank);

/// call this to dlclose modules while being sure that no module
/// functions are still active. The good place is in your topmost
/// [event or computing] loop near the main
  void qish_postponed_dlclose (void);

  typedef void (qish_voidfun_t) ();
  typedef void *(qish_ptrfun_t) ();
  typedef int (qish_intfun_t) ();
  typedef long (qish_longfun_t) ();
  typedef double (qish_doublefun_t) ();


// initialize the GC! mandatory to call it once before any GC or allocation
  void qishgc_init (void);


/// in addition to module constant, we have a global array of pointer constants
#define QISH_MAXNBCONST (1<<16)	/*should be a power of two */

  extern volatile void *qish_globconstab[QISH_MAXNBCONST + 1];
  extern volatile char qish_globconstabwrbar[(QISH_MAXNBCONST >> 8) + 1];
#define QISH_GLOBCONST(N) qish_globconstab[(N)&(QISH_MAXNBCONST-1)]
#define QISH_SET_GLOBCONST(N,V) do{				\
  qish_globconstabwrbar[((N) & (QISH_MAXNBCONST-1))>>8]=1;	\
  qish_globconstab[(N) & (QISH_MAXNBCONST-1)] = (V);		\
} while(0)

#ifndef NO_QISH_INLINE
  static inline void
    qish_change_module_constant (int rk, void *val) {
    assert (rk >= 0 && rk < QISH_MAX_MODULE);
    // even constant changed to new objects need notification!
    qish_moduletab[rk].km_constant = val;
    if (rk >= qishgc_birth.bt_changedconstrank)
      qishgc_birth.bt_changedconstrank = rk + 1;
  }				// end of qish_changeconstant

  static inline void *qish_module_constant (int rk) {
    assert (rk >= 0 && rk < QISH_MAX_MODULE);
    return qish_moduletab[rk].km_constant;
  }



// allocate an object in the birth region without specific alignement constraints
  static inline void *qish_allocate (int siz) {
    volatile void *res = 0;
    assert (siz >= (int) sizeof (void *));
    if (siz & (sizeof (void *) - 1)) {
      siz |= sizeof (void *) - 1;
      siz++;
    };
    if (qish_need_full_gc
	|| (qish_uaddr_t) (((char *) qishgc_birth.bt_cur) + siz) >=
	(qish_uaddr_t) (qishgc_birth.bt_storeptr - 2))
      qish_garbagecollect (siz + 4 * sizeof (double), 0);
    res = qishgc_birth.bt_cur;
    qishgc_birth.bt_cur = (char *) qishgc_birth.bt_cur + siz;
    return (void *) res;
  }				// end of qish_allocate
#endif /* NO_QISH_INLINE */

  // even constant changed to new objects need notification!    
#define QISH_CHANGE_MODULE_CONSTANT(Rk,Val) {		       	\
  assert ((Rk) >= 0 && (Rk) < QISH_MAX_MODULE);			\
  qish_moduletab[(Rk)].km_constant = (Val);			\
  if ((Rk) >= qishgc_changedconstrank)				\
    qishgc_changedconstrank = (Rk) + 1;				\
}

#define QISH_MODULE_CONSTANT(Rk) qish_moduletab[Rk].km_constant

#define QISH_ALLOCATE(Ptr,Siz) do {				\
  int _qialsz = (Siz);						\
  Ptr = 0;							\
  if (QISH_UNLIKELY(qish_need_gc		       		\
      || ((char *) qishgc_birth.bt_cur) + _qialsz		\
         >= (char *) (qishgc_birth.bt_storeptr - 2)))		\
    qish_garbagecollect (_qialsz+16*sizeof(void*), 0);		\
  Ptr = (void*) qishgc_birth.bt_cur;				\
  qishgc_birth.bt_cur = (char*)qishgc_birth.bt_cur + _qialsz;	\
} while(0)


  static inline void
    qish_reserve (int siz) {
    if (QISH_UNLIKELY((qish_uaddr_t) (((char *) qishgc_birth.bt_cur) + siz) >=
		      (qish_uaddr_t) (qishgc_birth.bt_storeptr - 2)))
      qish_garbagecollect (siz + 4 * sizeof (double), 0);
  }				// end of qish_reserve

// allocate an object in the birth region with an alignement (bigger than the machine word)
  static inline void *qish_allocate_aligned (int siz, int align) {
    void *res = 0;
    qish_uaddr_t ad = 0;
    assert (siz >= (int) sizeof (void *));
    assert (align == sizeof (int) || align == 2 * sizeof (int)
	    || align == 4 * sizeof (int) || align == 8 * sizeof (int)
	    || align == 16 * sizeof (int) || align == 32 * sizeof (int));
    if (siz & (align - 1)) {
      siz |= align - 1;
      siz++;
    };
    if (QISH_UNLIKELY((qish_uaddr_t) (((char *) qishgc_birth.bt_cur) + siz + 2 * align) >=
		      (qish_uaddr_t) (qishgc_birth.bt_storeptr - 2)))
      qish_garbagecollect (siz + 2 * align + 4 * sizeof (double), 0);
    ad = (qish_uaddr_t) qishgc_birth.bt_cur;
    if (ad & (align - 1)) {
      ad |= align - 1;
      ad++;
    };
    res = (void *) ad;
    qishgc_birth.bt_cur = (void *) (ad + siz);
    return res;
  }

#define QISH_ALLOCATE_TYPED(Ptr,Type) do {				\
  if (__alignof__(Type) <= sizeof(void*)) {				\
    QISH_ALLOCATE((Ptr),sizeof(Type));					\
  } else {								\
    (Ptr) = qish_allocate_aligned(sizeof(Type),__alignof__(Type));	\
  }									\
} while(0)

#define QISH_ALLOCATE_TYPED_GAP(Ptr,Type,Gap) do {	\
  int _qisz = sizeof(Type) + (Gap);			\
  if (__alignof__(Type) <= sizeof(void*)) {		\
    QISH_ALLOCATE((Ptr),_qisz);				\
  } else {						\
    (Ptr) = qish_allocate_aligned(_qisz,		\
				  __alignof__(Type));	\
  }							\
} while(0)

// allocate a fixed object with a destructor
  void *qish_fixed_alloc (size_t sz, void (*destr) (void *));

// allocate a fixed executable object with a destructor - useful for
// objects containing machine code
  void *qish_fixed_exec_alloc (size_t sz, void (*destr) (void *));

  // forget a fixed object ie clear it and clear any pointers to it
  void qish_fixed_forget(void* ptr);

// compute an hashcode of a string; if len<0 take len=strlen(str)
  unsigned qish_strhash (const char *str, int len);

// execvp and wait for termination - return exit status or -1 on signal
  int qish_sigexecvp (const char *file, char *const argv[]);

// return a prime number after x or 0 if too big
  int qish_prime_after (int x);

//// runtime parameters
  void qish_put_parameter (const char *param, const char *val);
  char *qish_parameter (const char *param);
  void qish_remove_parameter (const char *param);


////////////////////////////////////////////////////////////
// decode a character encoded like in XML with &lt; &gt; &apos; &quot;
// &nl; &cr; &bel; &ff; &tab; &esc; &nbsp; &#33; &#x1f;
// return -1 if not found
  int qish_decodexmlchar (unsigned char *buf, char **pend);

// input an XML char from a file - if strict is set don't read any of
// ' < > " characters
  int qish_getxmlc (FILE * f, int strict);
#define QISH_STRICT_XMLC 1
#define QISH_NOSTRICT_XMLC 0
// encode an xmlchar - return the next position in buffer; consume at most 8 bytes there
  char *qish_encodexmlchar (int c, char *buf);

#ifndef NO_QISH_INLINE
// output such a char on a file
  static inline void
    qish_putxmlc (int c, FILE * f) {
    char buf[12];
    buf[0] = 0;
    qish_encodexmlchar (c, buf);
    fputs (buf, f);
  }
#endif

////////////////////////////////////////////////////////////
//// config file parsing

/*** syntax of configuration files 
 * any line starting with a # in first column is a comment
 * other lines are: symbol = value 
 * symbol are like C identifiers
 * values have no spaces 
 ***/
  void qish_parse_configfile (const char *configname);

#ifdef __cplusplus
}				/* end of extern "C" */
#endif


#ifdef __cplusplus
//// additional C++ support
class QishCpp_StackRoots_Any {
  static QishCpp_StackRoots_Any* qishr_top_;
  QishCpp_StackRoots_Any* _qishr_prev;
  const int _qishr_nbroots;
  const int _qishr_nbextra;
  void*volatile* _qishr_extra;
  volatile void* _qishr_tabroots[];
  static void qishr_minor();
  static void qishr_full();
  class Qishr_installer_cl {
  public:
    Qishr_installer_cl();
  };
  static Qishr_installer_cl _qishr_init_;
 protected:
  QishCpp_StackRoots_Any(int nbroot, int nbextra, void**extra) :
    _qishr_prev(qishr_top_), _qishr_nbroots(nbroot), 
    _qishr_nbextra(nbextra), _qishr_extra(extra) {
    qishr_top_ = this;
    for (int i=0; i<_qishr_nbroots; i++) _qishr_tabroots[i]=0;
  };
  QishCpp_StackRoots_Any(int nbextra, void**extra, size_t sz) :
    _qishr_prev(qishr_top_), 
    _qishr_nbroots((sz-sizeof(QishCpp_StackRoots_Any))/sizeof(void*)), 
    _qishr_nbextra(nbextra), _qishr_extra(extra) {
    qishr_top_ = this;
    for (int i=0; i<_qishr_nbroots; i++) _qishr_tabroots[i]=0;
  };
  ~QishCpp_StackRoots_Any() {
    assert(qishr_top_ == this);
    qishr_top_ = _qishr_prev;
    for (int i=0; i<_qishr_nbroots; i++) _qishr_tabroots[i]=0;
  };
  void qishstack_minor_scan() {
    for (int i=0; i<_qishr_nbroots; i++)
      QISHGC_MINOR_UPDATE(_qishr_tabroots[i]);
    if (_qishr_extra) 
      for (int i=0; i<_qishr_nbextra; i++)
	QISHGC_MINOR_UPDATE(_qishr_extra[i]);
  };
  void qishstack_full_scan() {
    for (int i=0; i<_qishr_nbroots; i++)
      QISHGC_FULL_UPDATE(_qishr_tabroots[i]);
    if (_qishr_extra) 
      for (int i=0; i<_qishr_nbextra; i++)
	QISHGC_FULL_UPDATE(_qishr_extra[i]);
  };
};				/* end class QishCpp_StackRoots_Any */

/**
   following macros are for C++ code; typical use is like
   void foo(aGcPtr*p1, anotherGcPtr*p2) {
     QISHCPP_BEGIN_LOCALS
       aGcPtr*l1;
       aGcPtr*l2;
     QISHCPP_END_LOCALS(2,p1)
   }
 **/
#define QISHCPP_BEGIN_LOCALS volatile class QishCpp_Locals      \
   : QishCpp_StackRoots_Any {                                   \
   public:

#if QISH_ARGS_DOWN
#define QISHCPP_END_LOCALS(NbArg,FirstArg)              \
 QishCpp_Locals(int nbarg, void**firstarg)              \
    : QishCpp_StackRoots_Any(nbarg,                     \
                        nbarg+firstarg,                 \
                        sizeof(QishCpp_Locals)) {};     \
} _locals_(NbArg, static_cast<void**>((void*)&FirstArg));

#else

#define QISHCPP_END_LOCALS(NbArg,FirstArg)              \
 QishCpp_Locals(int nbarg, void**firstarg)              \
    : QishCpp_StackRoots_Any(nbarg,                     \
                        firstarg,                       \
                        sizeof(QishCpp_Locals)) {};     \
} _locals_(NbArg, static_cast<void**>((void*)&FirstArg));
#endif

#define QISHCPP_END_LOCALS_WITHOUT_ARGS                 \
 QishCpp_Locals()                                       \
    : QishCpp_StackRoots_Any(0,static_cast<void**>(0),  \
                        sizeof(QishCpp_Locals)) {};     \
} _locals_;

class QishCpp_BoxedHash;	/* defined in qicpp.cpp */
class QishCpp_ABox {
  QishCpp_ABox*_qishnext;
  QishCpp_ABox*_qishprev;
  static QishCpp_ABox*qishlast_;
  friend class QishCpp_BoxedHash;
  static QishCpp_BoxedHash*qishboxhash_;
  static unsigned long qishnbbox_;
 protected:
  void* _qishptrtab[];
 private:
  virtual void qishscan_minor()=0;
  virtual void qishscan_full()=0;
  virtual int qishscan_nbptr() const=0;
  friend class QishCpp_StackRoots_Any;
  void qishremove();
 public:
  void qishtouch();		/* to be called when any pointer is updated */
 protected:
  QishCpp_ABox() {
    qishnbbox_++;
    if (qishlast_) {
      qishlast_->_qishnext = this;
      this->_qishprev = qishlast_;
    };
    qishlast_ = this;
  };
  virtual ~QishCpp_ABox() {
    qishremove();
    qishnbbox_--;
    if (_qishnext) _qishnext->_qishprev = _qishprev;
    if (_qishprev) _qishprev->_qishnext = _qishnext;
    if (this==qishlast_) qishlast_ = _qishprev;
    _qishprev = _qishnext = 0;
  };
};				/* end of class QishCpp_ABox */


/// utility class to box pointers inside C++ classes; be careful to
/// call qishtouch() when any pointer is changed

template<class PtrStruct> class QishCpp_Box
: public QishCpp_ABox, public PtrStruct {
  virtual void qishscan_minor() {
    for (unsigned i=0; i<sizeof(PtrStruct)/sizeof(void*); i++)
      QISHGC_MINOR_UPDATE(_qishptrtab[i]);
  };
  virtual void qishscan_full() {
    for (unsigned i=0; i<sizeof(PtrStruct)/sizeof(void*); i++)
      QISHGC_FULL_UPDATE(_qishptrtab[i]);
  };
  virtual int qishscan_nbptr() const 
  { return sizeof(PtrStruct)/sizeof(void*); }
 public:
  QishCpp_Box() : QishCpp_ABox(), PtrStruct() {
    memset(_qishptrtab, 0, sizeof(PtrStruct));
    qishtouch();
  };
  virtual ~QishCpp_Box() {
    memset(_qishptrtab, 0, sizeof(PtrStruct));
  };
};				/* end of template QishCpp_Box */

#endif /* c++ */

#endif /*QISH INCLUDED eof $Id: qish.h 35 2005-08-17 09:44:19Z basile $ */
