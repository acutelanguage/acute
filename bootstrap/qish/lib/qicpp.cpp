// file Qish/lib/qicpp.cpp
// $Id: qicpp.cpp 35 2005-08-17 09:44:19Z basile $

//  Copyright © 2005 Basile STARYNKEVITCH

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


#include <cstdio>
#include <cstdlib>
#include <cerrno>
#include <csetjmp>
#include <climits>
#include <sys/mman.h>
#include <cstring>
#include <cassert>


#include "qish.h"

QishCpp_StackRoots_Any* QishCpp_StackRoots_Any::qishr_top_;
QishCpp_StackRoots_Any::Qishr_installer_cl QishCpp_StackRoots_Any::_qishr_init_;

QishCpp_StackRoots_Any::Qishr_installer_cl::Qishr_installer_cl() {
  qishcpp_minor_p = qishr_minor;
  qishcpp_full_p = qishr_full;
};

#define QISHCPP_EMPTY_SLOT (void*)(((void**)0)+2)

class QishCpp_BoxedHash {
  friend class QishCpp_StackRoots_Any;
  int _hsiz;			// prime size of hashtable
  int _hcnt;			// count
  QishCpp_ABox* _htab[];
  void raw_add(const QishCpp_ABox*bx) {
    assert(_hcnt+2<_hsiz);
    assert(_hsiz>2);
    assert(bx!=0);
    int pos= -1;
    qish_word_t h = ((qish_word_t)bx)  % _hsiz;
    for (int i=(int)h; i<_hcnt; i++) {
      QishCpp_ABox* cur=_htab[i];
      if (cur==bx) return;
      else if (cur==QISHCPP_EMPTY_SLOT) {
	if (pos<0) pos=i;
      }
      else if (!cur) {
	if (pos<0) pos=i;
	break;
      };
    };
    if (pos>=0 && _htab[pos]==0) {
      _htab[pos] = const_cast<QishCpp_ABox*>(bx);
      _hcnt++;
      return;
    }
    for (int i=(int)0; i<h; i++) {
      QishCpp_ABox* cur=_htab[i];
      if (cur==bx) return;
      else if (cur==QISHCPP_EMPTY_SLOT) {
	if (pos<0) pos=i;
      }
      else if (!cur) {
	if (pos<0) pos=i;
	break;
      };
    };
    assert(pos>=0);
    _htab[pos] = const_cast<QishCpp_ABox*>(bx);
    _hcnt++;
  };				// end of raw_add
  void*operator new(size_t sz, int siz) { void *p = calloc(sz + siz * sizeof(QishCpp_ABox*), 1);
  if (!p) qish_panic("cannot allocate QishCpp_ABoxHash of %d entries", siz);
  return p;
  };
  void operator delete(void*p) { free(p); };
  QishCpp_BoxedHash(int sz) : _hsiz(sz), _hcnt(0) {
    assert(sz>2);
    memset(_htab, 0, sizeof(_htab[0])*sz);
  };
  ~QishCpp_BoxedHash() {
    memset(_htab, 0, sizeof(_htab[0])*_hsiz);
    _hsiz=0; _hcnt=0;
  };
public:
  static QishCpp_BoxedHash* make(int sz=0) {
    if (sz<80) sz=80;
    sz = qish_prime_after(sz);
    return new(sz) QishCpp_BoxedHash(sz);
  };
  int index(const QishCpp_ABox*bx) const {
    assert(_hsiz>2);
    assert(bx!=0);
    qish_word_t h = ((qish_word_t)bx)  % _hsiz;
    for (int i=(int)h; i<_hcnt; i++) {
      QishCpp_ABox* cur = _htab[i];
      if (cur==bx) return i;
      else if (cur==QISHCPP_EMPTY_SLOT) continue;
      else if (!cur) return  -1;
    };
    for (int i=(int)0; i<h; i++) {
      QishCpp_ABox* cur = _htab[i];
      if (cur==bx) return i;
      else if (cur==QISHCPP_EMPTY_SLOT) continue;
      else if (!cur) return  -1;
    };
    return -1;
  };
  bool has(const QishCpp_ABox*bx) const {
    if (!bx) return false;
    return index(bx)>=0;
  };
  static QishCpp_BoxedHash* add(const QishCpp_ABox*ptr, QishCpp_BoxedHash*ht=0, int incr=0) {
    if (incr<100) incr=100;
    if (!ht || 4*ht->_hsiz > 5*ht->_hcnt) {
      int newsiz = ((ht?((5*ht->_hcnt)/4):100)+incr);
      QishCpp_BoxedHash* newh = QishCpp_BoxedHash::make(newsiz);
      if (ht) {
	for (int i=ht->_hsiz-1; i>=0; i--) {
	  QishCpp_ABox* cur=ht->_htab[i];
	  if (!cur || cur==QISHCPP_EMPTY_SLOT) continue;
	  newh->raw_add(cur);
	};
	delete (ht);
      };
      newh->raw_add(ptr);
      return newh;
    }
    else {
      ht->raw_add(ptr);
      return ht;
    };
  };
  static QishCpp_BoxedHash* remove(const QishCpp_ABox*ptr, QishCpp_BoxedHash* ht=0, int decr=0) {
    if (decr<100) decr=100;
    if (!ht) return 0;
    if (!ptr) return ht;
    int ix = ht->index(ptr);
    if (ix<0) return ht;
    ht->_htab[ix] = static_cast<QishCpp_ABox*>(QISHCPP_EMPTY_SLOT);
    ht->_hcnt--;
    if (ht->_hsiz>200 && ht->_hcnt+decr < ht->_hsiz/2) {
      int newsiz = ((5*ht->_hcnt)/4 + decr);
      if (newsiz<ht->_hsiz) {
	QishCpp_BoxedHash* newh = QishCpp_BoxedHash::make(newsiz);
	for (int i=ht->_hsiz-1; i>=0; i--) {
	  QishCpp_ABox* cur=ht->_htab[i];
	  if (!cur || cur==QISHCPP_EMPTY_SLOT) continue;
	  newh->raw_add(cur);
	};
	delete ht;
	return newh;
      };
    };
    return ht;
  };
};				// end class QishCpp_BoxedHash

