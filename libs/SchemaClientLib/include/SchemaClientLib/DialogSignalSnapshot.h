#pragma once

#include "../AppSignalLib/IAppSignalManager.h"
#include "../OnlineLib/SoftwareEndpoint.h"
#include "../VFrame30/SchemaDetails.h"
#include "../lib/ISignalDataServer.h"

#include <set>
#include <vector>

#include <QDialog>


namespace SchemaClientLib
{
	class SignalSnapshotWidget;
}

namespace SchemaClientLib
{
	class ISignalSnapshotWidget
	{
	public:
		virtual ~ISignalSnapshotWidget() = default;

		virtual std::vector<VFrame30::SchemaDetails> schemasDetails() = 0;
		virtual std::set<QString> schemaAppSignals(const QString& schemaStrId) = 0;
	};
} // namespace SchemaClientLib


namespace SchemaClientLib
{
	class DialogSignalSnapshot : public QDialog,
								 public SchemaClientLib::ISignalSnapshotWidget
	{
		Q_OBJECT

	protected:
		DialogSignalSnapshot(IAppSignalManager* appSignalManager,
							 ISignalDataServer* signalDataServer,                                  // Can be nullptr, e.g. in Simulator
							 const std::vector<SoftwareEndpoint::AppDataService>& appDataServices, // Can be empty, e.g. in Simulator
							 const QString& projectName,
							 const QString& equipmentId,
							 QWidget* parent);

		DialogSignalSnapshot(IAppSignalManager* appSignalManager, const QString& projectName, const QString& equipmentId, QWidget* parent);

		virtual ~DialogSignalSnapshot();

	protected:
		void showEvent(QShowEvent* event) override;
		void closeEvent(QCloseEvent* event) override;

	public:
		QString projectName() const;
		void setProjectName(const QString& projectName);

		const std::vector<AppSignalParam>& specificSignals() const;
		void setSpecificSignals(const std::vector<AppSignalParam>& specificSignals);

		void setLmEquipmentId(const QString& lmEquipmentId);
		void setSignalsMask(const QStringList& masks);
		void setSignalsTags(const QStringList& tags);
		void resetSignalsType();

	public slots:
		void schemasUpdated();
		void signalsUpdated(); // Should be called when new signals arrived from AppDataService

	protected:
		virtual std::vector<VFrame30::SchemaDetails> schemasDetails() = 0;
		virtual std::set<QString> schemaAppSignals(const QString& schemaStrId) = 0;

	signals:
		void signalContextMenu(const QStringList signalList, const QList<QMenu*>& customMenu);
		void signalInfo(QString appSignalId);

	private:
		SchemaClientLib::SignalSnapshotWidget* m_widget;
	};
} // namespace SchemaClientLib