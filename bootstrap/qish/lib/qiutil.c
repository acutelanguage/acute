/* file Qish/lib/qiutil.c */
// prcsid $Id: qiutil.c 26 2005-07-17 09:57:40Z basile $
// prcsproj $ProjectHeader: Qish 1.9 Tue, 28 Dec 2004 12:08:57 +0100 basile $

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
#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include <limits.h>
#include <dlfcn.h>
//#include <printf.h>
#include <syslog.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/poll.h>
#include <execinfo.h>

#include "qish.h"

#ifndef STRICT_C99
/* we need to define the symbol qish_nil as having the adress 0 for
   use in BEGIN_SIMPLE_FRAME... macros  */
asm (".globl qish_nil\n qish_nil = 0\n");
#endif

#define EMPTYSLOT  (void*)-1

struct paramhtab_st {
  unsigned tablen;
  int nbparam;
  struct {
    char *key;			/* strdup-ed key */
    char *val;			/* strdup-ed value */
  } tab[FLEXIBLE_ARRAY_DIM];
} *qish_paramhtab;




// add a physical entry to the paramtable
static void
addparam (char *mkey, char *mval)
{
  unsigned h = 0;
  int i = 0;
  int pos = -1;
  int incr = 0;
  int ln = 0;
  h = qish_strhash (mkey, -1) % qish_paramhtab->tablen;
  ln = qish_paramhtab->tablen;
  for (i = h; i < ln; i++) {
    char *curkey = qish_paramhtab->tab[i].key;
    if (!curkey) {
      if (pos < 0)
	pos = i;
      incr = 1;
      break;
    } else if (curkey == EMPTYSLOT) {
      if (pos < 0)
	pos = i;
      incr = 1;
    } else if (curkey[0] == mkey[0] && !strcmp (curkey, mkey)) {
      if (qish_paramhtab->tab[i].val) {
	free (qish_paramhtab->tab[i].val);
	qish_paramhtab->tab[i].val = 0;
      }
      pos = i;
      break;
    }
  }
  if (pos >= 0)
    goto add_at_pos;
  for (i = 0; i < (int) h; i++) {
    char *curkey = qish_paramhtab->tab[i].key;
    if (!curkey) {
      if (pos < 0)
	pos = i;
      incr = 1;
      break;
    } else if (curkey == EMPTYSLOT) {
      if (pos < 0)
	pos = i;
      incr = 1;
    } else if (curkey[0] == mkey[0] && !strcmp (curkey, mkey)) {
      if (qish_paramhtab->tab[i].val) {
	free (qish_paramhtab->tab[i].val);
	qish_paramhtab->tab[i].val = 0;
      }
      pos = i;
      break;
    }
  }
  assert (pos >= 0);		/* otherwise table was full */
add_at_pos:
  qish_paramhtab->tab[pos].key = mkey;
  qish_paramhtab->tab[pos].val = mval;
  qish_paramhtab->nbparam += incr;
}				// end of addparam

void
qish_put_parameter (const char *param, const char *val)
{
  char *lparam = 0;
  char *lval = 0;
  struct paramhtab_st *oldtab = 0;
  assert (param != 0 && param[0] != 0);
  assert (val != 0);
  lparam = strdup (param);
  if (!lparam)
    qish_epanic ("cannot duplicate param %s", param);
  lval = strdup (val);
  if (!lval)
    qish_epanic ("cannot duplicate param val %s", val);
  if (!qish_paramhtab
      || (int) (4 * qish_paramhtab->nbparam) >=
      (int) (3 * qish_paramhtab->tablen)) {
    int newsiz = 0;
    int i = 0;
    oldtab = qish_paramhtab;
    newsiz =
      qish_paramhtab ? qish_prime_after (9 * qish_paramhtab->nbparam / 8 +
					 50) : 31;
    qish_paramhtab = calloc (1, sizeof (*qish_paramhtab)
			     + newsiz * sizeof (qish_paramhtab->tab[0]));
    if (!qish_paramhtab)
      qish_epanic ("no memory for %d params", newsiz);
    qish_paramhtab->tablen = newsiz;
    qish_paramhtab->nbparam = 0;
    if (oldtab)
      for (i = 0; i < (int) oldtab->tablen; i++) {
	char *curkey = oldtab->tab[i].key;
	if (curkey && curkey != EMPTYSLOT)
	  addparam (curkey, oldtab->tab[i].val);
      }
  }
  addparam (lparam, lval);
}				// end of qish_put_parameter 