unsigned long QishCpp_ABox::qishnbbox_=0;
QishCpp_BoxedHash* QishCpp_ABox::qishboxhash_=0;
QishCpp_ABox* QishCpp_ABox::qishlast_=0;

void QishCpp_StackRoots_Any::qishr_minor() {
  qish_dbgprintf("begin cpp qishr_minor");
  for (QishCpp_StackRoots_Any*r = QishCpp_StackRoots_Any::qishr_top_; 
       r; r= r->_qishr_prev) {
    qish_dbgprintf("minorscan cpp roots %p nbroots %d nbextra %d", 
		   r, r->_qishr_nbroots, r->_qishr_nbextra);
    r->qishstack_minor_scan();
  };
  QishCpp_BoxedHash* ht = QishCpp_ABox::qishboxhash_;
  if (ht) {
    for (int i=ht->_hsiz-1; i>=0; i--) {
      QishCpp_ABox*bx = ht->_htab[i];
      if (bx && bx !=QISHCPP_EMPTY_SLOT) {
	qish_dbgprintf("minorscan touched cpp box %p",  
		       bx);
	bx->qishscan_minor();
      };
    };
    delete (QishCpp_ABox::qishboxhash_);
  };
  QishCpp_ABox::qishboxhash_ =
    QishCpp_BoxedHash::make(33+QishCpp_ABox::qishnbbox_/16);
};

void QishCpp_ABox::qishtouch() {
  QishCpp_ABox::qishboxhash_
    = QishCpp_BoxedHash::add(this, QishCpp_ABox::qishboxhash_, 200);
}

void QishCpp_ABox::qishremove() {
  QishCpp_ABox::qishboxhash_
    = QishCpp_BoxedHash::remove(this, QishCpp_ABox::qishboxhash_, 120);
}

void QishCpp_StackRoots_Any::qishr_full() {
  qish_dbgprintf("begin cpp qishr_full");
  for (QishCpp_StackRoots_Any*r = QishCpp_StackRoots_Any::qishr_top_; 
       r; r= r->_qishr_prev) {
    qish_dbgprintf("fullscanning cpp stack %p nbroots %d nbextra %d", 
		   r, r->_qishr_nbroots, r->_qishr_nbextra);
    r->qishstack_full_scan();
  };
  for (QishCpp_ABox*b = QishCpp_ABox::qishlast_; b;
       b = b->_qishprev) {
    qish_dbgprintf("fullscanning cpp box %p", b);
    b->qishscan_full();
  };
  delete (QishCpp_ABox::qishboxhash_);
  QishCpp_ABox::qishboxhash_ =
    QishCpp_BoxedHash::make(33+QishCpp_ABox::qishnbbox_/16);
};


// eof $Id: qicpp.cpp 35 2005-08-17 09:44:19Z basile $
