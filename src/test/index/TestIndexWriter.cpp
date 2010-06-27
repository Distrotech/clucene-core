/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
*
* Distributable under the terms of either the Apache License (Version 2.0) or
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#include "test.h"
#include <stdio.h>

//checks if a merged index finds phrases correctly
void testIWmergePhraseSegments(CuTest *tc){
	char fsdir[CL_MAX_PATH];
	sprintf(fsdir,"%s/%s",cl_tempDir, "test.indexwriter");
	SimpleAnalyzer a;
	Directory::Pointer dir = FSDirectory::getDirectory(fsdir, true);

	IndexWriter ndx2(dir,&a,true);
	ndx2.setUseCompoundFile(false);
	Document doc0;
	doc0.add(
		*_CLNEW Field(
			_T("field0"),
			_T("value0 value1"),
			Field::STORE_YES | Field::INDEX_TOKENIZED
		)
	);
	ndx2.addDocument(&doc0);
	ndx2.optimize();
	ndx2.close();

	IndexWriter ndx(fsdir,&a,false);
	ndx.setUseCompoundFile(false);
	Document doc1;
	doc1.add(
		*_CLNEW Field(
			_T("field0"),
			_T("value1 value0"),
			Field::STORE_YES | Field::INDEX_TOKENIZED
		)
	);
	ndx.addDocument(&doc1);
	ndx.optimize();
	ndx.close();

	//test the index querying
	IndexSearcher searcher(fsdir);
	Query* query0 = QueryParser::parse(
		_T("\"value0 value1\""),
		_T("field0"),
		&a
	);
	Hits* hits0 = searcher.search(query0);
	CLUCENE_ASSERT(hits0->length() > 0);
	Query* query1 = QueryParser::parse(
		_T("\"value1 value0\""),
		_T("field0"),
		&a
	);
	Hits* hits1 = searcher.search(query1);
	CLUCENE_ASSERT(hits1->length() > 0);
	_CLDELETE(query0);
	_CLDELETE(query1);
	_CLDELETE(hits0);
	_CLDELETE(hits1);
}

//checks that adding more than the min_merge value goes ok...
//checks for a mem leak that used to occur
void testIWmergeSegments1(CuTest *tc){
	Directory::Pointer ram(new RAMDirectory);
	SimpleAnalyzer a;

	IndexWriter ndx2(ram,&a,true);
	ndx2.close(); //test immediate closing bug reported

	IndexWriter ndx(ram,&a,true); //set create to false

	ndx.setUseCompoundFile(false);
	ndx.setMergeFactor(2);
	TCHAR fld[1000];
	for ( int i=0;i<1000;i++ ){
    English::IntToEnglish(i,fld,1000);

		Document doc;

		doc.add ( *_CLNEW Field(_T("field0"),fld,Field::STORE_YES | Field::INDEX_TOKENIZED) );
		doc.add ( *_CLNEW Field(_T("field1"),fld,Field::STORE_YES | Field::INDEX_TOKENIZED) );
		doc.add ( *_CLNEW Field(_T("field2"),fld,Field::STORE_YES | Field::INDEX_TOKENIZED) );
		doc.add ( *_CLNEW Field(_T("field3"),fld,Field::STORE_YES | Field::INDEX_TOKENIZED) );
		ndx.addDocument(&doc);
	}
	//ndx.optimize(); //optimize so we can read terminfosreader with segmentreader
	ndx.close();

	//test the ram loading
	Directory::Pointer ram2(new RAMDirectory(ram));
	IndexReader* reader2 = IndexReader::open(ram2);
	Term::Pointer term(new Term(_T("field0"),fld));
	TermEnum* en = reader2->terms(term);
	CLUCENE_ASSERT(en->next());
	_CLDELETE(en);
	_CLDELETE(reader2);
}

