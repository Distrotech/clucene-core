/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
*
* Distributable under the terms of either the Apache License (Version 2.0) or
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#include "test.h"

unittest tests[] = {
    {"threads", testatomicupdates},
    {"indexreader", testindexreader},
    {"reuters", testreuters},
    {"analyzers", testanalyzers},
    {"document", testdocument},
    {"numbertools", testNumberTools},
    {"debug", testdebug},
    {"analysis", testanalysis},
    {"indexwriter", testindexwriter},
    {"indexmodifier", testindexmodifier},
    {"highfreq", testhighfreq},
    {"priorityqueue", testpriorityqueue},
    {"queryparser", testQueryParser},
    {"mfqueryparser", testMultiFieldQueryParser},
    {"search", testsearch},
    {"queries", testqueries},
    {"termvector",testtermvector},
    {"sort",testsort},
    {"duplicates", testduplicates},
    {"datefilter", testdatefilter},
    {"wildcard", testwildcard},
    {"store", teststore},
    {"utf8", testutf8},
    {"LastTest", NULL}
};
