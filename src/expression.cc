/*=============================================================================
    Copyright (c) 2014 Lingyun Yang
=============================================================================*/
#if defined(_MSC_VER)
# pragma warning(disable: 4345)
#endif

#include "expression_def.h"

typedef std::string::const_iterator iterator_type;
template struct glad::expression<iterator_type>;
