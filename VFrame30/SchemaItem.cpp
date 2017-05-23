#include "SchemaItem.h"
#include "SchemaItemAfb.h"
#include "SchemaItemControl.h"
#include "DrawParam.h"
#include "PropertyNames.h"
#include "SchemaView.h"
#include <QPainter>

namespace VFrame30
{
	Factory<VFrame30::SchemaItem> SchemaItemFactory;

	const QColor SchemaItem::errorColor(0xE0, 0x33, 0x33, 0xFF);
	const QColor SchemaItem::warningColor(0xF8, 0x72, 0x17, 0xFF);
	const QColor SchemaItem::selectionColor(0x33, 0x99, 0xFF, 0x80);
	const QColor SchemaItem::lockedSelectionColor(0xF0, 0x80, 0x80, 0xB0);
	const QColor SchemaItem::commentedColor(0xE0, 0xE0, 0xEF, 0xC0);

	// SchemaItem
	//
	SchemaItem::SchemaItem() :
		Proto::ObjectSerialization<SchemaItem>(Proto::ProtoCompress::Never),
		m_itemUnit(SchemaUnit::Display)
	{	
		m_guid = QUuid::createUuid();

		auto commentedProp = ADD_PROPERTY_GETTER_SETTER(bool, PropertyNames::commented, true, SchemaItem::commented, SchemaItem::setCommented);
		commentedProp->setCategory(PropertyNames::functionalCategory);

		auto lockedProp = ADD_PROPERTY_GETTER_SETTER(bool, PropertyNames::locked, true, SchemaItem::isLocked, SchemaItem::setLocked);
		lockedProp->setCategory(PropertyNames::appearanceCategory);

		auto acceptClickProp = ADD_PROPERTY_GETTER_SETTER(bool, PropertyNames::acceptClick, true, SchemaItem::acceptClick, SchemaItem::setAcceptClick);
		acceptClickProp->setCategory(PropertyNames::scriptsCategory);

		auto clickScriptProp = ADD_PROPERTY_GETTER_SETTER(QString, PropertyNames::clickScript, true, SchemaItem::clickScript, SchemaItem::setClickScript);
		clickScriptProp->setCategory(PropertyNames::scriptsCategory);
		clickScriptProp->setIsScript(true);

		auto objectNameProp = ADD_PROPERTY_GETTER_SETTER(QString, PropertyNames::objectName, true, QObject::objectName, SchemaItem::setObjectName);
		objectNameProp->setCategory(PropertyNames::scriptsCategory);

		ADD_PROPERTY_GET_SET_CAT(QString, PropertyNames::preDrawScript, PropertyNames::scriptsCategory, true, SchemaItem::preDrawScript, SchemaItem::setPreDrawScript);

		return;
	}

	SchemaItem::~SchemaItem()
	{
	}
	
	// Serialization
	//
	bool SchemaItem::SaveData(Proto::Envelope* message) const
	{
		const std::string& className = this->metaObject()->className();
		quint32 classnamehash = CUtils::GetClassHashCode(className);

		message->set_classnamehash(classnamehash);	// Обязательное поле, хш имени класса, по нему восстанавливается класс.

		Proto::SchemaItem* schemaItem = message->mutable_schemaitem();

		Proto::Write(schemaItem->mutable_uuid(), m_guid);
		schemaItem->set_isstatic(m_static);
		schemaItem->set_islocked(m_locked);
		schemaItem->set_iscommented(m_commented);
		schemaItem->set_itemunit(static_cast<Proto::SchemaUnit>(m_itemUnit));

		schemaItem->set_acceptclick(m_acceptClick);
		schemaItem->set_clickscript(m_clickScript.toStdString());
		schemaItem->set_predrawscript(m_preDrawScript.toStdString());

		schemaItem->set_objectname(objectName().toStdString());

		return true;
	}

