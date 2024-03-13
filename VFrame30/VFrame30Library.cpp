#include "VFrame30Library.h"

#include "./SchemaItems/FblItemLine.h"
#include "./SchemaItems/FblItemRect.h"
#include "./SchemaItems/SchemaItemAfb.h"
#include "./SchemaItems/SchemaItemBus.h"
#include "./SchemaItems/SchemaItemConnection.h"
#include "./SchemaItems/SchemaItemConst.h"
#include "./SchemaItems/SchemaItemDiagValue.h"
#include "./SchemaItems/SchemaItemFrame.h"
#include "./SchemaItems/SchemaItemImage.h"
#include "./SchemaItems/SchemaItemImageValue.h"
#include "./SchemaItems/SchemaItemIndicator.h"
#include "./SchemaItems/SchemaItemLine.h"
#include "./SchemaItems/SchemaItemLineEdit.h"
#include "./SchemaItems/SchemaItemLink.h"
#include "./SchemaItems/SchemaItemLoopback.h"
#include "./SchemaItems/SchemaItemPath.h"
#include "./SchemaItems/SchemaItemPushButton.h"
#include "./SchemaItems/SchemaItemRect.h"
#include "./SchemaItems/SchemaItemSignal.h"
#include "./SchemaItems/SchemaItemSlider.h"
#include "./SchemaItems/SchemaItemTerminator.h"
#include "./SchemaItems/SchemaItemUfb.h"
#include "./SchemaItems/SchemaItemValue.h"
#include "./SchemaItems/SchemaItemVduLine.h"
#include "./SchemaItems/SchemaItemVduRect.h"
#include "./SchemaItems/SchemaItemVduValue.h"
#include "DiagSchema.h"
#include "LogicSchema.h"
#include "MonitorSchema.h"
#include "Schema.h"
#include "SchemaLayer.h"
#include "TuningSchema.h"
#include "UfbSchema.h"
#include "WiringSchema.h"
#include "VduSchema.h"

namespace VFrame30
{
	bool init()
	{
		qDebug() << Q_FUNC_INFO;

		// Registering Schemas
		//
		SchemaFactory.Register<DiagSchema>();
		SchemaFactory.Register<LogicSchema>();
		SchemaFactory.Register<UfbSchema>();
		SchemaFactory.Register<MonitorSchema>();
		SchemaFactory.Register<TuningSchema>();
		SchemaFactory.Register<WiringSchema>();
		SchemaFactory.Register<VduSchema>();

		// Registering VideoLayers
		//
		VideoLayerFactory.Register<SchemaLayer>();

		// Registering SchemaItems
		//
		//SchemaItemFactory.Register<SchemaItemFrame>();
		SchemaItemFactory.Register<FblItemLine>();
		SchemaItemFactory.Register<FblItemRect>();
		SchemaItemFactory.Register<SchemaItemAfb>();
		SchemaItemFactory.Register<SchemaItemBus>();
		SchemaItemFactory.Register<SchemaItemBusComposer>();
		SchemaItemFactory.Register<SchemaItemBusExtractor>();
		SchemaItemFactory.Register<SchemaItemConst>();
		SchemaItemFactory.Register<SchemaItemDiagValue>();
		SchemaItemFactory.Register<SchemaItemImage>();
		SchemaItemFactory.Register<SchemaItemImageValue>();
		SchemaItemFactory.Register<SchemaItemIndicator>();
		SchemaItemFactory.Register<SchemaItemInOut>();
		SchemaItemFactory.Register<SchemaItemInput>();
		SchemaItemFactory.Register<SchemaItemLine>();
		SchemaItemFactory.Register<SchemaItemLineEdit>();
		SchemaItemFactory.Register<SchemaItemLink>();
		SchemaItemFactory.Register<SchemaItemLoopback>();
		SchemaItemFactory.Register<SchemaItemLoopbackSource>();
		SchemaItemFactory.Register<SchemaItemLoopbackTarget>();
		SchemaItemFactory.Register<SchemaItemOutput>();
		SchemaItemFactory.Register<SchemaItemPath>();
		SchemaItemFactory.Register<SchemaItemPushButton>();
		SchemaItemFactory.Register<SchemaItemReceiver>();
		SchemaItemFactory.Register<SchemaItemRect>();
		SchemaItemFactory.Register<SchemaItemSlider>();
		SchemaItemFactory.Register<SchemaItemTerminator>();
		SchemaItemFactory.Register<SchemaItemTransmitter>();
		SchemaItemFactory.Register<SchemaItemUfb>();
		SchemaItemFactory.Register<SchemaItemValue>();
		
		SchemaItemFactory.Register<SchemaItemVduLine>();
		SchemaItemFactory.Register<SchemaItemVduRect>();
		SchemaItemFactory.Register<SchemaItemVduValue>();

		QMetaType::registerConverter<int, VFrame30::SchemaItemConst::ConstType>(IntToEnum<VFrame30::SchemaItemConst::ConstType>);

		qRegisterMetaType<E::ColumnData>();

		return true;
	}

	bool shutdown()
	{
		qDebug() << Q_FUNC_INFO;

		SchemaItem::PrintRefCounter();

		return true;
	}
}
