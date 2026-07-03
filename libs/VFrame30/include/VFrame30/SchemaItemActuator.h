#pragma once

#include <VFrame30/ActuatorHeader.h>
#include <VFrame30/FblItemRect.h>
#include <VFrame30/IMatsSchemaItemAssociations.h>


namespace VFrame30
{
	class ActuatorHeader;
}

namespace VFrame30
{
	//
	// SchemaItemActuator
	//

	/// <summary>
	/// Schema item representing Actuator element.
	/// </summary>
	class SchemaItemActuator final : public FblItemRect,
									 public IMatsSchemaItemAssociations
	{
		Q_OBJECT

		/// \brief ActuatorControlModule EquipmentId, readonly
		Q_PROPERTY(QString acmEquipmentId READ acmEquipmentId)
		Q_PROPERTY(QString AcmEquipmentId READ acmEquipmentId)

		/// \brief User defined caption. If not empty, this caption is used instead of the default one from the ActuatorHeader
		Q_PROPERTY(QString caption READ caption WRITE setCaption)
		Q_PROPERTY(QString Caption READ caption WRITE setCaption)

	public:
		SchemaItemActuator(void);
		explicit SchemaItemActuator(SchemaUnit unit);
		SchemaItemActuator(SchemaUnit unit, const ActuatorHeader& actuatorHeader);

		virtual ~SchemaItemActuator(void);

		// Draw Functions
		//
	public:
		virtual void draw(CDrawParam* drawParam) const override;
		virtual void drawHighlight(CDrawParam* drawParam) const override;

		void drawActuatorItemHelp(QPainter* painter, const QRect& drawRect) const;

		// Serialization
		//
	protected:
		virtual bool SaveData(Proto::Envelope* message) const override;
		virtual bool LoadData(const Proto::Envelope& message) override;

		// Methods
		//
	public:
		virtual QString toolTipText(double dpiX, double dpiY, double devicePixelRatio) const override;
		virtual QString buildName() const override;

		bool updateElement(const ActuatorHeader& actuatorHeader);

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
		[[nodiscard]] QString acmEquipmentId() const;
		void setAcmEquipmentId(const QString& value);

		[[nodiscard]] QString caption() const;
		void setCaption(const QString& value);

		[[nodiscard]] const VFrame30::ActuatorHeader& actuatorHeader() const;
		void setActuatorHeader(const VFrame30::ActuatorHeader& value);

		[[nodiscard]] QString actuatorTypeId() const;
		[[nodiscard]] QString actuatorCaption() const;
		[[nodiscard]] QString actuatorDescription() const;
		[[nodiscard]] int actuatorHeaderVersion() const;

	private:
		QString m_acmEquipmentId = "SYSTEMID_RACKID_CHASSISID_PLACE";
		QString m_caption = "$(item.ActuatorCaption)\\n$(item.AcmEquipmentID)";

		ActuatorHeader m_actuatorHeader;
	};
} // namespace VFrame30