//checks if appending to an index works correctly
void testIWmergeSegments2(CuTest *tc){
	char fsdir[CL_MAX_PATH];
	sprintf(fsdir,"%s/%s",cl_tempDir, "test.indexwriter");
	SimpleAnalyzer a;
	Directory::Pointer dir = FSDirectory::getDirectory(fsdir, true);

	IndexWriter ndx2(dir,&a,true);
	ndx2.setUseCompoundFile(false);
	Document doc0;
	doc0.add(
		*_CLNEW Field(
			_T("field0"),
			_T("value0"),
			Field::STORE_YES | Field::INDEX_TOKENIZED
		)
	);
	ndx2.addDocument(&doc0);
	ndx2.optimize();
	ndx2.close();

	IndexWriter ndx(fsdir,&a,false);
	ndx.setUseCompoundFile(false);
	Document doc1;
	doc1.add(
		*_CLNEW Field(
			_T("field0"),
			_T("value1"),
			Field::STORE_YES | Field::INDEX_TOKENIZED
		)
	);
	ndx.addDocument(&doc1);
	ndx.optimize();
	ndx.close();

	//test the ram querying
	IndexSearcher searcher(fsdir);
	Term::Pointer term0(new Term(_T("field0"),_T("value1")));
	Query* query0 = QueryParser::parse(_T("value0"),_T("field0"),&a);
	Hits* hits0 = searcher.search(query0);
	CLUCENE_ASSERT(hits0->length() > 0);
	Term::Pointer term1(new Term(_T("field0"),_T("value0")));
	Query* query1 = QueryParser::parse(_T("value1"),_T("field0"),&a);
	Hits* hits1 = searcher.search(query1);
	CLUCENE_ASSERT(hits1->length() > 0);
	_CLDELETE(query0);
	_CLDELETE(query1);
	_CLDELETE(hits0);
	_CLDELETE(hits1);
}

void testAddIndexes(CuTest *tc){
	char reuters_origdirectory[1024];
  strcpy(reuters_origdirectory, clucene_data_location);
  strcat(reuters_origdirectory, "/reuters-21578-index");

  {
    Directory::Pointer dir(new RAMDirectory);
    WhitespaceAnalyzer a;
    IndexWriter w(dir, &a, true);
	CLVector<Directory::Pointer, Directory::Deletor> dirs;
    dirs.push_back(FSDirectory::getDirectory(reuters_origdirectory));
    dirs.push_back(FSDirectory::getDirectory(reuters_origdirectory));
    w.addIndexesNoOptimize(dirs);
    w.flush();
    CLUCENE_ASSERT(w.docCount()==62); //31 docs in reuters...

    // TODO: Currently there is a double ref-counting mechanism in place for Directory objects,
    //      so we need to dec them both
    dirs[1]->close();
    dirs[0]->close();
  }
  {
    Directory::Pointer dir(new RAMDirectory);
    WhitespaceAnalyzer a;
    IndexWriter w(dir, &a, true);
	CLVector<Directory::Pointer, Directory::Deletor> dirs;
    dirs.push_back(FSDirectory::getDirectory(reuters_origdirectory));
    dirs.push_back(FSDirectory::getDirectory(reuters_origdirectory));
    w.addIndexes(dirs);
    w.flush();
    CLUCENE_ASSERT(w.docCount()==62); //31 docs in reuters...

    // TODO: Currently there is a double ref-counting mechanism in place for Directory objects,
    //      so we need to dec them both
    dirs[1]->close();
    dirs[0]->close();
  }
}

