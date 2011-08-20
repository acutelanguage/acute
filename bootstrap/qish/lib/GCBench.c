// This is adapted from a benchmark written by John Ellis and Pete Kovac
// of Post Communications.
// It was modified by Hans Boehm of Silicon Graphics.
// Translated to C++ 30 May 1997 by William D Clinger of Northeastern Univ.
// Translated to C 15 March 2000 by Hans Boehm, now at HP Labs.
// Adapted to Qish, 20 April 2002 by Basile Starynkevitch
// to use it with H.Boehm's GC, compile with -DGC and link with libgc.a
// to us it with Qish, compile with -DQISH and link with libqish.a and -ldl
// to use it without a GC but with malloc&free compile it without any -Dflags
//
//      This is no substitute for real applications.  No actual application
//      is likely to behave in exactly this way.  However, this benchmark was
//      designed to be more representative of real applications than other
//      Java GC benchmarks of which we are aware.
//      It attempts to model those properties of allocation requests that
//      are important to current GC techniques.
//      It is designed to be used either to obtain a single overall performance
//      number, or to give a more detailed estimate of how collector
//      performance varies with object lifetimes.  It prints the time
//      required to allocate and collect balanced binary trees of various
//      sizes.  Smaller trees result in shorter object lifetimes.  Each cycle
//      allocates roughly the same amount of memory.
//      Two data structures are kept around during the entire process, so
//      that the measured performance is representative of applications
//      that maintain some live in-memory data.  One of these is a tree
//      containing many pointers.  The other is a large array containing
//      double precision floating point numbers.  Both should be of comparable
//      size.
//
//      The results are only really meaningful together with a specification
//      of how much memory was used.  It is possible to trade memory for
//      better time performance.  This benchmark should be run in a 32 MB
//      heap, though we don't currently know how to enforce that uniformly.
//
//      Unlike the original Ellis and Kovac benchmark, we do not attempt
//      measure pause times.  This facility should eventually be added back
//      in.  There are several reasons for omitting it for now.  The original
//      implementation depended on assumptions about the thread scheduler
//      that don't hold uniformly.  The results really measure both the
//      scheduler and GC.  Pause time measurements tend to not fit well with
//      current benchmark suites.  As far as we know, none of the current
//      commercial Java implementations seriously attempt to minimize GC pause
//      times.

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/times.h>

#ifdef GC
#include "gc.h"
#endif

#ifdef QISH
#ifdef GC
#error do not define both QISH and GC
#endif
#include "qish.h"
#define QISHVOLATILE volatile
#else
#define QISHVOLATILE		/*normal */
#endif

#ifdef PROFIL
extern void init_profiling ();
extern dump_profile ();
#endif

//  These macros were a quick hack for the Macintosh.
//
//  #define currentTime() clock()
//  #define elapsedTime(x) ((1000*(x))/CLOCKS_PER_SEC)

#define currentTime() stats_rtclock()
#define elapsedTime(x) (x)

/* Get the current time in milliseconds */

unsigned
stats_rtclock (void)
{
  struct timeval t;
  struct timezone tz;

  if (gettimeofday (&t, &tz) == -1)
    return 0;
  return (t.tv_sec * 1000 + t.tv_usec / 1000);
}

static const int kStretchTreeDepth = 20;	// about 64Mb
static const int kLongLivedTreeDepth = 18;	// about 16Mb
static const int kArraySize = 2000000;	// about 16Mb
static const int kMinTreeDepth = 4;
static const int kMaxTreeDepth = 20;

typedef struct Node0_struct {
  int i, j;			// Qish requires the node to start with a never zero word
  struct Node0_struct *left;
  struct Node0_struct *right;
} Node0;

#ifdef HOLES
#   define HOLE() GC_NEW(Node0);
#else
#   define HOLE()
#endif

typedef Node0 *Node;

