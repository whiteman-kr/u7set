#include "Stable.h"
#include "SchemeItem.h"
#include "SchemeItemRect.h"

namespace VFrame30
{
	Factory<VFrame30::SchemeItem> SchemeItemFactory;

	// SchemeItem

	SchemeItem::SchemeItem() :
		m_static(true),
		m_locked(false),
		m_itemUnit(SchemeUnit::Display),
		m_acceptClick(false)
	{	
		m_guid = QUuid::createUuid();

		ADD_PROPERTY_GETTER_SETTER(bool, AcceptClick, true, SchemeItem::acceptClick, SchemeItem::setAcceptClick);
		ADD_PROPERTY_GETTER_SETTER(QString, ClickScript, true, SchemeItem::clickScript, SchemeItem::setClickScript);

	}

	SchemeItem::~SchemeItem()
	{
	}
	
	// Serialization
	//

	bool SchemeItem::SaveData(Proto::Envelope* message) const
	{
		const std::string& className = this->metaObject()->className();
		quint32 classnamehash = CUtils::GetClassHashCode(className);

		message->set_classnamehash(classnamehash);	// Обязательное поле, хш имени класса, по нему восстанавливается класс.

		Proto::SchemeItem* schemeItem = message->mutable_schemeitem();

		Proto::Write(schemeItem->mutable_uuid(), m_guid);
		schemeItem->set_isstatic(m_static);
		schemeItem->set_islocked(m_locked);
		schemeItem->set_itemunit(static_cast<Proto::SchemeUnit>(m_itemUnit));

		schemeItem->set_acceptclick(m_acceptClick);

		if (m_clickScript.isEmpty() == false)
		{
			Proto::Write(schemeItem->mutable_clickscript(), m_clickScript);
		}

		return true;
	}

	bool SchemeItem::LoadData(const Proto::Envelope& message)
	{
		if (message.has_schemeitem() == false)
		{
			assert(message.has_schemeitem());
			return false;
		}

		const Proto::SchemeItem& schemeitem = message.schemeitem();

		m_guid = Proto::Read(schemeitem.uuid());
		m_static = schemeitem.isstatic();
		m_locked = schemeitem.islocked();
		m_itemUnit = static_cast<SchemeUnit>(schemeitem.itemunit());

		m_acceptClick = schemeitem.acceptclick();

		if (schemeitem.has_clickscript() == true)
		{
			Proto::Read(schemeitem.clickscript(), &m_clickScript);
		}
		else
		{
			m_clickScript.clear();
		}

		return true;
	}

	SchemeItem* SchemeItem::CreateObject(const Proto::Envelope& message)
	{
		// Эта функция может создавать только один экземпляр
		//
		if (message.has_schemeitem() == false)
		{
			assert(message.has_schemeitem());
			return nullptr;
		}

		quint32 classNameHash = message.classnamehash();
		SchemeItem* schemeItem = SchemeItemFactory.Create(classNameHash);

		if (schemeItem == nullptr)
		{
			assert(schemeItem);
			return nullptr;
		}
		
		schemeItem->LoadData(message);

		return schemeItem;
	}

	// Action Functions
	//

	void SchemeItem::MoveItem(double /*horzOffsetDocPt*/, double /*vertOffsetDocPt*/)
	{
		assert(false);	// Implement in child classes
	}

	void SchemeItem::snapToGrid(double /*gridSize*/)
	{
		assert(false);
	}

	double SchemeItem::GetWidthInDocPt() const
	{
		assert(false);	// Implement in child classes
		return 0;
	}

	double SchemeItem::GetHeightInDocPt() const
	{
		assert(false);	// Implement in child classes
		return 0;
	}

	void SchemeItem::SetWidthInDocPt(double /*widthInDocPt*/)
	{
		assert(false);	// Implement in child classes
	}

	void SchemeItem::SetHeightInDocPt(double /*heightInDocPt*/)
	{
		assert(false);	// Implement in child classes
	}

	// Drawing Functions
	//

