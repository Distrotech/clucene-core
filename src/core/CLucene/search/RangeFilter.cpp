/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#include "CLucene/_ApiHeader.h"
#include <boost/shared_ptr.hpp>
#include "CLucene/index/Term.h"
#include "CLucene/index/Terms.h"
#include "CLucene/store/Directory.h"
#include "CLucene/index/IndexReader.h"
#include "CLucene/util/BitSet.h"
#include "RangeFilter.h"

CL_NS_DEF(search)
CL_NS_USE(index)
CL_NS_USE(util)
CL_NS_USE(document)


RangeFilter::RangeFilter( const TCHAR* fieldName, const TCHAR* lowerTerm, const TCHAR* upperTerm,
                         bool includeLower, bool includeUpper ) : field(NULL), lowerValue(NULL), upperValue(NULL)
{
    if (NULL == lowerTerm && NULL == upperTerm) {
        _CLTHROWT(CL_ERR_IllegalArgument, _T("At least one value must be non-null"));
    }
    if (includeLower && NULL == lowerTerm) {
        _CLTHROWT(CL_ERR_IllegalArgument, _T("The lower bound must be non-null to be inclusive"));
    }
    if (includeUpper && NULL == upperTerm) {
        _CLTHROWT(CL_ERR_IllegalArgument, _T("The upper bound must be non-null to be inclusive"));
    }

	this->field = STRDUP_TtoT(fieldName);
	if ( lowerTerm != NULL )
		this->lowerValue = STRDUP_TtoT(lowerTerm);
	if ( upperTerm != NULL )
		this->upperValue = STRDUP_TtoT(upperTerm);
	this->includeLower = includeLower;
	this->includeUpper = includeUpper;
}

RangeFilter* RangeFilter::Less( const TCHAR* fieldName, const TCHAR* upperTerm ) {
	return _CLNEW RangeFilter( fieldName, NULL, upperTerm, false, true );
}

RangeFilter* RangeFilter::More( const TCHAR* fieldName, const TCHAR* lowerTerm ) {
	return _CLNEW RangeFilter( fieldName, lowerTerm, NULL, true, false );
}


RangeFilter::~RangeFilter()
{
    _CLDELETE_LCARRAY( field );
	_CLDELETE_LCARRAY( lowerValue );
	_CLDELETE_LCARRAY( upperValue );
}


RangeFilter::RangeFilter( const RangeFilter& copy ) : 
	field( STRDUP_TtoT(copy.field) ),
	lowerValue( STRDUP_TtoT(copy.lowerValue) ), 
	upperValue( STRDUP_TtoT(copy.upperValue) ),
	includeLower( copy.includeLower ),
	includeUpper( copy.includeUpper )
{
}


Filter* RangeFilter::clone() const {
	return _CLNEW RangeFilter(*this );
}

/** Returns a BitSet with true for documents which should be permitted in
search results, and false for those that should not. */
BitSet* RangeFilter::bits( IndexReader* reader )
{
	BitSet* bts = _CLNEW BitSet( reader->maxDoc() );
	Term::Pointer term;
	
	Term::Pointer t(new Term( field, (lowerValue ? lowerValue : _T("")), false ));
	TermEnum* enumerator = reader->terms( t );	// get enumeration of all terms after lowerValue
	
	if( !enumerator->term() ) {
		_CLDELETE( enumerator );
		return bts;
	}
	
	bool checkLower = false;
	if( !includeLower ) // make adjustments to set to exclusive
		checkLower = true;
	
	TermDocs* termDocs = reader->termDocs();
	
	try
	{
		do
		{
			term.swap(enumerator->term());
			
			if( !term || _tcscmp(term->field(), field) )
				break;
			
			if( !checkLower || lowerValue == NULL || _tcscmp(term->text(), lowerValue) > 0 )
			{
				checkLower = false;
				if( upperValue != NULL )
				{
					int compare = _tcscmp( upperValue, term->text() );
					
					/* if beyond the upper term, or is exclusive and
					 * this is equal to the upper term, break out */
					if( (compare < 0) || (!includeUpper && compare == 0) )
						break;
				}
				
				termDocs->seek( enumerator->term() );
				while( termDocs->next() ) {
					bts->set( termDocs->doc() );
				}
			}
			
		}
		while( enumerator->next() );
	}
	_CLFINALLY
	(
		termDocs->close();
		_CLVDELETE( termDocs );
		enumerator->close();
		_CLDELETE( enumerator );
	);
	
	return bts;
}

TCHAR* RangeFilter::toString()
{
	size_t len = (field ? _tcslen(field) : 0) + (lowerValue ? _tcslen(lowerValue) : 0) + (upperValue ? _tcslen(upperValue) : 0) + 8;
	TCHAR* ret = _CL_NEWARRAY( TCHAR, len );
	ret[0] = 0;
	_sntprintf( ret, len, _T("%s: [%s-%s]"), field, (lowerValue?lowerValue:_T("")), (upperValue?upperValue:_T("")) );
	
	return ret;
}

CL_NS_END
