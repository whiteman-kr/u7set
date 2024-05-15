#pragma once

#include <VFrame30/SchemaManager.h>


namespace SchemaClientLib
{
	class SchemaClientConfigController;

	class ClientSchemaManager : public VFrame30::SchemaManager
	{
		Q_OBJECT

	public:
		explicit ClientSchemaManager(SchemaClientConfigController& configController, QObject* parent = nullptr);

	protected:
		[[nodiscard]] virtual std::shared_ptr<VFrame30::Schema> loadSchema(const QString& schemaId) override;

	public:
		[[nodiscard]] bool hasSchema(const QString& schemaId) const;

		[[nodiscard]] virtual int schemaCount() const override;
		[[nodiscard]] virtual std::shared_ptr<VFrame30::Schema> schemaByIndex(int schemaIndex,
																			  std::shared_ptr<VFrame30::Context> context) override;

		[[nodiscard]] virtual QString schemaCaptionById(const QString& schemaId) const override;
		[[nodiscard]] virtual QString schemaCaptionByIndex(int schemaIndex) const override;
		[[nodiscard]] virtual QString schemaIdByIndex(int schemaIndex) const override;

	public:
		[[nodiscard]] SchemaClientConfigController& configController();
		[[nodiscard]] const SchemaClientConfigController& configController() const;

		// Data
		//
	private:
		SchemaClientConfigController& m_configController;
	};

} // namespace ClientLib