void testHashingBug(CuTest *tc){
  //Manuel Freiholz's indexing bug

  CL_NS(document)::Document doc;
  CL_NS(document)::Field* field;
  CL_NS(analysis::standard)::StandardAnalyzer analyzer;
  CL_NS(store)::Directory::Pointer dir(new RAMDirectory);
  CL_NS(index)::IndexWriter writer(dir, &analyzer, true, true );

  field = _CLNEW CL_NS(document)::Field( _T("CNS_VERSION"), _T("1"), CL_NS(document)::Field::STORE_NO | CL_NS(document)::Field::INDEX_UNTOKENIZED );
  doc.add( (*field) );

  field = _CLNEW CL_NS(document)::Field( _T("CNS_PID"), _T("5"), CL_NS(document)::Field::STORE_YES | CL_NS(document)::Field::INDEX_UNTOKENIZED );
  doc.add( (*field) );

  field = _CLNEW CL_NS(document)::Field( _T("CNS_DATE"), _T("20090722"), CL_NS(document)::Field::STORE_YES | CL_NS(document)::Field::INDEX_UNTOKENIZED );
  doc.add( (*field) );

  field = _CLNEW CL_NS(document)::Field( _T("CNS_SEARCHDATA"), _T("all kind of data"), CL_NS(document)::Field::STORE_NO | CL_NS(document)::Field::INDEX_TOKENIZED );
  doc.add( (*field) );

  writer.addDocument( &doc ); // ADDING FIRST DOCUMENT. -> this works!

  doc.clear();

  field = _CLNEW CL_NS(document)::Field( _T("CNS_VERSION"), _T("1"), CL_NS(document)::Field::STORE_NO | CL_NS(document)::Field::INDEX_UNTOKENIZED );
  doc.add( (*field) );

  field = _CLNEW CL_NS(document)::Field( _T("CNS_PID"), _T("5"), CL_NS(document)::Field::STORE_YES | CL_NS(document)::Field::INDEX_UNTOKENIZED );
  doc.add( (*field) );

  field = _CLNEW CL_NS(document)::Field( _T("CNS_LINEID"), _T("20"), CL_NS(document)::Field::STORE_YES | CL_NS(document)::Field::INDEX_UNTOKENIZED );
  doc.add( (*field) );

  field = _CLNEW CL_NS(document)::Field( _T("CNS_VT_ORDER"), _T("456033000"), CL_NS(document)::Field::STORE_NO | CL_NS(document)::Field::INDEX_UNTOKENIZED );
  doc.add( (*field) );

  field = _CLNEW CL_NS(document)::Field( _T("CNS_VN_H"), _T("456033000"), CL_NS(document)::Field::STORE_NO | CL_NS(document)::Field::INDEX_UNTOKENIZED );
  doc.add( (*field) );

  field = _CLNEW CL_NS(document)::Field( _T("CNS_VN_HF"), _T("456033000"), CL_NS(document)::Field::STORE_NO | CL_NS(document)::Field::INDEX_UNTOKENIZED );
  doc.add( (*field) );

  field = _CLNEW CL_NS(document)::Field( _T("CNS_VN_D"), _T("456033000"), CL_NS(document)::Field::STORE_NO | CL_NS(document)::Field::INDEX_UNTOKENIZED );
  doc.add( (*field) );

  field = _CLNEW CL_NS(document)::Field( _T("CNS_VN_OD"), _T("456033000"), CL_NS(document)::Field::STORE_NO | CL_NS(document)::Field::INDEX_UNTOKENIZED );
  doc.add( (*field) );

  field = _CLNEW CL_NS(document)::Field( _T("CNS_VN_P1"), _T("456033000"), CL_NS(document)::Field::STORE_NO | CL_NS(document)::Field::INDEX_UNTOKENIZED );
  doc.add( (*field) );

  field = _CLNEW CL_NS(document)::Field( _T("CNS_VN_H1"), _T("456033000"), CL_NS(document)::Field::STORE_NO | CL_NS(document)::Field::INDEX_UNTOKENIZED );
  doc.add( (*field) ); // the problematic field!

  writer.addDocument( &doc ); // ADDING SECOND DOCUMENT - will never return from this function
  writer.optimize();          // stucks in line 222-223
  writer.close();
}

void testExceptionFromTokenStream(CuTest *tc) {

    class TokenFilterWithException : public TokenFilter
    {
    private:
        int count;

    public:
        TokenFilterWithException(TokenStream * in) : 
          TokenFilter(in, true), count(0) {};

          Token* next(Token * pToken) {
              if (count++ == 5) {
                  _CLTHROWA(CL_ERR_IO, "TokenFilterWithException testing IO exception");         
              }
              return input->next(pToken);
          };
    };

    class AnalyzerWithException : public Analyzer
    {
    public:
        TokenStream* tokenStream(const TCHAR * fieldName, Reader * reader) {
            return _CLNEW TokenFilterWithException(_CLNEW WhitespaceTokenizer(reader));
        };
    };

    Directory::Pointer dir(new RAMDirectory());
    AnalyzerWithException a;
    IndexWriter * writer = _CLNEW IndexWriter(dir, &a, true);

    Document* doc = _CLNEW Document();
    doc->add(* _CLNEW Field(_T("content"), _T("aa bb cc dd ee ff gg hh ii"),
        Field::STORE_NO | Field::INDEX_TOKENIZED));
    try {
        writer->addDocument(doc);
        CuFail(tc, _T("did not hit expected exception"));
    } catch (CLuceneError& e) {
    }
    _CLLDELETE(doc);

    // Make sure we can add another normal document
    doc = _CLNEW Document();
    doc->add(* _CLNEW Field(_T("content"), _T("aa bb cc dd"), Field::STORE_NO | Field::INDEX_TOKENIZED));
    writer->addDocument(doc);
    _CLLDELETE(doc);

    // Make sure we can add another normal document
    doc = _CLNEW Document();
    doc->add(* _CLNEW Field(_T("content"), _T("aa bb cc dd"), Field::STORE_NO | Field::INDEX_TOKENIZED));
    writer->addDocument(doc);
    _CLLDELETE(doc);

    writer->close();
    _CLLDELETE(writer);

    IndexReader* reader = IndexReader::open(dir);
    Term::Pointer t(new Term(_T("content"), _T("aa")));
    assertEquals(reader->docFreq(t), 3);
    
    // Make sure the doc that hit the exception was marked
    // as deleted:
    TermDocs* tdocs = reader->termDocs(t);
    int count = 0;
    while(tdocs->next()) {
      count++;
    }
    _CLLDELETE(tdocs);
    assertEquals(2, count);
    
    t->set(_T("content"), _T("gg"));
    assertEquals(reader->docFreq(t), 0);

    reader->close();
    _CLLDELETE(reader);

    dir->close();
}