void
qish_remove_parameter (const char *param)
{
  unsigned h = 0;
  int i = 0;
  int ln = 0;
  if (!qish_paramhtab || qish_paramhtab->tablen <= 0)
    return;
  assert (param && param[0]);
  h = qish_strhash (param, -1) % qish_paramhtab->tablen;
  ln = qish_paramhtab->tablen;
  for (i = h; i < ln; i++) {
    char *curkey = qish_paramhtab->tab[i].key;
    if (!curkey)
      return;
    else if (curkey == EMPTYSLOT)
      continue;
    else if (curkey[0] == param[0] && !strcmp (curkey, param)) {
      if (qish_paramhtab->tab[i].val) {
	free (qish_paramhtab->tab[i].val);
	free (qish_paramhtab->tab[i].key);
	qish_paramhtab->tab[i].val = 0;
	qish_paramhtab->tab[i].key = EMPTYSLOT;
	qish_paramhtab->nbparam--;
	return;
      }
      break;
    }
  }
  for (i = 0; i < (int) h; i++) {
    char *curkey = qish_paramhtab->tab[i].key;
    if (!curkey)
      return;
    else if (curkey == EMPTYSLOT)
      continue;
    else if (curkey[0] == param[0] && !strcmp (curkey, param)) {
      if (qish_paramhtab->tab[i].val) {
	free (qish_paramhtab->tab[i].val);
	free (qish_paramhtab->tab[i].key);
	qish_paramhtab->tab[i].val = 0;
	qish_paramhtab->tab[i].key = EMPTYSLOT;
	qish_paramhtab->nbparam--;
	return;
      }
    }
  }
}				// end of qish_remove_parameter

char *
qish_parameter (const char *param)
{
  int i = 0;
  unsigned h = 0;
  int ln = 0;
  if (qish_paramhtab == 0)
    return 0;
  h = qish_strhash (param, -1) % qish_paramhtab->tablen;
  ln = qish_paramhtab->tablen;
  for (i = h; i < ln; i++) {
    char *curkey = qish_paramhtab->tab[i].key;
    if (!curkey)
      break;
    if (curkey == EMPTYSLOT)
      continue;
    if (curkey[0] == param[0] && !strcmp (curkey, param))
      return qish_paramhtab->tab[i].val;
  };
  for (i = 0; i < (int) h; i++) {
    char *curkey = qish_paramhtab->tab[i].key;
    if (!curkey)
      break;
    if (curkey == EMPTYSLOT)
      continue;
    if (curkey[0] == param[0] && !strcmp (curkey, param))
      return qish_paramhtab->tab[i].val;
  };
  return 0;
}				// end of qish_parameter

/// the dlhandle of the whole program
static void *wholeprog_dlh;


/*** syntax of configuration files 
 * any line starting with a # in first column is a comment
 * other lines are: symbol = value 
 * symbol are like C identifiers
 * values have no spaces 
 ***/


void
qish_parse_configfile (const char *configname)
{
  // it is not worth using GScanner
  int lincnt = 0;
  FILE *fil = 0;
  char *ps = 0;
  char *sym = 0;
  char *val = 0;
  char linbuf[512];
  fil = fopen (configname, "r");
  if (!fil)
    qish_epanic ("cannot open config file %s", configname);
  while (!feof (fil)) {
    sym = val = 0;
    memset (linbuf, 0, sizeof (linbuf));
    lincnt++;
    fgets (linbuf, sizeof (linbuf), fil);
    linbuf[sizeof (linbuf) - 1] = 0;
    if (linbuf[0] == '#')
      continue;			/* skip comments */
    for (ps = linbuf; *ps && isspace (*ps); ps++);
    if (*ps == 0 || *ps == '\n')
      continue;			/* skip blank lines */
    sym = ps;
    if (!isalpha (*sym))
      qish_panic ("bad symbol %s in config file %s line %d",
		  sym, configname, lincnt);
    for (; *ps && (isalnum (*ps) || *ps == '_' || *ps == '.'); ps++);
    for (; *ps && isspace (*ps); ps++)
      *ps = 0;
    if (*ps != '=')
      qish_panic
	("missing equal sign for symbol %s in config file %s line %d", sym,
	 configname, lincnt);
    *(ps++) = 0;
    for (; *ps && isspace (*ps); ps++)
      *ps = 0;
    val = ps;
    for (; *ps && isgraph (*ps); ps++);
    if (isspace (*ps))
      *ps = 0;
    if (!qish_parameter (sym))
      qish_put_parameter (sym, val);
  };
  fclose (fil);
}				// end of qish_parse_configfile



