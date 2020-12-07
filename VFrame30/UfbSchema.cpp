#include "UfbSchema.h"
#include "SchemaItemAfb.h"
#include "SchemaItemSignal.h"
#include "PropertyNames.h"

namespace VFrame30
{
	UfbSchema::UfbSchema(void)
	{
		//qDebug() << "UfbSchema::UfbSchema(void)";

		ADD_PROPERTY_GETTER_SETTER(QString, "Description", true, UfbSchema::description, UfbSchema::setDescription);
		ADD_PROPERTY_GETTER(int, "Version", true, UfbSchema::version);
		ADD_PROPERTY_GETTER_SETTER(QString, PropertyNames::lmDescriptionFile, true, UfbSchema::lmDescriptionFile, UfbSchema::setLmDescriptionFile);

		setUnit(SchemaUnit::Inch);

		setDocWidth(mm2in(297));
		setDocHeight(mm2in(210));

		Layers.push_back(std::make_shared<SchemaLayer>("Logic", true));
		Layers.push_back(std::make_shared<SchemaLayer>("Frame", false));

		setTagsList(QStringList{"ufb"});

		return;
	}

	UfbSchema::~UfbSchema(void)
	{
		//qDebug() << "UfbSchema::~UfbSchema(void)";
	}

	bool UfbSchema::SaveData(Proto::Envelope* message) const
	{
		bool result = Schema::SaveData(message);

		if (result == false || message->has_schema() == false)
		{
			assert(result);
			assert(message->has_schema());
			return false;
		}

		// --
		//
		Proto::UfbSchema* us = message->mutable_schema()->mutable_ufb_schema();

		us->set_description(m_description.toStdString());

		this->m_version++;		// Incerement version
		us->set_version(m_version);
		us->set_lmdescriptionfile(m_lmDescriptionFile.toStdString());

		return true;
	}

	bool UfbSchema::LoadData(const Proto::Envelope& message)
	{
		if (message.has_schema() == false)
		{
			assert(message.has_schema());
			return false;
		}

		// --
		//
		bool result = Schema::LoadData(message);
		if (result == false)
		{
			return false;
		}

		// --
		//
		if (message.schema().has_ufb_schema() == false)
		{
			assert(message.schema().has_ufb_schema());
			return false;
		}

		const Proto::UfbSchema& us = message.schema().ufb_schema();

		m_description = QString::fromStdString(us.description());
		m_version = us.version();
		m_lmDescriptionFile = QString::fromStdString(us.lmdescriptionfile());

		return true;
	}

	void UfbSchema::Draw(CDrawParam* drawParam, const QRectF& clipRect) const
	{
		BuildFblConnectionMap();

		Schema::Draw(drawParam, clipRect);
		return;
    }

	QString UfbSchema::description() const
	{
		return m_description;
	}

	void UfbSchema::setDescription(QString value)
	{
		m_description = value;
	}

	int UfbSchema::version() const
	{
		return m_version;
	}

	QString UfbSchema::lmDescriptionFile() const
	{
		return m_lmDescriptionFile;
	}

	void UfbSchema::setLmDescriptionFile(QString value)
	{
		m_lmDescriptionFile = value;
	}
}