	bool SchemaItem::LoadData(const Proto::Envelope& message)
	{
		if (message.has_schemaitem() == false)
		{
			assert(message.has_schemaitem());
			return false;
		}

		const Proto::SchemaItem& schemaitem = message.schemaitem();

		m_guid = Proto::Read(schemaitem.uuid());
		m_static = schemaitem.isstatic();
		m_locked = schemaitem.islocked();
		m_commented = schemaitem.iscommented();
		m_itemUnit = static_cast<SchemaUnit>(schemaitem.itemunit());

		m_acceptClick = schemaitem.acceptclick();
		m_clickScript = QString::fromStdString(schemaitem.clickscript());
		m_preDrawScript = QString::fromStdString(schemaitem.predrawscript());

		setObjectName(QString::fromStdString(schemaitem.objectname()));

		return true;
	}

	std::shared_ptr<SchemaItem> SchemaItem::CreateObject(const Proto::Envelope& message)
	{
		// Эта функция может создавать только один экземпляр
		//
		if (message.has_schemaitem() == false)
		{
			assert(message.has_schemaitem());
			return nullptr;
		}

		quint32 classNameHash = message.classnamehash();
		std::shared_ptr<SchemaItem> schemaItem = SchemaItemFactory.Create(classNameHash);

		if (schemaItem == nullptr)
		{
			assert(schemaItem);
			return nullptr;
		}
		
		schemaItem->LoadData(message);

		return schemaItem;
	}

	// Action Functions
	//

	void SchemaItem::MoveItem(double /*horzOffsetDocPt*/, double /*vertOffsetDocPt*/)
	{
		assert(false);	// Implement in child classes
	}

	void SchemaItem::snapToGrid(double /*gridSize*/)
	{
		assert(false);
	}

	double SchemaItem::GetWidthInDocPt() const
	{
		assert(false);	// Implement in child classes
		return 0;
	}

	double SchemaItem::GetHeightInDocPt() const
	{
		assert(false);	// Implement in child classes
		return 0;
	}

	void SchemaItem::SetWidthInDocPt(double /*widthInDocPt*/)
	{
		assert(false);	// Implement in child classes
	}

	void SchemaItem::SetHeightInDocPt(double /*heightInDocPt*/)
	{
		assert(false);	// Implement in child classes
	}

	void SchemaItem::dump(std::shared_ptr<SchemaItem> item)
	{
		QString str = QString("SchemaItem %1, [ptr: %2, counter: %3], uuid: %4")
					  .arg(item->metaObject()->className())
					  .arg(reinterpret_cast<qulonglong>(item.get()))
					  .arg(item.use_count())
					  .arg(item->guid().toString());

		qDebug() << str;
	}

	void SchemaItem::dump() const
	{
		qDebug() << "Item: " << metaObject()->className();
		qDebug() << "\tguid:" << guid();
	}

	void SchemaItem::clickEvent(QString globalScript, QJSEngine* engine,  QWidget* parentWidget)
	{
		if (engine == nullptr ||
			parentWidget == nullptr)
		{
			assert(engine);
			assert(parentWidget);
			return;
		}

		if (m_clickScript.trimmed().isEmpty() == true)
		{
			return;
		}

		// Evaluate script
		//
		if (m_jsClickScript.isUndefined() == true)
		{
			m_jsClickScript = evaluateScript(m_clickScript, globalScript, engine, parentWidget);
		}

		if (m_jsClickScript .isError() == true ||
			m_jsClickScript .isNull() == true)
		{
			return;
		}

		runScript(m_jsClickScript, engine);

		return;
	}

	bool SchemaItem::preDrawEvent(QString globalScript, QJSEngine* engine)
	{
		if (engine == nullptr)
		{
			assert(engine);
			return false;
		}

		if (m_preDrawScript.trimmed().isEmpty() == true)
		{
			return true;
		}

		// Evaluate script
		//
		if (m_jsPreDrawScript.isUndefined() == true)
		{
			m_jsPreDrawScript = evaluateScript(m_preDrawScript, globalScript, engine, nullptr);
		}

		if (m_jsPreDrawScript.isError() == true ||
			m_jsPreDrawScript.isNull() == true)
		{
			return false;
		}

		bool result = runScript(m_jsPreDrawScript, engine);

		return result;
	}

