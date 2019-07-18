#include "VFrame30Library.h"
#include "Schema.h"
#include "DiagSchema.h"
#include "LogicSchema.h"
#include "UfbSchema.h"
#include "MonitorSchema.h"
#include "TuningSchema.h"
#include "WiringSchema.h"
#include "SchemaLayer.h"
#include "SchemaItemLine.h"
#include "SchemaItemRect.h"
#include "SchemaItemFrame.h"
#include "SchemaItemPath.h"
#include "SchemaItemImage.h"
#include "FblItemLine.h"
#include "FblItemRect.h"
#include "SchemaItemLink.h"
#include "SchemaItemAfb.h"
#include "SchemaItemSignal.h"
#include "SchemaItemConst.h"
#include "SchemaItemConnection.h"
#include "SchemaItemUfb.h"
#include "SchemaItemTerminator.h"
#include "SchemaItemPushButton.h"
#include "SchemaItemLineEdit.h"
#include "SchemaItemValue.h"
#include "SchemaItemImageValue.h"
#include "SchemaItemBus.h"
#include "SchemaItemLoopback.h"
#include "../lib/Types.h"

namespace VFrame30
{
	VFrame30Library::VFrame30Library()
	{
		qDebug() << Q_FUNC_INFO;
	}

	bool VFrame30Library::init()
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
		SchemaItemFactory.Register<SchemaItemLine>();
		SchemaItemFactory.Register<SchemaItemRect>();
		SchemaItemFactory.Register<SchemaItemPath>();
		SchemaItemFactory.Register<FblItemLine>();
		SchemaItemFactory.Register<FblItemRect>();
		SchemaItemFactory.Register<SchemaItemFrame>();
		SchemaItemFactory.Register<SchemaItemLink>();
		SchemaItemFactory.Register<SchemaItemImage>();
		SchemaItemFactory.Register<SchemaItemAfb>();
		SchemaItemFactory.Register<SchemaItemInput>();
		SchemaItemFactory.Register<SchemaItemOutput>();
		SchemaItemFactory.Register<SchemaItemInOut>();
		SchemaItemFactory.Register<SchemaItemConst>();
		SchemaItemFactory.Register<SchemaItemTransmitter>();
		SchemaItemFactory.Register<SchemaItemReceiver>();
		SchemaItemFactory.Register<SchemaItemUfb>();
		SchemaItemFactory.Register<SchemaItemTerminator>();
		SchemaItemFactory.Register<SchemaItemPushButton>();
		SchemaItemFactory.Register<SchemaItemLineEdit>();
		SchemaItemFactory.Register<SchemaItemValue>();
		SchemaItemFactory.Register<SchemaItemImageValue>();
		SchemaItemFactory.Register<SchemaItemBus>();
		SchemaItemFactory.Register<SchemaItemBusComposer>();
		SchemaItemFactory.Register<SchemaItemBusExtractor>();
		SchemaItemFactory.Register<SchemaItemLoopback>();
		SchemaItemFactory.Register<SchemaItemLoopbackSource>();
		SchemaItemFactory.Register<SchemaItemLoopbackTarget>();

		QMetaType::registerConverter<int, VFrame30::SchemaItemConst::ConstType>(IntToEnum<VFrame30::SchemaItemConst::ConstType>);

		return true;
	}

	bool VFrame30Library::shutdown()
	{
		qDebug() << Q_FUNC_INFO;

		SchemaItem::PrintRefCounter("SchemaItem");

		google::protobuf::ShutdownProtobufLibrary();
		return true;
	}
}