/**
* Make sure we skip wicked long terms.
*/
void testWickedLongTerm(CuTest *tc) {
    Directory::Pointer dir(new RAMDirectory());
    StandardAnalyzer a;
    IndexWriter* writer = _CLNEW IndexWriter(dir, &a, true);

    TCHAR bigTerm[16383];
    for (int i=0; i<16383; i++)
        bigTerm[i]=_T('x');
    bigTerm[16383] = 0;

    Document* doc = _CLNEW Document();

    // Max length term is 16383, so this contents produces
    // a too-long term:
    TCHAR* contents = _CL_NEWARRAY(TCHAR, 17000);
    _tcscpy(contents, _T("abc xyz x"));
    _tcscat(contents, bigTerm);
    _tcscat(contents, _T(" another term"));
    doc->add(* _CLNEW Field(_T("content"), contents, Field::STORE_NO | Field::INDEX_TOKENIZED));
    writer->addDocument(doc);
    _CLLDELETE(doc);

    // Make sure we can add another normal document
    doc = _CLNEW Document();
    doc->add(* _CLNEW Field(_T("content"), _T("abc bbb ccc"), Field::STORE_NO | Field::INDEX_TOKENIZED));
    writer->addDocument(doc);
    _CLLDELETE(doc);
    writer->close();

    IndexReader* reader = IndexReader::open(dir);

    // Make sure all terms < max size were indexed
    Term::Pointer t(new Term(_T("content"), _T("abc"), true));
    assertEquals(2, reader->docFreq(t));
    t->set(_T("content"), _T("bbb"), true);
    assertEquals(1, reader->docFreq(t));
    t->set(_T("content"), _T("term"), true);
    assertEquals(1, reader->docFreq(t));
    t->set(_T("content"), _T("another"), true);
    assertEquals(1, reader->docFreq(t));

    // Make sure position is still incremented when
    // massive term is skipped:
    t->set(_T("content"), _T("another"), true);
    TermPositions* tps = reader->termPositions(t);
    assertTrue(tps->next());
    assertEquals(1, tps->freq());
    assertEquals(3, tps->nextPosition());
    _CLLDELETE(tps);

    // Make sure the doc that has the massive term is in
    // the index:
    assertEqualsMsg(_T("document with wicked long term should is not in the index!"), 1, reader->numDocs());

    reader->close();
    _CLLDELETE(reader);

    // Make sure we can add a document with exactly the
    // maximum length term, and search on that term:
    doc = _CLNEW Document();
    doc->add(*_CLNEW Field(_T("content"), bigTerm, Field::STORE_NO | Field::INDEX_TOKENIZED));
    StandardAnalyzer sa;
    sa.setMaxTokenLength(100000);
    writer = _CLNEW IndexWriter(dir, &sa, true);
    writer->addDocument(doc);
    _CLLDELETE(doc);
    writer->close();
    reader = IndexReader::open(dir);
    t->set(_T("content"), bigTerm);
    assertEquals(1, reader->docFreq(t));
    reader->close();

    _CLLDELETE(writer);
    _CLLDELETE(reader);

    dir->close();
}

CuSuite *testindexwriter(void)
{
    CuSuite *suite = CuSuiteNew(_T("CLucene IndexWriter Test"));

    SUITE_ADD_TEST(suite, testHashingBug);
    SUITE_ADD_TEST(suite, testAddIndexes);
    SUITE_ADD_TEST(suite, testIWmergeSegments1);
    SUITE_ADD_TEST(suite, testIWmergeSegments2);
    SUITE_ADD_TEST(suite, testIWmergePhraseSegments);

    SUITE_ADD_TEST(suite, testWickedLongTerm);
    SUITE_ADD_TEST(suite, testExceptionFromTokenStream);

  return suite;
}
// EOF