	bool SchemaItem::runScript(QJSValue& evaluatedJs, QJSEngine* engine)
	{
		if (evaluatedJs.isUndefined() == true ||
			evaluatedJs.isError() == true ||
			engine == nullptr)
		{
			assert(engine);
			return false;
		}

		// Set argument list
		//
		QJSValue jsSchemaItem = engine->newQObject(this);
		QQmlEngine::setObjectOwnership(this, QQmlEngine::CppOwnership);

		QJSValueList args;
		args << jsSchemaItem;

		// Run script
		//
		QJSValue jsResult = evaluatedJs.call(args);
		if (jsResult.isError() == true)
		{
			m_lastScriptError = formatSqriptError(jsResult);
			return false;
		}
		else
		{
			m_lastScriptError.clear();
		}

		return true;
	}

	QJSValue SchemaItem::evaluateScript(QString script, QString globalScript, QJSEngine* engine, QWidget* parentWidget) const
	{
		if (engine == nullptr)
		{
			assert(engine);
			assert(parentWidget);
			return QJSValue();
		}


		QJSValue result = engine->evaluate(script + globalScript);

		if (result.isError() == true)
		{
			m_lastScriptError = formatSqriptError(result);

			if (parentWidget != nullptr)
			{
				QMessageBox::critical(parentWidget, qAppName(), m_lastScriptError);
			}
		}

		return result;
	}

	QString SchemaItem::formatSqriptError(const QJSValue& scriptValue) const
	{
		qDebug() << "Script running uncaught exception at line " << scriptValue.property("lineNumber").toInt();
		qDebug() << "\tItem: " << guid().toString() << " " << metaObject()->className();
		qDebug() << "\tStack: " << scriptValue.property("stack").toString();
		qDebug() << "\tMessage: " << scriptValue.toString();

		QString str = QString("Script running uncaught exception at line %1\n"
							  "\tItem: %2 %3\n"
							  "\tStack: %4\n"
							  "\tMessage: %5")
					  .arg(scriptValue.property("lineNumber").toInt())
					  .arg(guid().toString()).arg(metaObject()->className())
					  .arg(scriptValue.property("stack").toString())
					  .arg(scriptValue.toString());

		return str;
	}

	void SchemaItem::reportSqriptError(const QJSValue& scriptValue, QWidget* parent) const
	{
		qDebug() << "Script running uncaught exception at line " << scriptValue.property("lineNumber").toInt();
		qDebug() << "\tItem: " << guid().toString() << " " << metaObject()->className();
		qDebug() << "\tStack: " << scriptValue.property("stack").toString();
		qDebug() << "\tMessage: " << scriptValue.toString();

		QMessageBox::critical(parent, QApplication::applicationDisplayName(),
							  tr("Script uncaught exception at line %1:\n%2")
								  .arg(scriptValue.property("lineNumber").toInt())
								  .arg(scriptValue.toString()));

		return;
	}

	bool SchemaItem::searchText(const QString& text) const
	{
		QUuid uuid(text);
		if (uuid.isNull() == false)
		{
			if (uuid == guid())
			{
				return true;
			}
		}

		// FblItem is not derived from SchemaItem, so serach for the text manualy, cant call virtual function
		//
		if (isFblItem() == true)
		{
			if (toFblItem()->searchText(text) == true)
			{
				return true;
			}
		}

		return false;
	}

	// Drawing Functions
	//

	// Рисование элемента, выполняется в 100% масштабе.
	// Graphcis должен иметь экранную координатную систему (0, 0 - левый верхний угол, вниз и вправо - положительные координаты)
	//
	void SchemaItem::Draw(CDrawParam* drawParam, const Schema* schema, const SchemaLayer* layer) const
	{
		Q_UNUSED(drawParam)
		Q_UNUSED(schema)
		Q_UNUSED(layer)
	}

	// Рисование элемента при его создании изменении
	//
	void SchemaItem::DrawOutline(CDrawParam* ) const
	{
	}