///////////////////////////////////////////////////////////////////////
/**************** module handling *************/

/****************
 * load a module, and if it was already loaded, unload the previous one 
 * by postponing its dlclose-ing
 ****************/

/*  array of dlhandles to dlclose */
#define NBPOSTPONEDLCLOSE (((QISH_MAX_MODULE/2)|0xff)+257)
static void *postponed_hdl[NBPOSTPONEDLCLOSE];

void
qish_postponed_dlclose (void)
{
  int i = 0;
  for (i = 0; i < NBPOSTPONEDLCLOSE; i++) {
    if (postponed_hdl[i]) {
      dlclose (postponed_hdl[i]);
      postponed_hdl[i] = 0;
    } else
      break;
  }
}

static void
postpone_dlclose (void *dlh)
{
  int i = 0;
  assert (dlh != 0);
  for (i = 0; i < NBPOSTPONEDLCLOSE; i++) {
    if (!postponed_hdl[i]) {
      postponed_hdl[i] = dlh;
      return;
    }
  };
  qish_panic ("postponed_hdl table full (%d elem)", NBPOSTPONEDLCLOSE);
}				/* end of postpone_dlclose */

/// return the module rank iff ok (ie loaded module successfully), or
/// else -1; the dirpath is like LD_LIBRARY_PATH, a colon separated
/// list of directories
int
qish_load_module (char *modname, char *dirpath, int modrank)
{
  char modpath[500];
  char dirname[400];
  void *dlh = 0;
  void *init = 0;
  memset (modpath, 0, sizeof (modpath));
  memset (dirname, 0, sizeof (dirname));
  if (modrank < 0) {
    // if a rank is not specified, find the original rank or a free one
    int i, freerk = -1;
    for (i = 0; i < QISH_MAX_MODULE; i++) {
      if (qish_moduletab[i].km_name[0]
	  && !strcmp (qish_moduletab[i].km_name, modname)) {
	modrank = i;
	break;
      } else if (freerk < 0 && !qish_moduletab[i].km_handle
		 && !qish_moduletab[i].km_name[0])
	freerk = i;
    };
    if (modrank < 0)
      modrank = freerk;
  };
  assert (modrank >= 0 && modrank < QISH_MAX_MODULE);
  if (dirpath) {
    char *colon = 0, *dir = dirpath;
    for (dir = dirpath; (colon = strchr (dir, ':')) != 0; dir = colon + 1) {
      memset (dirname, 0, sizeof (dirname));
      strncpy (dirname, dir,
	       ((colon - dir) >=
		(int) sizeof (dirname) - 1) ? ((int) sizeof (dirname) - 1)
	       : colon - dir - 1);
      if (!dirname[0]) {
	dirname[0] = '.';
	dirname[1] = (char) 0;
      };
      snprintf (modpath, sizeof (modpath) - 1, "%s/%s_qi.so", dirname,
		modname);
      if (!access (modpath, R_OK))
	goto found;
      snprintf (modpath, sizeof (modpath) - 1, "%s/%s_qj.so", dirname,
		modname);
      if (!access (modpath, R_OK))
	goto found;
      snprintf (modpath, sizeof (modpath) - 1, "%s/%s_qk.so", dirname,
		modname);
      if (!access (modpath, R_OK))
	goto found;
    };
    if (dir && dir[0]) {
      snprintf (modpath, sizeof (modpath) - 1, "%s/%s_qi.so", dir, modname);
      if (!access (modpath, R_OK))
	goto found;
      snprintf (modpath, sizeof (modpath) - 1, "%s/%s_qj.so", dir, modname);
      if (!access (modpath, R_OK))
	goto found;
      snprintf (modpath, sizeof (modpath) - 1, "%s/%s_qk.so", dir, modname);
      if (!access (modpath, R_OK))
	goto found;
    };
    if (!modpath[0]) {
      fprintf (stderr, "QISH cannot find module %s in dirpath %s\n",
	       modname, dirpath);
      return -1;
    };
  } else {			/* no dirpath, so use usual dlopen with LD_LIBRARY_PATH */
    snprintf (modpath, sizeof (modpath) - 1, "%s_qi.so", modname);
    if (!access (modpath, R_OK))
      goto found;
    snprintf (modpath, sizeof (modpath) - 1, "%s_qj.so", modname);
    if (!access (modpath, R_OK))
      goto found;
    snprintf (modpath, sizeof (modpath) - 1, "%s_qk.so", modname);
    if (!access (modpath, R_OK))
      goto found;
  };
found:
  dlh = dlopen (modpath, RTLD_GLOBAL | RTLD_NOW);
  if (!dlh) {
    fprintf (stderr,
	     "QISH cannot load module %s at %d (path %s, dirpath %s) - %s\n",
	     modname, modrank, modpath, dirpath ? : "*none*", dlerror ());
    return -1;
  };
  qish_dbgprintf ("module %s named %s is dlopened as %p", modpath, modname,
		  dlh);
  if (qish_moduletab[modrank].km_handle) {
    qish_dbgprintf ("postponing dlclose of %p",
		    qish_moduletab[modrank].km_handle);
    postpone_dlclose (qish_moduletab[modrank].km_handle);
  };
  strncpy (qish_moduletab[modrank].km_name, modname,
	   sizeof (qish_moduletab[modrank].km_name) - 1);
  snprintf (modpath, sizeof (modpath) - 1, "qishinit_%s", modname);
  init = dlsym (dlh, modpath);
  if (init)
    (*(qish_voidfun_t *) init) (modrank);
  qish_moduletab[modrank].km_handle = dlh;
  return modrank;
}				// end of qish_load_module



