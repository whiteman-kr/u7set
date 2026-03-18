#pragma once

#include <VFrame30/FblItemRect.h>
#include <VFrame30/IMatsSchemaItemAssociations.h>


namespace VFrame30
{
	class UfbSchema;
}


namespace VFrame30
{
	//
	// SchemaItemUfb
	//

	/// <summary>
	/// Schema item representing UFB (User Function Block) element of UFB schema.
	/// </summary>
	class SchemaItemUfb final : public FblItemRect,
								public IMatsSchemaItemAssociations
	{
		Q_OBJECT

		/// \brief User defined caption. If not empty, this caption is used instead of the default one from the UFB schema
		Q_PROPERTY(QString userCaption READ userCaption WRITE setUserCaption)
		Q_PROPERTY(QString UserCaption READ userCaption WRITE setUserCaption)

	public:
		SchemaItemUfb(void);
		explicit SchemaItemUfb(SchemaUnit unit);
		SchemaItemUfb(SchemaUnit unit, const UfbSchema& ufbSchema, QString* errorMsg);

		virtual ~SchemaItemUfb(void);

		// Draw Functions
		//
	public:
		virtual void draw(CDrawParam* drawParam) const override;

		virtual void drawHighlight(CDrawParam* drawParam) const override;

		// Serialization
		//
	protected:
		virtual bool SaveData(Proto::Envelope* message) const override;
		virtual bool LoadData(const Proto::Envelope& message) override;

		// Methods
		//
	public:
		virtual QString buildName() const override;

		bool updateUfbElement(const UfbSchema& ufbSchema, QString* errorMsg);

		// IMatsSchemaItemAssociations implementation.
		//
	public:
		virtual QStringList associatedDiagObjectIds() const override { return {}; };
		virtual QStringList associatedAppSignalIds() const override;
		virtual QStringList associatedImpactAppSignalIds() const override;
		virtual QStringList associatedConnectionIds() const override;
		virtual QStringList associatedLoopbackIds() const override;
		virtual QStringList associatedSchemaItemLabels() const override;

		// Properties and Data
		//
	public:
		[[nodiscard]] QString userCaption() const;
		void setUserCaption(const QString& value);

		[[nodiscard]] QString ufbSchemaId() const;
		[[nodiscard]] QString ufbCaption() const;
		[[nodiscard]] int ufbSchemaVersion() const;

		[[nodiscard]] QString specificProperties() const;

	private:
		void setSpecificProperties(QString value);

	private:
		QString m_userCaption;              // If not empty, this caption is used instead of the default one from the UFB schema

		QString m_ufbSchemaId;
		QString m_ufbCaption;
		int m_ufbVersion = -1;

		QString m_specificPropertiesStruct; // Description of the UFB's specific properties
	};
} // namespace VFrame30