	void SchemaItem::DrawOutline(CDrawParam* pDrawParam, const std::vector<std::shared_ptr<SchemaItem>>& items)
	{
		if (pDrawParam == nullptr)
		{
			assert(pDrawParam != nullptr);
			return;
		}

		for (auto vi = items.cbegin(); vi != items.cend(); ++vi)				
		{
			vi->get()->DrawOutline(pDrawParam);
		}
	}

	void SchemaItem::DrawIssue(CDrawParam* /*drawParam*/, OutputMessageLevel /*issue*/) const
	{
		assert(false);
	}

	void SchemaItem::DrawDebugInfo(CDrawParam*, const QString&) const
	{
	}

	void SchemaItem::DrawScriptError(CDrawParam* drawParam) const
	{
		if (m_lastScriptError.isEmpty() == true)
		{
			return;
		}

		QPainter* p = drawParam->painter();

		std::vector<SchemaPoint> points = getPointList();
		QRectF r = {points[0].X, points[0].Y, 2000, 2000};

		static FontParam font("Sans", drawParam->gridSize() * 1.75, false, false);
		p->setPen(Qt::red);

		DrawHelper::drawText(p,
							 font,
							 itemUnit(),
							 m_lastScriptError,
							 r,
							 Qt::TextDontClip | Qt::AlignTop | Qt::AlignLeft);

		return;
	}

	// Нарисовать выделение объекта, в зависимости от используемого интрефейса расположения.
	//
	void SchemaItem::DrawSelection(CDrawParam*, bool) const
	{
	}

	void SchemaItem::DrawSelection(CDrawParam* pDrawParam, const std::vector<std::shared_ptr<SchemaItem>>& items, bool drawSizeBar)
	{
		if (pDrawParam == nullptr)
		{
			assert(pDrawParam != nullptr);
			return;
		}

		for (auto vi = items.cbegin(); vi != items.cend(); ++vi)				
		{
			vi->get()->DrawSelection(pDrawParam, drawSizeBar);
		}
	}

	void SchemaItem::drawCompareAction(CDrawParam* /*drawParam*/, QColor /*color*/) const
	{
	}

	void SchemaItem::drawCommentDim(CDrawParam* /*drawParam*/) const
	{

	}

	// Определение, входит ли точка в элемент, x и y в дюймах или в пикселях
	// 
	bool SchemaItem::IsIntersectPoint(double x, double y) const
	{
		if (itemUnit() == SchemaUnit::Display)
		{
			return IsIntersectRect(x, y, 1, 1);
		}
		else
		{
			// Из точки делается прямоугольник 0.5мм на 0.5мм (0.02 in )
			//
			return IsIntersectRect(x - 0.01, y - 0.01, 0.02, 0.02);
		}
	}

	// Определение, пересекает ли элемент указанный прямоугольник (использовать для выделения),
	// координаты и размер прямоугольника заданы в дюймах или пикселях
	// 
	bool SchemaItem::IsIntersectRect(double x, double y, double width, double height) const
	{ 
		x = x; y = y; width = width; height = height;		// убираю unreferenced warning
		assert(false);
		return false;
	};

	double SchemaItem::penDeviceWidth(const QPaintDevice* device, double penWidth)
	{
		if (dynamic_cast<const QPdfWriter*>(device) != nullptr)
		{
			const QPdfWriter* pdfDevice = dynamic_cast<const QPdfWriter*>(device);

			if (pdfDevice->resolution() >= 600)
			{
				if (penWidth == 0.0)
				{
					return 1.0 / 256.0;
				}
			}
		}

		return penWidth;
	}

	// ISchemaItemPropertiesPos interface implementation
	//
	double SchemaItem::left() const
	{
		assert(true);
		return 0;
	}
	void SchemaItem::setLeft(double)
	{
		assert(true);
	}

	double SchemaItem::top() const
	{
		assert(false);
		return 0;
	}
	void SchemaItem::setTop(double)
	{
		assert(false);
	}

	double SchemaItem::width() const
	{
		assert(false);
		return 0;
	}
	void SchemaItem::setWidth(double)
	{
		assert(false);
	}

	double SchemaItem::height() const
	{
		assert(false);
		return 0;
	}
	void SchemaItem::setHeight(double)
	{
		assert(false);
	}