/// get a symbol in a module (or find it in all modules if modrank < 0)
void *
qish_get_symbol (char *name, int modrank)
{
  void *h = 0;
  int i = 0;
  assert (modrank < QISH_MAX_MODULE);
  if (modrank >= 0) {
    assert (qish_moduletab[modrank].km_name[0]);	//used module rank
    if (qish_moduletab[modrank].km_handle != 0)
      h = dlsym (qish_moduletab[modrank].km_handle, name);
    if (!wholeprog_dlh)
      wholeprog_dlh = dlopen ((char *) 0, RTLD_NOW | RTLD_GLOBAL);
    if (!h)
      h = dlsym (wholeprog_dlh, name);
    if (!h) {
      fprintf (stderr, "qish cannot find symbol %s in module #%d = %s - %s\n",
	       name, modrank, qish_moduletab[modrank].km_name, dlerror ());
      return 0;
    }
  } else {
    // scan module downwards (from younguest to oldest)
    for (i = QISH_MAX_MODULE - 1; i >= 0; i--)
      if (qish_moduletab[i].km_handle != 0) {
	h = dlsym (qish_moduletab[i].km_handle, name);
	if (h)
	  break;
      };
    if (!h)
      h = dlsym (wholeprog_dlh, name);
    if (!h) {
      fprintf (stderr, "qish cannot find symbol %s in any module - %s\n",
	       name, dlerror ());
      return 0;
    };
  }
  return h;
}				// end of qish_get_symbol