void
init_Node (Node QISHVOLATILE me, Node QISHVOLATILE l, Node QISHVOLATILE r)
{
  static int count;
#ifdef QISH
  BEGIN_SIMPLE_FRAME (3, me, 0, qish_nil);
#endif
  count++;
  me->i = count;		/// qish requires the first word to be non zero
  me->left = l;
  me->right = r;
#ifdef QISH
  qish_write_notify (me);
  EXIT_FRAME ();
#endif
}

#if !defined(GC) && !defined(QISH)
void
destroy_Node (Node me)
{
  if (me->left) {
    destroy_Node (me->left);
  }
  if (me->right) {
    destroy_Node (me->right);
  }
  free (me);
}
#endif

// Nodes used by a tree of a given size
static int
TreeSize (int i)
{
  return ((1 << (i + 1)) - 1);
}

// Number of iterations to use for a given tree depth
static int
NumIters (int i)
{
  return 2 * TreeSize (kStretchTreeDepth) / TreeSize (i);
}

// Build tree top down, assigning to older objects.
static void
Populate (int iDepth, Node QISHVOLATILE thisNode)
{
#ifdef QISH
  Node QISHVOLATILE newNode = 0;
  BEGIN_SIMPLE_FRAME (1, thisNode, 1, newNode);
#endif
  if (iDepth <= 0) {
#ifdef QISH
    goto end;
#else
    return;
#endif
  } else {
    iDepth--;
#if defined(GC)
    thisNode->left = GC_NEW (Node0);
    HOLE ();
    thisNode->right = GC_NEW (Node0);
    HOLE ();
#elif defined(QISH)
    // left
    newNode = qish_allocate (sizeof (Node0));
    newNode->i = -1;		// the first word have to be non zero for Qish
    thisNode->left = newNode;
    qish_write_notify (thisNode);
#ifdef HOLES
    // allocate a useless node
    newNode = qish_allocate (sizeof (Node0));
    newNode->i = -5;
#endif
    // right
    newNode = qish_allocate (sizeof (Node0));
    newNode->i = -1;		// the first word have to be non zero for Qish
    thisNode->right = newNode;
    qish_write_notify (thisNode);
#ifdef HOLES
    // allocate a useless node
    newNode = qish_allocate (sizeof (Node0));
    newNode->i = -5;
#endif
#else
    thisNode->left = calloc (1, sizeof (Node0));
    thisNode->right = calloc (1, sizeof (Node0));
#endif
    Populate (iDepth, thisNode->left);
    Populate (iDepth, thisNode->right);
  }
#ifdef QISH
end:
  EXIT_FRAME ();
#endif
}

// Build tree bottom-up
static Node
MakeTree (int iDepth)
{
#ifndef QISH
  Node left, right;
  Node result;
#else
  struct {
    Node QISHVOLATILE _left, _right, _result;
  } _locals_ = {
  0, 0, 0};
#define result _locals_._result
#define left _locals_._left
#define right _locals_._right
  BEGIN_LOCAL_FRAME (0, qish_nil);
#endif
  if (iDepth <= 0) {
#if !defined(GC) && !defined(QISH)
    result = calloc (1, sizeof (Node0));
    /* result is implicitly initialized  */
    return result;
#elif defined(GC)
    result = GC_NEW (Node0);
    HOLE ();
    /* result is implicitly initialized  */
    return result;
#else /*QISH*/
      result = qish_allocate (sizeof (Node0));
    result->i = -1;
    goto end;
#endif
  } else {
    /* originally left and right where declared here - Basile declares
       them at start of function */
    left = MakeTree (iDepth - 1);
    right = MakeTree (iDepth - 1);
#if !defined(GC) && !defined(QISH)
    result = malloc (sizeof (Node0));
#elif defined(GC)
    result = GC_NEW (Node0);
    HOLE ();
#else /*QISH*/
      result = qish_allocate (sizeof (Node0));
    result->i = -2;
#endif
    init_Node (result, left, right);
#ifdef QISH
    goto end;
#else
    return result;
#endif
  }
#ifdef QISH
end:
  EXIT_FRAME ();
  return result;
#undef left
#undef right
#undef result
#endif
}