	// Рисование элемента, выполняется в 100% масштабе.
	// Graphcis должен иметь экранную координатную систему (0, 0 - левый верхний угол, вниз и вправо - положительные координаты)
	//
	void SchemeItem::Draw(CDrawParam*, const Scheme*, const SchemeLayer*) const
	{
	}

	// Рисование элемента при его создании изменении
	//
	void SchemeItem::DrawOutline(CDrawParam* ) const
	{
	}

	void SchemeItem::DrawOutline(CDrawParam* pDrawParam, const std::vector<std::shared_ptr<SchemeItem>>& items)
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

	// Нарисовать выделение объекта, в зависимости от используемого интрефейса расположения.
	//
	void SchemeItem::DrawSelection(CDrawParam*, bool) const
	{
	}

	void SchemeItem::DrawSelection(CDrawParam* pDrawParam, const std::vector<std::shared_ptr<SchemeItem>>& items, bool drawSizeBar)
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

	// Определение, входит ли точка в элемент, x и y в дюймах или в пикселях
	// 
	bool SchemeItem::IsIntersectPoint(double x, double y) const
	{
		if (itemUnit() == SchemeUnit::Display)
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
	bool SchemeItem::IsIntersectRect(double x, double y, double width, double height) const
	{ 
		x = x; y = y; width = width; height = height;		// убираю unreferenced warning
		assert(false);
		return false;
	};

	// ISchemeItemPropertiesPos interface implementation
	//
	double SchemeItem::left() const
	{
		assert(true);
		return 0;
	}
	void SchemeItem::setLeft(double)
	{
		assert(true);
	}

	double SchemeItem::top() const
	{
		assert(false);
		return 0;
	}
	void SchemeItem::setTop(double)
	{
		assert(false);
	}

	double SchemeItem::width() const
	{
		assert(false);
		return 0;
	}
	void SchemeItem::setWidth(double)
	{
		assert(false);
	}

	double SchemeItem::height() const
	{
		assert(false);
		return 0;
	}
	void SchemeItem::setHeight(double)
	{
		assert(false);
	}

	std::vector<SchemePoint> SchemeItem::getPointList() const
	{
		Q_ASSERT(false);
		return std::vector<SchemePoint>();
	}

	void SchemeItem::setPointList(const std::vector<SchemePoint>& /*points*/)
	{
		Q_ASSERT(false);
	}

	// Properties and Data
	//

	bool SchemeItem::IsStatic() const
	{
		return m_static;
	}

	bool SchemeItem::IsDynamic() const
	{
		return !m_static;
	}

	bool SchemeItem::IsFblItem() const
	{
		return false;
	}

	bool SchemeItem::IsLocked() const
	{
		return m_locked;
	}

	void SchemeItem::setLocked(bool lock)
	{
		m_locked = lock;
		return;
	}

	const QUuid& SchemeItem::guid() const
	{
		return m_guid;
	}

	void SchemeItem::setGuid(const QUuid& guid)
	{
		m_guid = guid;
		return;
	}

	void SchemeItem::setNewGuid()
	{
		QUuid uuid = QUuid::createUuid();
		setGuid(uuid);

		return;
	}

	// Единицы измерения, в которых хранятся координаты (может быть только дюймы или точки)
	//
	SchemeUnit SchemeItem::itemUnit() const
	{
		return m_itemUnit;
	}

	void SchemeItem::setItemUnit(SchemeUnit value)
	{
		assert(value == SchemeUnit::Display || value == SchemeUnit::Inch);
		m_itemUnit = value;
	}

	// AcceptClick property
	//
	bool SchemeItem::acceptClick() const
	{
		return m_acceptClick;
	}

	void SchemeItem::setAcceptClick(bool value)
	{
		m_acceptClick = value;
	}

	// ClickScript property
	//
	const QString& SchemeItem::clickScript() const
	{
		return m_clickScript;
	}

	void SchemeItem::setClickScript(const QString& value)
	{
		m_clickScript = value;
	}

	QRectF SchemeItem::boundingRectInDocPt() const
	{
		assert(false);		// Must be implemented in child classes
		return QRectF();
	}

}