void
qish_unload_module (int modrank)
{
  char nampath[256];
  void *fini = 0;
  int i;
  if (modrank < 0 || modrank >= QISH_MAX_MODULE
      || !qish_moduletab[modrank].km_handle)
    return;
  qish_dbgprintf ("module #%d %s unloaded", modrank,
		  qish_moduletab[modrank].km_name);
  memset (nampath, 0, sizeof (nampath));
  for (i = 0; i < NBPOSTPONEDLCLOSE; i++)
    if (postponed_hdl[i] == 0) {
      postponed_hdl[i] = qish_moduletab[modrank].km_handle;
      break;
    };
  snprintf (nampath, sizeof (nampath) - 1, "qishfini_%s",
	    qish_moduletab[modrank].km_name);
  fini = dlsym (qish_moduletab[modrank].km_handle, nampath);
  if (fini)
    (*(qish_voidfun_t *) fini) (modrank);
  memset (qish_moduletab + modrank, 0, sizeof (struct qishmodule_st));
}				/* end of qish_unload_module */

//////////////////////////////////////////////////////////////////

// abort wrapper
void
qish_abort (void)
{
  abort ();
}

// usual panic function
void
qish_panic_at (int err, const char *fil, int lin, const char *fct,
	       const char *fmt, ...)
{
  static char logbuf[500];
#define MAXBACKTRACE 150
  static void *backarr[MAXBACKTRACE];
  int backdepth = 0;
  va_list ap;
  fflush (stderr);
  fflush (stdout);
  fprintf (stderr, "\n**QISH PANIC [%s:%d", fil, lin);
  if (fct && fct[0])
    fprintf (stderr, "@%s]", fct);
  else
    putc (']', stderr);
  putc (' ', stderr);
  fflush (stderr);
  va_start (ap, fmt);
  vfprintf (stderr, fmt, ap);
  vsnprintf (logbuf, sizeof (logbuf), fmt, ap);
  va_end (ap);
  logbuf[sizeof (logbuf) - 1] = (char) 0;
  if (err > 0)
    fprintf (stderr, ":%d - %s", err, strerror (err));
  putc ('\n', stderr);
  fflush (stderr);
  syslog (LOG_USER | LOG_CRIT, "[qish %s:%d%s%s] %s",
	  fil, lin, fct ? "@" : "", fct ? fct : "", logbuf);
  backdepth = backtrace (backarr, MAXBACKTRACE);
  backtrace_symbols_fd (backarr, backdepth, STDERR_FILENO);
  qish_abort ();
   /*NOTREACHED*/ exit (2);
}				// end of qish_panic_at

unsigned
qish_strhash (const char *str, int len)
{
  unsigned h = 0;
  int i = 0;
  const signed char *p = (const signed char *) str;
  if (len < 0) {
    for (; *p; i++, p++) {
      h = (h << 6) ^ (i - *p);
      h &= (unsigned) INT_MAX;
    }
  } else {
    for (; len > 0; i++, p++, len--) {
      h = (h << 6) ^ (i - *p);
      h &= (unsigned) INT_MAX;
    }
  }
  return h;
}				// end of qish_strhash 


// execvp and wait for termination
int
qish_sigexecvp (const char *file, char *const argv[])
{
  pid_t cpid = 0, wpid = 0;
  sigset_t allsigs;
  sigset_t oldsigs;
  int waitstat = 0;
  sigfillset (&allsigs);
  sigemptyset (&oldsigs);
  if (sigprocmask (SIG_SETMASK, &allsigs, &oldsigs))
    qish_panic ("cannot block all signals - %s", strerror (errno));
  cpid = fork ();
  if (cpid == 0) {
    /* child process */
    if (execvp (file, argv))
      qish_panic ("execvp %s failed - %s", file, strerror (errno));
    /* NOTREACHED */
    exit (1);
  } else if (cpid > 0) {
    /* father process */
    if (sigprocmask (SIG_SETMASK, &oldsigs, 0))
      qish_panic ("cannot unblock signals - %s", strerror (errno));
    for (;;) {
      waitstat = 0;
      wpid = waitpid (cpid, &waitstat, 0);
      if (wpid == cpid)
	break;
      if (wpid == -1 && errno == EINTR)
	continue;
      if (wpid == -1 && errno == ECHILD)
	break;
    };
    if (WIFSIGNALED (waitstat)) {
      fprintf (stderr, "qish subcommand %s terminated by signal %d = %s\n",
	       file, WTERMSIG (waitstat), sys_siglist[WTERMSIG (waitstat)]);
      return -1;
    } else
      return WEXITSTATUS (waitstat);
  } else
    /* fork failed */
    qish_epanic ("failed to fork for exec %s", file);
}				// end of qish_sigexecvp