static void
PrintDiagnostics ()
{
#if 0
  long lFreeMemory = Runtime.getRuntime ().freeMemory ();
  long lTotalMemory = Runtime.getRuntime ().totalMemory ();

  System.out.print (" Total memory available=" + lTotalMemory + " bytes");
  System.out.println ("  Free memory=" + lFreeMemory + " bytes");
#endif
}

static void
TimeConstruction (int depth)
{
  long tStart, tFinish;
  int iNumIters = NumIters (depth);
  Node QISHVOLATILE tempTree = 0;
  int i;
#ifdef QISH
  BEGIN_SIMPLE_FRAME (0, qish_nil, 1, tempTree);
#endif
  printf ("Creating %d trees of depth %d\n", iNumIters, depth);

  tStart = currentTime ();
  for (i = 0; i < iNumIters; ++i) {
#if !defined(GC) && !defined(QISH)
    tempTree = calloc (1, sizeof (Node0));
#elif defined(GC)
    tempTree = GC_NEW (Node0);
#else /*QISH*/
      tempTree = qish_allocate (sizeof (Node0));
    tempTree->i = -3;
#endif
    Populate (depth, tempTree);
#if !defined(GC) && !defined(QISH)
    destroy_Node (tempTree);
#endif
    tempTree = 0;
  }
  tFinish = currentTime ();
  printf ("\tTop down construction took %d msec\n",
	  elapsedTime (tFinish - tStart));

  tStart = currentTime ();
  for (i = 0; i < iNumIters; ++i) {
    tempTree = MakeTree (depth);
#if !defined(GC) && !defined(QISH)
    destroy_Node (tempTree);
#endif
    tempTree = 0;
  }
  tFinish = currentTime ();
  printf ("\tBottom up construction took %d msec\n",
	  elapsedTime (tFinish - tStart));
#ifdef QISH
  EXIT_FRAME ();
#endif

}


