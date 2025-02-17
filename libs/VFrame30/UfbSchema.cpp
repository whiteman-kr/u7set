#include <VFrame30/PropertyNames.h>
#include <VFrame30/SchemaLayer.h>
#include <VFrame30/UfbSchema.h>

namespace VFrame30
{
	UfbSchema::UfbSchema(void)
	{
		setUnit(SchemaUnit::Inch);

		setDocWidth(mm2in(297));
		setDocHeight(mm2in(210));

		ADD_PROPERTY_GETTER_SETTER(QString, "Description", true, UfbSchema::description, UfbSchema::setDescription);
		ADD_PROPERTY_GETTER(int, "Version", true, UfbSchema::version);
		ADD_PROPERTY_GETTER_SETTER(QString,
								   PropertyNames::lmDescriptionFile,
								   true,
								   UfbSchema::lmDescriptionFile,
								   UfbSchema::setLmDescriptionFile);

		ADD_PROPERTY_GETTER_SETTER(QString,
								   PropertyNames::specificProperties,
								   true,
								   UfbSchema::specificProperties,
								   UfbSchema::setSpecificProperties)
			->setSpecificEditor(E::PropertySpecificEditor::SpecificPropertyStruct);

		addLayer(std::make_shared<SchemaLayer>(this, LayerFrameName, false));
		addLayer(std::make_shared<SchemaLayer>(this, LayerLogicName, true));
		addLayer(std::make_shared<SchemaLayer>(this, LayerNotesName, true));

		setTagsList(QStringList{"ufb"});

		return;
	}

	UfbSchema::~UfbSchema(void) {}

	bool UfbSchema::SaveData(Proto::Envelope* message) const
	{
		bool result = Schema::SaveData(message);
		if (result == false || message->HasExtension(Proto::schema) == false)
		{
			assert(result);
			assert(message->HasExtension(Proto::schema));
			return false;
		}

		// --
		//
		auto schemaMessage = message->MutableExtension(Proto::schema);
		Proto::UfbSchema* us = schemaMessage->mutable_ufb_schema();

		us->set_description(m_description.toStdString());

		this->m_version++; // Increment version
		us->set_version(m_version);
		us->set_lmdescriptionfile(m_lmDescriptionFile.toStdString());
		us->set_specific_properties_struct(m_specificPropertiesStruct.toStdString());

		return true;
	}

	bool UfbSchema::LoadData(const Proto::Envelope& message)
	{
		if (message.HasExtension(Proto::schema) == false)
		{
			assert(message.HasExtension(Proto::schema));
			return false;
		}

		// --
		//
		bool result = Schema::LoadData(message);
		if (result == false)
		{
			return false;
		}

		// Add frame Notes if it is not present
		//
		if (const auto& ls = layers(); std::find_if(ls.begin(),
													ls.end(),
													[](const auto& l)
													{
														return l->name() == LayerNotesName;
													}) == ls.end())
		{
			// Add layer.
			//
			addLayer(std::make_shared<SchemaLayer>(this, LayerNotesName, true));
			fixLayerOrder();
		}

		// --
		//
		const auto& schemaMessage = message.GetExtension(Proto::schema);
		if (schemaMessage.has_ufb_schema() == false)
		{
			assert(schemaMessage.has_ufb_schema());
			return false;
		}

		const Proto::UfbSchema& us = schemaMessage.ufb_schema();

		m_description = QString::fromStdString(us.description());
		m_version = us.version();
		m_lmDescriptionFile = QString::fromStdString(us.lmdescriptionfile());
		m_specificPropertiesStruct = QString::fromStdString(us.specific_properties_struct());

		return true;
	}

	void UfbSchema::Draw(CDrawParam* drawParam, const QRectF& clipRect)
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

	QString UfbSchema::specificProperties() const
	{
		return m_specificPropertiesStruct;
	}

	void UfbSchema::setSpecificProperties(QString value)
	{
		m_specificPropertiesStruct = value;
	}
} // namespace VFrame30