// decode a character encoded like in XML with &lt; &gt; &apos; &quot;
// &nl; &cr; &bel; &ff; &tab; &esc; &nbsp; &#33; &#x1f;
// return -1 if not found
int
qish_decodexmlchar (unsigned char *buf, char **pend)
{
  if (*buf == '&') {
    if (!strncmp ((char*)buf + 1, "lt;", 3)) {
      *pend = (char*)buf + 4;
      return '<';
    } else if (!strncmp ((char*)buf + 1, "gt;", 3)) {
      *pend = (char*)buf + 4;
      return '>';
    } else if (!strncmp ((char*)buf + 1, "amp;", 4)) {
      *pend = (char*)buf + 5;
      return '&';
    } else if (!strncmp ((char*)buf + 1, "apos;", 5)) {
      *pend = (char*)buf + 6;
      return '\'';
    } else if (!strncmp ((char*)buf + 1, "asla;", 5)) {
      *pend = (char*)buf + 6;
      return '\\';
    } else if (!strncmp ((char*)buf + 1, "quot;", 5)) {
      *pend = (char*)buf + 6;
      return '\"';
    } else if (!strncmp ((char*)buf + 1, "nl;", 3)) {
      *pend = (char*)buf + 4;
      return '\n';
    } else if (!strncmp ((char*)buf + 1, "cr;", 3)) {
      *pend = (char*)buf + 4;
      return '\r';
    } else if (!strncmp ((char*)buf + 1, "bel;", 4)) {
      *pend = (char*)buf + 5;
      return '\a';
    } else if (!strncmp ((char*)buf + 1, "ff;", 3)) {
      *pend = (char*)buf + 4;
      return '\f';
    } else if (!strncmp ((char*)buf + 1, "bs;", 3)) {
      *pend = (char*)buf + 4;
      return '\b';
    } else if (!strncmp ((char*)buf + 1, "sp;", 3)) {
      *pend = (char*)buf + 4;
      return ' ';
    } else if (!strncmp ((char*)buf + 1, "tab;", 4)) {
      *pend = (char*)buf + 5;
      return '\t';
    } else if (!strncmp ((char*)buf + 1, "vtb;", 4)) {
      *pend = (char*)buf + 5;
      return '\v';
    } else if (!strncmp ((char*)buf + 1, "esc;", 4)) {
      *pend = (char*)buf + 5;
      return 27;
    } else if (!strncmp ((char*)buf + 1, "del;", 4)) {
      *pend = (char*)buf + 5;
      return 127;
    } else if (!strncmp ((char*)buf + 1, "nul;", 4)) {
      *pend = (char*)buf + 5;
      return 0;
    } else if (!strncmp ((char*)buf + 1, "nbsp;", 5)) {
      *pend = (char*)buf + 6;
      return 160;
    } else if (buf[1] == '#') {
      char *end;
      long i = 0;
      if (isdigit (buf[2])) {
	for (i = 2; i < 9; i++)
	  if (!isdigit (buf[i]))
	    goto shortenoughdec;
	fprintf (stderr, "bad charcode entity %.10s\n", (char*)buf);
	*pend = 0;
	return -1;
      shortenoughdec:
	i = strtol ((char*)buf + 2, &end, 10);
	if (*end == ';') {
	  *pend = end + 1;
	  return i;
	}
      } else if (buf[2] == 'x' || buf[2] == 'X') {
	for (i = 3; i < 9; i++)
	  if (!isxdigit (buf[i]))
	    goto shortenoughhex;
	fprintf (stderr, "bad charhexcode entity %.10s\n", buf);
	*pend = 0;
	return -1;
      shortenoughhex:
	i = strtol ((char*)buf + 3, &end, 16);
	if (*end == ';') {
	  *pend = end + 1;
	  return i;
	}
      }
    };
    fprintf (stderr, "bad char entity %.8s\n", (char*)buf);
    *pend = 0;
    return -1;
  } else if (isprint (*buf)) {
    *pend = (char*)buf + 1;
    return *buf;
  };
  *pend = 0;
  return -1;
}				// end of  qish_decodexmlchar

