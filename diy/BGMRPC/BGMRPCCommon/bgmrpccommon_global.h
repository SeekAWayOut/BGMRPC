#ifndef BGMRPCCOMMON_GLOBAL_H
#define BGMRPCCOMMON_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(BGMRPCCOMMON_LIBRARY)
#  define BGMRPCCOMMONSHARED_EXPORT Q_DECL_EXPORT
#else
#  define BGMRPCCOMMONSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // BGMRPCCOMMON_GLOBAL_H