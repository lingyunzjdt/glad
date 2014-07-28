/*=============================================================================
    Copyright (c) 2014 Lingyun Yang
=============================================================================*/
#if defined(_MSC_VER)
# pragma warning(disable: 4345)
#endif

#include "statement_def.h"

typedef std::string::const_iterator iterator_type;
template struct glad::statement<iterator_type>;