// encode an xmlchar - return the next position in buffer; consume at most 8 bytes there
char *
qish_encodexmlchar (int c, char *buf)
{
  if (c < 0 || c > 0xffff)
    return 0;
  switch (c) {
#define PUTENT(E) strcpy(buf, E); return buf+sizeof(E)-1
  case '<':
    PUTENT ("&lt;");
  case '>':
    PUTENT ("&gt;");
  case '&':
    PUTENT ("&amp;");
  case '\'':
    PUTENT ("&apos;");
  case '\\':
    PUTENT ("&asla;");
  case '\"':
    PUTENT ("&quot;");
  case '\n':
    PUTENT ("&nl;");
  case '\r':
    PUTENT ("&cr;");
  case '\a':
    PUTENT ("&bel;");
  case '\f':
    PUTENT ("&ff;");
  case '\b':
    PUTENT ("&bs;");
  case '\t':
    PUTENT ("&tab;");
  case '\v':
    PUTENT ("&vtb;");
  case 27:
    PUTENT ("&esc;");
  case 127:
    PUTENT ("&del;");
  case 0:
    PUTENT ("&nul;");
  case 160:
    PUTENT ("&nbsp;");
#undef PUTENT
  case ' ':
    *buf = ' ';
    return buf + 1;
  default:
    if (c < 0xff && isgraph (c)) {
      *buf = c;
      return buf + 1;
    } else {
      sprintf (buf, "&#%d;", c);
      return buf + strlen (buf);
    };
  }
  return 0;
}				// end of qish_encodexmlchar

int
qish_getxmlc (FILE * f, int strict)
{
  int c = -1;
  char buf[16];
  if (!f)
    return EOF;
  c = getc (f);
  if (c == '&') {
    int i;
    char *end;
    memset (buf, 0, sizeof (buf));
    buf[0] = '&';
    for (i = 1; i < (int) sizeof (buf) - 2; i++) {
      c = getc (f);
      if (c < 0)
	return -1;
      buf[i] = c;
      if (c == ';')
	break;
    };
    c = qish_decodexmlchar ((unsigned char*)buf, &end);
  } else if (strict) {
    if (c == '\'' || c == '\"' || c == '<' || c == '>') {
      ungetc (c, f);
      return -2;
    }
  }
  return c;
}				// end of qish_getxmlc


// array of primes gotten by shell command 
/// primes 10 2000000000 | awk '($1<256 || $1>p+p/16){print $1, ","; p=$1}' 
// and then removed some primes >20 <255

