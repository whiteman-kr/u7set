#include <SchemaClientLib/DialogSignalSnapshot.h>

#include "SignalSnapshotWidget.h"

//
// DialogSignalSnapshot
//
namespace SchemaClientLib
{
	DialogSignalSnapshot::DialogSignalSnapshot(IAppSignalManager* appSignalManager,
											   ISignalDataServer* signalDataServer,
											   AppSignalLists::AppSignalListSet* appSignalListSet,
											   const std::vector<SoftwareEndpoint::AppDataService>& appDataServices,
											   const QString& projectName,
											   const QString& equipmentId,
											   QWidget* parent) :
		QDialog{parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowMaximizeButtonHint | Qt::WindowCloseButtonHint}
	{
		setAttribute(Qt::WA_DeleteOnClose, true);

		m_widget = new SchemaClientLib::SignalSnapshotWidget{*this,
															 appSignalManager,
															 signalDataServer,
															 appSignalListSet,
															 appDataServices,
															 projectName,
															 equipmentId,
															 this};

		auto layout = new QVBoxLayout{this};
		layout->setContentsMargins(0, 0, 0, 0);
		layout->addWidget(m_widget);
		setLayout(layout);

		connect(m_widget, &SchemaClientLib::SignalSnapshotWidget::signalContextMenu, this, &DialogSignalSnapshot::signalContextMenu);
		connect(m_widget, &SchemaClientLib::SignalSnapshotWidget::signalInfo, this, &DialogSignalSnapshot::signalInfo);
		return;
	}

	DialogSignalSnapshot::DialogSignalSnapshot(IAppSignalManager* appSignalManager,
											   const QString& projectName,
											   const QString& equipmentId,
											   QWidget* parent) :
		DialogSignalSnapshot{appSignalManager, nullptr, nullptr, {}, projectName, equipmentId, parent}
	{
		return;
	}

	DialogSignalSnapshot::~DialogSignalSnapshot() = default;

	void DialogSignalSnapshot::showEvent(QShowEvent* event)
	{
		QDialog::showEvent(event);

		QSettings settings;

		auto mainWindowPos = settings.value("DialogSignalSnapshot/pos", QPoint(200, 200)).toPoint();
		auto mainWindowGeometry = settings.value("DialogSignalSnapshot/geometry").toByteArray();

		move(mainWindowPos);
		restoreGeometry(mainWindowGeometry);

		return;
	}

	void DialogSignalSnapshot::closeEvent(QCloseEvent* event)
	{
		QDialog::closeEvent(event);

		// Save the window size and position
		//
		QSettings settings;
		settings.setValue("DialogSignalSnapshot/pos", pos());
		settings.setValue("DialogSignalSnapshot/geometry", saveGeometry());

		return;
	}

	QString DialogSignalSnapshot::projectName() const
	{
		return m_widget->projectName();
	}

	void DialogSignalSnapshot::setProjectName(const QString& projectName)
	{
		return m_widget->setProjectName(projectName);
	}

	const std::vector<AppSignalParam>& DialogSignalSnapshot::specificSignals() const
	{
		return m_widget->specificSignals();
	}

	void DialogSignalSnapshot::setSpecificSignals(const std::vector<AppSignalParam>& specificSignals)
	{
		m_widget->setSpecificSignals(specificSignals);
	}

	void DialogSignalSnapshot::setLmEquipmentId(const QString& lmEquipmentId)
	{
		m_widget->setLmEquipmentId(lmEquipmentId);
	}

	void DialogSignalSnapshot::setSignalsMask(const QStringList& masks)
	{
		m_widget->setSignalsMask(masks);
	}

	void DialogSignalSnapshot::setSignalsTags(const QStringList& tags)
	{
		m_widget->setSignalsTags(tags);
	}

	void DialogSignalSnapshot::resetSignalsType()
	{
		m_widget->resetSignalsType();
	}

	void DialogSignalSnapshot::schemasUpdated()
	{
		return m_widget->schemasUpdated();
	}

	void DialogSignalSnapshot::signalsUpdated()
	{
		return m_widget->signalsUpdated();
	}
} // namespace SchemaClientLib