#ifndef QISH
int
main ()
#else
int
qishmain ()
#endif
{
#ifndef QISH
  Node root;
  Node longLivedTree;
  Node tempTree;
#else
  struct {
    Node QISHVOLATILE _root, _longLivedTree, _tempTree;
  } _locals_ = {
  0, 0, 0};
#define root _locals_._root
#define longLivedTree _locals_._longLivedTree
#define tempTree _locals_._tempTree
#endif /*QISH*/
  long tStart, tFinish;
  long tElapsed;
  int i, d;
  double *array;
  struct tms mytms;
  long tick = 0;
#ifdef GC
  // GC_full_freq = 30;
  // GC_free_space_divisor = 16;
  // GC_enable_incremental();
#endif

#ifdef QISH
  BEGIN_LOCAL_FRAME (0, qish_nil);
#endif
  memset (&mytms, 0, sizeof (mytms));
  printf ("\nGarbage Collector Test:"
#ifdef QISH
	  " QISH"
#endif
#ifdef GC
	  " BoehmGC"
#endif
#ifdef HOLES
	  " Holes"
#endif
	  "\n");
  printf (" Live storage will peak at %d bytes.\n\n",
	  2 * sizeof (Node0) * TreeSize (kLongLivedTreeDepth) +
	  sizeof (double) * kArraySize);
  printf (" Stretching memory with a binary tree of depth %d\n",
	  kStretchTreeDepth);
  PrintDiagnostics ();
#ifdef PROFIL
  init_profiling ();
#endif

  tStart = currentTime ();

  // Stretch the memory space quickly
  tempTree = MakeTree (kStretchTreeDepth);
#if !defined(GC) && !defined(QISH)
  destroy_Node (tempTree);
#endif
  tempTree = 0;

  // Create a long lived object
  printf (" Creating a long-lived binary tree of depth %d\n",
	  kLongLivedTreeDepth);
#if !defined(GC) && !defined(QISH)
  longLivedTree = calloc (1, sizeof (Node0));
#elif defined(GC)
  longLivedTree = GC_NEW (Node0);
#else /*QISH*/
    longLivedTree = qish_allocate (sizeof (Node0));
  longLivedTree->i = -4;
#endif
  Populate (kLongLivedTreeDepth, longLivedTree);

  // Create long-lived array, filling half of it
  printf (" Creating a long-lived array of %d doubles\n", kArraySize);
#ifndef GC
  // this array is irrelavant to Qish
  array = malloc (kArraySize * sizeof (double));
#else
#ifndef NO_PTRFREE
  array = GC_MALLOC_ATOMIC (sizeof (double) * kArraySize);
#else
  array = GC_MALLOC (sizeof (double) * kArraySize);
#endif
#endif
  for (i = 0; i < kArraySize / 2; ++i) {
    array[i] = 1.0 / i;
  }
  PrintDiagnostics ();

  for (d = kMinTreeDepth; d <= kMaxTreeDepth; d += 2) {
    TimeConstruction (d);
  }

  if (longLivedTree == 0 || array[1000] != 1.0 / 1000)
    fprintf (stderr, "Failed\n");
  // fake reference to LongLivedTree
  // and array
  // to keep them from being optimized away

  tFinish = currentTime ();
  tElapsed = elapsedTime (tFinish - tStart);
  PrintDiagnostics ();
  times (&mytms);
  tick = sysconf (_SC_CLK_TCK);
  printf ("Completed in %d msec\n", tElapsed);
  printf (" CPU %.3f user + %.3f system = %.3f total time (sec)\n",
	  (double) mytms.tms_utime / (double) tick,
	  (double) mytms.tms_stime / (double) tick,
	  ((double) mytms.tms_stime +
	   (double) mytms.tms_utime) / (double) tick);
#ifdef GC
  printf ("Completed %d Boehm collections\n", GC_gc_no);
  printf ("Heap size is %d\n", GC_get_heap_size ());
#endif
#ifdef QISH
  printf ("Qish done %d minor and %d full garbage collections\n"
	  "   (min.birth.size=%dK, max.size=%dK, gc.thresh=%dK)\n",
	  qish_nb_minor_collections, qish_nb_full_collections,
	  qish_min_birth_size >> 10, qish_max_birth_size >> 10,
	  qish_full_gc_threshold >> 10);
#endif
#ifdef PROFIL
  dump_profile ();
#endif
#ifdef QISH
  EXIT_FRAME ();
#endif
  return 0;
}

#ifdef QISH
/// the rest of the file is for QISH; it defines the required utility routines
/// and encapsulate the previous main

/// in this test we only have one single (moved object data) type, the Node type

static void *
gc_copy_forqish (void **padr, void *dst, const void *src)
{
  ((Node) dst)->i = ((Node) src)->i;
  ((Node) dst)->j = ((Node) src)->j;
  ((Node) dst)->left = ((Node) src)->left;
  ((Node) dst)->right = ((Node) src)->right;
  *padr = dst;
  return ((Node) dst) + 1;
}

static void *
minor_scan_forqish (void *pt)
{
  QISHGC_MINOR_PTR_UPDATE (((Node) pt)->left);
  QISHGC_MINOR_PTR_UPDATE (((Node) pt)->right);
  return ((Node) pt) + 1;
}

static void *
full_scan_forqish (void *pt)
{
  QISHGC_FULL_PTR_UPDATE (((Node) pt)->left);
  QISHGC_FULL_PTR_UPDATE (((Node) pt)->right);
  return ((Node) pt) + 1;
}

static void
fixed_scan_forqish (void *ptr, int size)
{
  qish_panic ("fixed scan should never be called ptr=%p size=%d", ptr, size);
}

int
main ()
{
  qishgc_init ();
  qish_gc_copy_p = gc_copy_forqish;
  qish_minor_scan_p = minor_scan_forqish;
  qish_full_scan_p = full_scan_forqish;
  qish_fixedfull_scan_p = fixed_scan_forqish;
  qish_fixedminor_scan_p = fixed_scan_forqish;
  qishmain ();
  return 0;
}
#endif /*QISH*/
