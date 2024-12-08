# VDU Script API

## Schemas

```c
string schemaId()
string schemaCaption()
```

```c
void setSchema(string schemaId)
```

## Common - AppSignals

```c
bool signalExists(string signalId)
bool signalExistsByIndex(int signalIndex)
```

```c
bool isDiscrete(string signalId)
bool isDiscreteByIndex(int signalIndex)
```

```c
bool isAnalog(string signalId)
bool isAnalogByIndex(int signalIndex)
```

```c
bool isTuning(string signalId)
bool isTuningByIndex(int signalIndex)
```

```c
int signalType(string signalId)
int signalTypeByIndex(int signalIndex)
```

```c
float signalValueFloat(string signalId)
float signalValueByIndexFloat(int signalIndex)
```

```c
int32 signalValueInt(string signalId)
int32 signalValueByIndexInt(int signalIndex)
```

```c
uint32 signalFlags(string signalId)
uint32 signalFlagsByIndex(int signalIndex)
```

```c
int32 signalPropertyInt(string signalId, int property)
int32 signalPropertyByIndexInt(int signalIndex, int property)

float signalPropertyFloat(string signalId, int property)
float signalPropertyByIndexFloat(int signalIndex, int property)

// Argument property can be one of the following values:
// 0 - TuningDefaultValue (float or int32, depending on the signal type)
// 1 - LowEngUnits (float or int32, depending on the signal type)
// 2 - HighEngUnits (float or int32, depending on the signal type)
// 11 - TuningLowBound (float or int32, depending on the signal type)
// 12 - TuningHighBound (float or int32, depending on the signal type)
// 100 - DecimalPlaces (int32)
```

```c
int32 signalTuningDefaultValueInt(string signalId)
float signalTuningDefaultValueFloat(string signalId)
int32 signalTuningDefaultValueByIndexInt(int signalIndex)
float signalTuningDefaultValueByIndexFloat(int signalIndex)
```

```c
int32 signalLowEngUnitsInt(string signalId)
float signalLowEngUnitsFloat(string signalId)
int32 signalLowEngUnitsByIndexInt(int signalIndex)
float signalLowEngUnitsByIndexFloat(int signalIndex)

int32 signalHighEngUnitsInt(string signalId)
float signalHighEngUnitsFloat(string signalId)
int32 signalHighEngUnitsByIndexInt(int signalIndex)
float signalHighEngUnitsByIndexFloat(int signalIndex)
```

```c
int32 signalTuningLowBoundInt(string signalId)
float signalTuningLowBoundFloat(string signalId)
int32 signalTuningLowBoundByIndexInt(int signalIndex)
float signalTuningLowBoundByIndexFloat(int signalIndex)

int32 signalTuningHighBoundInt(string signalId)
float signalTuningHighBoundFloat(string signalId)
int32 signalTuningHighBoundByIndexInt(int signalIndex)
float signalTuningHighBoundByIndexFloat(int signalIndex)
```

```c
int32 decimalPlaces(string signalId)
int32 decimalPlacesByIndex(int signalIndex)
```

## Control - AppSignals

```c
bool writeValueInt(string signalId, int32 value);
bool writeValueByIndexInt(int signalIndex, int32 value);

bool writeValueFloat(string signalId, float value);
bool writeValueByIndexFloat(string signalId, float value);
```

```c
bool copyValue(string sourceSignalId, string targetSignalId);
bool copyValueByIndex(int sourceSignalIndex, int targetSignalIndex);
```


## SchemaItem - Common

```c
SchemaItem* currentItem()
```

```c
SchemaItem* itemByObjectName(string objectName)
```

```c
int itemType(SchemaItem* item)
// Returns 
//	Line - 0x4E4C, 
//	Rect - 0x4352, 
//	Value - 0x4C56
```

```c
bool itemIsStatic(SchemaItem* item)
```

```c
string itemObjectName(SchemaItem* item)
```

```c
int itemSignalCount(SchemaItem* item)
```

```c
int signalGlobalIndex(SchemaItem* item, int signalIndex)
// Returns global AppSignal index
```

```c
int32 itemSignalValueInt(SchemaItem* item, int signalIndex)
float itemSignalValueFloat(SchemaItem* item, int signalIndex)
```

```c
bool itemAcceptClick(SchemaItem* item);
void itemSetAcceptClick(SchemaItem* item, bool value);
```

### SchemaItemVduLine
> Type = 0x4E4C

```c
int itemLineWeight(SchemaItem* item)
void itemLineSetWeight(SchemaItem* item, int weight)
```
```c
uint32 itemLineColor(SchemaItem* item)
void itemLineSetLineColor(SchemaItem* item,uint32 color)
```

### SchemaItemVduRect
> Type = 0x4352

```c
int itemRectWeight(SchemaItem* item)
void itemRectSetWeight(SchemaItem* item,int weight)
```

```c
bool itemRectFill(SchemaItem* item)
void itemRectSetFill(SchemaItem* item, bool fill)
```

```c
bool itemRectDrawRect(SchemaItem* item)
void itemRectSetDrawRect(SchemaItem* item, bool drawRect)
```

```c
uint32 itemRectLineColor(SchemaItem* item)
void itemRectSetLineColor(SchemaItem* item, uint32 color)
```

```c
uint32 itemRectFillColor(SchemaItem* item)
void itemRectSetFillColor(Item* item, uint32 color)
```

```c
uint32 itemRectTextColor(SchemaItem* item)
void itemRectSetTextColor(SchemaItem* item, uint32 color)
```

```c
string itemRectText(SchemaItem* item)
void itemRectSetText(SchemaItem* item, string text)
```

### SchemaItemVduValue
> Type = 0x4C56

```c
int itemValueWeight(SchemaItem* item)
void itemValueSetWeight(SchemaItem* item, int weight)
```

```c
bool itemValueDrawRect(SchemaItem* item)
void itemValueSetDrawRect(SchemaItem* item, bool drawRect)
```

```c
uint32 itemValueLineColor(SchemaItem* item)
void itemValueSetLineColor(SchemaItem* item, uint32 color)
```

```c
uint32 itemValueFillColor(SchemaItem* item)
void itemValueSetFillColor(SchemaItem* item, uint32 color)
```

```c
uint32 itemValueTextColor(SchemaItem* item)
void itemValueSetTextColor(SchemaItem* item, uint32 color)
```

```c
int itemValueDecimalPlaces(SchemaItem* item)
void itemValueSetDecimalPlaces(SchemaItem* item, int decimalPlaces)
```

```c
string itemValueText(SchemaItem* item)
void itemValueSetText(SchemaItem* item, string text)
```

Text can contain placeholders for signals in the form of:
```
%% - Sign '%'
%i - CustomAppSignalID
%c - Signal caption
%v - Signal value
%V - Signal value + units
%s - +/- signal value
%S - +/- signal value + units
%u - Units
%e - Value in exponential form (1.0e-11)
%E - Value in exponential form (1.0E-11)
%x - Value in HEX (only for integer signal type). m_precision plays the role of the number of zeros to add (00009abc).
%X - Value in HEX (only for integer signal type). m_precision plays the role of the number of zeros to add (00009ABC).
```
**Note: Placeholders can be applied ONLY for the signal with index 0.**

Example: 
> "Value %i: %E %u" -> "Value YCB10B23: 1.0E-11 kg"

## Examples
