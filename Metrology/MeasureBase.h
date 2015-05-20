#ifndef MEASUREBASE_H
#define MEASUREBASE_H

#include "Measure.h"
#include "ObjectVector.h"

// ==============================================================================================

class MeasureBase : public PtrObjectVector<MeasureItem>
{

public:

    explicit                MeasureBase();

    int                     load(int measureType);

private:

    int                     m_measureType = MEASURE_TYPE_UNKNOWN;
};

// ==============================================================================================

#endif // MEASUREBASE_H
