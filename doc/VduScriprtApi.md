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
bool isDiscrete(string appSignalId)
bool isDiscreteByIndex(int appSignalIndex)
```

```c
bool isAnalog(string appSignalId)
bool IsAnalogByIndex(int appSignalIndex)
```

```c
int signalType(string appSignalId)
int signalTypeByIndex(int appSignalIndex)
```

```c
double signalValue(string appSignalId) // double covers float and int32
double signalValueByIndex(int appSignalIndex)
```

```c
float signalFloatValue(string appSignalId)
float signalFloatValueByIndex(int appSignalIndex)
```

```c
int32 signalIntValue(string appSignalId)
int32 signalIntValueByIndex(int appSignalIndex)
```

```c
uint32 signalFlags(string appSignalId)
uint32 signalFlagsByIndex(int appSignalIndex)
```

```c
bool signalExists(string appSignalId)
bool signalExistsByIndex(int appSignalIndex)
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
double itemSignalValue(SchemaItem* item, int signalIndex)
// double covers float and all 32-bit integers, 
// returns NaN if signalIndex is out of range
```

```c
int32 itemSignalIntValue(SchemaItem* item, int signalIndex)
float itemSignalFloatValue(SchemaItem* item, int signalIndex)
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
%V - Signal value + unit
%s - +/- signal value
%S - +/- signal value + unit
%u - Unit
%e - Value in exponential form (1.0e-11)
%E - Value in exponential form (1.0E-11)
%x - Value in HEX (only for integer signal type). m_precision plays the role of the number of zeros to add (00009abc).
%X - Value in HEX (only for integer signal type). m_precision plays the role of the number of zeros to add (00009ABC).
```
**Note: Placeholders can be applied ONLY for the signal with index 0.**

Example: 
> "Value %i: %E %u" -> "Value YCB10B23: 1.0E-11 kg"

## Examples
