#include "VFrame30Library.h"

#include "../CommonLib/Types.h"

#include "DiagSchema.h"
#include "FblItemLine.h"
#include "FblItemRect.h"
#include "LogicSchema.h"
#include "MonitorSchema.h"
#include "Schema.h"
#include "SchemaItemAfb.h"
#include "SchemaItemBus.h"
#include "SchemaItemConnection.h"
#include "SchemaItemConst.h"
#include "SchemaItemDiagValue.h"
#include "SchemaItemFrame.h"
#include "SchemaItemImage.h"
#include "SchemaItemImageValue.h"
#include "SchemaItemIndicator.h"
#include "SchemaItemLine.h"
#include "SchemaItemLineEdit.h"
#include "SchemaItemLink.h"
#include "SchemaItemLoopback.h"
#include "SchemaItemPath.h"
#include "SchemaItemPushButton.h"
#include "SchemaItemRect.h"
#include "SchemaItemSignal.h"
#include "SchemaItemSlider.h"
#include "SchemaItemTerminator.h"
#include "SchemaItemUfb.h"
#include "SchemaItemValue.h"
#include "SchemaLayer.h"
#include "TuningSchema.h"
#include "UfbSchema.h"
#include "WiringSchema.h"


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
