/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#include "CLucene/_ApiHeader.h"
#include "DateFilter.h"
#include "CLucene/document/DateField.h"
#include "CLucene/index/Term.h"
#include "CLucene/index/IndexReader.h"
#include "CLucene/index/Terms.h"
#include "CLucene/util/BitSet.h"
#include <boost/shared_ptr.hpp>

CL_NS_USE(index)
CL_NS_USE(util)
CL_NS_USE(document)
CL_NS_DEF(search)

  DateFilter::~DateFilter(){
  }
  
  DateFilter::DateFilter(const DateFilter& copy):
    start( copy.start ),
    end ( copy.end )
  {
  }

  /** Constructs a filter for field <code>f</code> matching times between
    <code>from</code> and <code>to</code>. */
  DateFilter::DateFilter(const TCHAR* f, int64_t from, int64_t to)
  {
    TCHAR* tmp = DateField::timeToString(from);
    start.reset(_CLNEW Term(f, tmp));
    _CLDELETE_CARRAY(tmp);
    
    tmp = DateField::timeToString(to);
    end.reset(_CLNEW Term(start, tmp));
    _CLDELETE_CARRAY(tmp);
  }

  /** Constructs a filter for field <code>f</code> matching times before
    <code>time</code>. */
  DateFilter* DateFilter::Before(const TCHAR* field, int64_t time) {
    return _CLNEW DateFilter(field, 0,time);
  }

  /** Constructs a filter for field <code>f</code> matching times after
    <code>time</code>. */
  DateFilter* DateFilter::After(const TCHAR* field, int64_t time) {
    return _CLNEW DateFilter(field,time, DATEFIELD_DATE_MAX );
  }

  /** Returns a BitSet with true for documents which should be permitted in
    search results, and false for those that should not. */
  BitSet* DateFilter::bits(IndexReader* reader) {
    BitSet* bts = _CLNEW BitSet(reader->maxDoc());

    TermEnum* enumerator = reader->terms(start);
    if (enumerator->term().get() == NULL){
      _CLDELETE(enumerator);
      return bts;
    }
    TermDocs* termDocs = reader->termDocs();

    try {
      while (enumerator->term().get()->compareTo(end.get()) <= 0) {
        termDocs->seek(enumerator->term());
        while (termDocs->next()) {
          bts->set(termDocs->doc());
        }
        if (!enumerator->next()) {
          break;
        }
      }
    } _CLFINALLY (
        termDocs->close();
        _CLDELETE(termDocs);
        enumerator->close();
        _CLDELETE(enumerator);
    );
    return bts;
  }
  
  Filter* DateFilter::clone() const{
  	return _CLNEW DateFilter(*this);	
  }

  TCHAR* DateFilter::toString(){
	size_t len = _tcslen(start.get()->field()) + start.get()->textLength() + end.get()->textLength() + 8;
	TCHAR* ret = _CL_NEWARRAY(TCHAR,len);
	ret[0]=0;
	_sntprintf(ret,len,_T("%s: [%s-%s]"), start.get()->field(),start.get()->text(),end.get()->text());
	return ret;
  }
CL_NS_END
