#pragma once

#include <QtGlobal>

#pragma pack(push, 1)


struct REQUEST_HEADER
{
    quint32 ID;
    quint32 ClientID;
    quint32 Version;
    quint32 No;
    quint32 ErrorCode;
    quint32 DataLen;
};


#pragma pack(pop)
