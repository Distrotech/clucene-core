/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#ifndef _lucene_search_RangeQuery_
#define _lucene_search_RangeQuery_

//#include "SearchHeader.h"
//#include "Scorer.h"
//#include "TermQuery.h"
#include "Query.h"

#include <boost/shared_ptr.hpp>
#include "CLucene/index/Term.h"

CL_CLASS_DEF(util,StringBuffer)


CL_NS_DEF(search)

/**
 * A Query that matches documents within an exclusive range. A RangeQuery
 * is built by QueryParser for input like <code>[010 TO 120]</code> but only if the QueryParser has
 * the useOldRangeQuery property set to true. The QueryParser default behaviour is to use
 * the newer ConstantScoreRangeQuery class. This is generally preferable because:
 * <ul>
 *  <li>It is faster than RangeQuery</li>
 *  <li>Unlike RangeQuery, it does not cause a BooleanQuery.TooManyClauses exception if the range of values is large</li>
 *  <li>Unlike RangeQuery it does not influence scoring based on the scarcity of individual terms that may match</li>
 * </ul>
 *
 *
 * @see ConstantScoreRangeQuery
 *
 *
 * @version $Id: RangeQuery.java 520891 2007-03-21 13:58:47Z yonik $
 */
class CLUCENE_EXPORT RangeQuery: public Query
{
private:
  CL_NS(index)::Term::Pointer lowerTerm;
  CL_NS(index)::Term::Pointer upperTerm;
  bool inclusive;
protected:
  RangeQuery(const RangeQuery& clone);

public:
  /** Constructs a query selecting all terms greater than
    * <code>lowerTerm</code> but less than <code>upperTerm</code>.
    * There must be at least one term and either term may be null,
    * in which case there is no bound on that side, but if there are
    * two terms, both terms <b>must</b> be for the same field.
    */
  RangeQuery(CL_NS(index)::Term::Pointer LowerTerm, CL_NS(index)::Term::Pointer UpperTerm, const bool Inclusive);
  ~RangeQuery();

  const char* getObjectName() const;
  static const char* getClassName();

  Query* rewrite(CL_NS(index)::IndexReader* reader);

  Query* combine(CL_NS(util)::ArrayBase<Query*>* queries);

  // Prints a user-readable version of this query.
  TCHAR* toString(const TCHAR* field) const;

  Query* clone() const;

  bool equals(Query * other) const;

  CL_NS(index)::Term* getLowerTerm() const;
  CL_NS(index)::Term::Pointer getLowerTermPointer() const;
  CL_NS(index)::Term* getUpperTerm() const;
  CL_NS(index)::Term::Pointer getUpperTermPointer() const;
  bool isInclusive() const;
  const TCHAR* getField() const;

  size_t hashCode() const;
};

CL_NS_END
#endif
