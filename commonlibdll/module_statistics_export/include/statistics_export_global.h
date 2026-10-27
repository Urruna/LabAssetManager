#pragma once

#include <QtCore/qglobal.h>

#if defined(EXPORT_LIBRARY)
#  define EXPORT_EXPORT Q_DECL_EXPORT
#else
#  define EXPORT_EXPORT Q_DECL_IMPORT
#endif
