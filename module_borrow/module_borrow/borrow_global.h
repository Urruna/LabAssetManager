#pragma once

#include <QtCore/qglobal.h>

#if defined(BORROW_LIBRARY)
#  define BORROW_EXPORT Q_DECL_EXPORT
#else
#  define BORROW_EXPORT Q_DECL_IMPORT
#endif