	std::vector<SchemaPoint> SchemaItem::getPointList() const
	{
		Q_ASSERT(false);
		return std::vector<SchemaPoint>();
	}

	void SchemaItem::setPointList(const std::vector<SchemaPoint>& /*points*/)
	{
		Q_ASSERT(false);
	}

	// Properties and Data
	//

	bool SchemaItem::IsStatic() const
	{
		return m_static;
	}

	bool SchemaItem::IsDynamic() const
	{
		return !m_static;
	}

	bool SchemaItem::isFblItemRect() const
	{
		return dynamic_cast<const FblItemRect*>(this) != nullptr;
	}

	FblItemRect* SchemaItem::toFblItemRect()
	{
		return dynamic_cast<FblItemRect*>(this);
	}

	const FblItemRect* SchemaItem::toFblItemRect() const
	{
		return dynamic_cast<const FblItemRect*>(this);
	}

	bool SchemaItem::isFblItem() const
	{
		return dynamic_cast<const FblItem*>(this) != nullptr;
	}

	FblItem* SchemaItem::toFblItem()
	{
		return dynamic_cast<FblItem*>(this);
	}

	const FblItem* SchemaItem::toFblItem() const
	{
		return dynamic_cast<const FblItem*>(this);
	}

	bool SchemaItem::isSchemaItemAfb() const
	{
		return dynamic_cast<const SchemaItemAfb*>(this) != nullptr;
	}

	SchemaItemAfb* SchemaItem::toSchemaItemAfb()
	{
		return dynamic_cast<SchemaItemAfb*>(this);
	}

	const SchemaItemAfb* SchemaItem::toSchemaItemAfb() const
	{
		return dynamic_cast<const SchemaItemAfb*>(this);
	}

	bool SchemaItem::isControl() const
	{
		return dynamic_cast<const SchemaItemControl*>(this) != nullptr;
	}

	bool SchemaItem::isLocked() const
	{
		return m_locked;
	}

	void SchemaItem::setLocked(bool lock)
	{
		m_locked = lock;
		return;
	}

	bool SchemaItem::isCommented() const
	{
		return m_commented;
	}

	bool SchemaItem::commented() const
	{
		return m_commented;
	}

	void SchemaItem::setCommented(bool value)
	{
		m_commented = value;
	}

	const QUuid& SchemaItem::guid() const
	{
		return m_guid;
	}

	void SchemaItem::setGuid(const QUuid& guid)
	{
		m_guid = guid;
		return;
	}

	void SchemaItem::setNewGuid()
	{
		QUuid uuid = QUuid::createUuid();
		setGuid(uuid);

		// FblItem is not derived from SchemaItem, setNewGuid must be called manualy
		//
		if (this->isType<FblItem>() == true)
		{
			toType<FblItem>()->setNewGuid();
		}

		return;
	}

	// Единицы измерения, в которых хранятся координаты (может быть только дюймы или точки)
	//
	SchemaUnit SchemaItem::itemUnit() const
	{
		return m_itemUnit;
	}

	void SchemaItem::setItemUnit(SchemaUnit value)
	{
		assert(value == SchemaUnit::Display || value == SchemaUnit::Inch);
		m_itemUnit = value;
	}

	// AcceptClick property
	//
	bool SchemaItem::acceptClick() const
	{
		return m_acceptClick;
	}

	void SchemaItem::setAcceptClick(bool value)
	{
		m_acceptClick = value;
	}

	// ClickScript property
	//
	const QString& SchemaItem::clickScript() const
	{
		return m_clickScript;
	}

	void SchemaItem::setClickScript(const QString& value)
	{
		m_clickScript = value;
	}

	QString SchemaItem::preDrawScript() const
	{
		return m_preDrawScript;
	}

	void SchemaItem::setPreDrawScript(const QString& value)
	{
		m_preDrawScript = value;
	}

	QRectF SchemaItem::boundingRectInDocPt() const
	{
		assert(false);		// Must be implemented in child classes
		return QRectF();
	}

	QString SchemaItem::lastScriptError() const
	{
		return m_lastScriptError;
	}

}
