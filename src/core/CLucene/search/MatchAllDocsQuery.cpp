/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
*
* Distributable under the terms of either the Apache License (Version 2.0) or
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#include "CLucene/_ApiHeader.h"
#include "Query.h"
#include "MatchAllDocsQuery.h"
#include "Explanation.h"
#include <boost/shared_ptr.hpp>
#include "CLucene/index/Term.h"
#include "Searchable.h"

#include "CLucene/store/Directory.h"
#include "CLucene/index/IndexReader.h"
#include "CLucene/util/StringBuffer.h"

CL_NS_DEF(search)

MatchAllDocsQuery::MatchAllScorer::MatchAllScorer(CL_NS(index)::IndexReader* _reader, Similarity* similarity, Weight* w)
			:Scorer(similarity),reader(_reader),id(-1)
{
	maxId = reader->maxDoc() - 1;
	_score = w->getValue();
}

Explanation* MatchAllDocsQuery::MatchAllScorer::explain(int32_t doc) {
	// not called... see MatchAllDocsWeight::explain()
	return NULL;
}

int32_t MatchAllDocsQuery::MatchAllScorer::doc() const {
	return id;
}

bool MatchAllDocsQuery::MatchAllScorer::next() {
	while (id < maxId) {
		id++;
		if (!reader->isDeleted(id)) {
			return true;
		}
	}
	return false;
}

float_t MatchAllDocsQuery::MatchAllScorer::score() {
	return _score;
}

bool MatchAllDocsQuery::MatchAllScorer::skipTo(int32_t target) {
	id = target - 1;
	return next();
}

TCHAR* MatchAllDocsQuery::MatchAllScorer::toString(){
	return stringDuplicate(_T("MatchAllScorer"));
}

MatchAllDocsQuery::MatchAllDocsWeight::MatchAllDocsWeight(MatchAllDocsQuery* enclosingInstance, Searcher* searcher):parentQuery(enclosingInstance){
	this->similarity = searcher->getSimilarity();
}

TCHAR* MatchAllDocsQuery::MatchAllDocsWeight::toString() {
	CL_NS(util)::StringBuffer buf(50);
	buf.append(_T("weight("));

	TCHAR* t = parentQuery->toString();
	buf.append(t);
	_CLDELETE_LCARRAY(t);

	buf.appendChar(_T(')'));
	return buf.giveBuffer();
}

Query* MatchAllDocsQuery::MatchAllDocsWeight::getQuery() {
	return parentQuery;
}

float_t MatchAllDocsQuery::MatchAllDocsWeight::getValue() {
	return queryWeight;
}

float_t MatchAllDocsQuery::MatchAllDocsWeight::sumOfSquaredWeights() {
	queryWeight = parentQuery->getBoost();
	return queryWeight * queryWeight;
}

void MatchAllDocsQuery::MatchAllDocsWeight::normalize(float_t _queryNorm) {
	this->queryNorm = _queryNorm;
	queryWeight *= this->queryNorm;
}

Scorer* MatchAllDocsQuery::MatchAllDocsWeight::scorer(CL_NS(index)::IndexReader* reader) {
	return _CLNEW MatchAllScorer(reader, similarity, this);
}

Explanation* MatchAllDocsQuery::MatchAllDocsWeight::explain(CL_NS(index)::IndexReader* reader, int32_t doc) {
	// explain query weight
	Explanation* queryExpl = _CLNEW ComplexExplanation(true, getValue(), _T("MatchAllDocsQuery, product of:"));
	if (parentQuery->getBoost() != 1.0f) {
		queryExpl->addDetail(_CLNEW Explanation(parentQuery->getBoost(),_T("boost")));
	}
	queryExpl->addDetail(_CLNEW Explanation(queryNorm,_T("queryNorm")));
	return queryExpl;
}

MatchAllDocsQuery::MatchAllDocsQuery(){
}

MatchAllDocsQuery::~MatchAllDocsQuery(){}

Weight* MatchAllDocsQuery::_createWeight(Searcher* searcher){
	return _CLNEW MatchAllDocsWeight(this, searcher);
}

const char* MatchAllDocsQuery::getClassName() {
	return "MatchAllDocsQuery";
}
const char* MatchAllDocsQuery::getObjectName() const{
	return getClassName();
}

TCHAR* MatchAllDocsQuery::toString(const TCHAR* field) const{
	CL_NS(util)::StringBuffer buffer(25);
    buffer.append(_T("MatchAllDocsQuery"));
    buffer.appendBoost(getBoost());
    return buffer.giveBuffer();
}

MatchAllDocsQuery::MatchAllDocsQuery(const MatchAllDocsQuery& clone){
}

Query* MatchAllDocsQuery::clone() const{
	return _CLNEW MatchAllDocsQuery(*this);
}

bool MatchAllDocsQuery::equals(Query* o) const{
	if (!(o->instanceOf(MatchAllDocsQuery::getClassName())))
		return false;
	MatchAllDocsQuery* other = static_cast<MatchAllDocsQuery*>(o);
	return this->getBoost() == other->getBoost();
}

size_t MatchAllDocsQuery::hashCode() const{
	return (static_cast<size_t>(getBoost())) ^ 0x1AA71190;
}

CL_NS_END