static const int primtab[] = {
  // if changed here also change primememotab below
  3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 47, 59, 67,
  73, 83, 97, 107, 113, 127, 137, 149, 163, 179, 191,
  199, 211, 223, 239, 251, 269, 293, 313, 337, 359, 383, 409,
  439, 467, 499, 541, 577, 617, 659, 701, 751, 809, 863, 919, 977,
  1039, 1109, 1181, 1259, 1361, 1447, 1543, 1657, 1777, 1889, 2011,
  2137, 2273, 2417, 2579, 2741, 2917, 3109, 3307, 3517, 3739, 3989,
  4241, 4507, 4789, 5099, 5419, 5779, 6143, 6529, 6947, 7393, 7867,
  8363, 8887, 9461, 10061, 10691, 11369, 12097, 12889, 13697, 14557,
  15467, 16447, 17477, 18583, 19751, 21001, 22343, 23741, 25229,
  26813, 28493, 30293, 32189, 34211, 36353, 38629, 41047, 43613,
  46349, 49253, 52361, 55639, 59119, 62819, 66749, 70921, 75367,
  80107, 85121, 90469, 96137, 102149, 108541, 115327, 122557, 130223,
  138371, 147029, 156227, 166013, 176389, 187417, 199151, 211619,
  224863, 238919, 253853, 269719, 286589, 304501, 323537, 343769,
  365257, 388099, 412387, 438169, 465581, 494687, 525607, 558469,
  593381, 630473, 669887, 711757, 756247, 803513, 853733, 907099,
  963793, 1024031, 1088039, 1156073, 1228333, 1305121, 1386703,
  1473379, 1565471, 1663327, 1767307, 1877773, 1995139, 2119837,
  2252353, 2393137, 2542733, 2701661, 2870519, 3049927, 3240551,
  3443087, 3658283, 3886937, 4129871, 4387991, 4662241, 4953667,
  5263309, 5592281, 5941841, 6313231, 6707819, 7127077, 7572529,
  8045819, 8548693, 9082993, 9650681, 10253851, 10894717, 11575639,
  12299129, 13067827, 13884581, 14752369, 15674401, 16654063,
  17694949, 18800897, 19975981, 21224503, 22551059, 23960509,
  25458047, 27049177, 28739797, 30536039, 32444543, 34472353,
  36626881, 38916071, 41348347, 43932619, 46678409, 49595831,
  52695571, 55989071, 59488409, 63206441, 67156853, 71354183,
  75813823, 80552189, 85586707, 90935891, 96619399, 102658133,
  109074271, 115891417, 123134639, 130830559, 139007497, 147695491,
  156926519, 166734433, 177155353, 188227579, 199991833, 212491361,
  225772073, 239882861, 254875541, 270805267, 287730643, 305713813,
  324820933, 345122249, 366692393, 389610671, 413961347, 439833959,
  467323609, 496531369, 527564581, 560537387, 595571023, 632794243,
  672343901, 714365453, 759013303, 806451641, 856854881, 910408357,
  967308887, 1027765699, 1092001067, 1160251153, 1232766853,
  1309814801, 1391678243, 1478658173, 1571074313, 1669266461,
  1773595633, 1884445399,

  0
};

#if __GNUC__ && !STRICT_C99
/* memoisation  of above for small values */
static const short primememotab[] = {
  /// if changed here also change primtab above
  // parenthesis are required to make indent happy
  [(0)...(2)] 3,[(3)...(4)] 5,[(5)...(6)] 7,
  [(7)...(10)] 11,[(11)...(12)] 13,[(13)...(16)] 17,
  [(17)...(18)] 19,[(19)...(22)] 23,[(23)...(28)] 29,
  [(29)...(30)] 31,[(31)...(36)] 37,[(37)...(40)] 41,
  [(41)...(46)] 47,[(47)...(58)] 59,[(59)...(66)] 67,
  [(67)...(72)] 73,[(73)...(82)] 83,[(83)...(96)] 97,
  [(97)...(106)] 107,[(107)...(112)] 113,[(113)...(126)] 127,
  [(127)...(136)] 137,[(137)...(148)] 149,[(149)...(162)] 163,
  [(163)...(178)] 179,[(179)...(190)] 191,[(191)...(198)] 199,
  [(199)...(210)] 211,[(211)...(222)] 223,[(223)...(238)] 239,
  [(239)...(250)] 251,[(251)...(268)] 269,[(269)...(292)] 293
};
#endif /* __GNUC__ && !STRICT_C99 */

int
qish_prime_after (int x)
{
  int tablen, lo, hi;

#if __GNUC__ && !STRICT_C99
  if (x > 0 && x < (int) (sizeof (primememotab) / sizeof (primememotab[0])))
    return (int) (primememotab[x]);
#endif /* __GNUC__ && !STRICT_C99 */
  tablen = (sizeof (primtab) / sizeof (int)) - 1;
  lo = 0;
  hi = tablen - 1;
  while (lo + 2 < hi) {
    int mid = (lo + hi) / 2;
    int midval = primtab[mid];
    if (x > midval)
      lo = mid;
    else
      hi = mid;
  };
  for (;;) {
    int loval = primtab[lo];
    if (loval > x)
      return loval;
    else if (loval <= 0) {
      fprintf (stderr, "qish got no prime after %d\n", x);
      return 0;
    };
    lo++;
  }
}

/* eof $Id: qiutil.c 26 2005-07-17 09:57:40Z basile $ */
