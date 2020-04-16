#pragma once
#include "../../Simulator/Simulator.h"

class SimOverrideValueWidget : public QDialog
{
	Q_OBJECT

private:
	explicit SimOverrideValueWidget(const Sim::OverrideSignalParam& signal, Sim::Simulator* simulator, QWidget* parent);
	virtual ~SimOverrideValueWidget();

public:
	static bool showDialog(const Sim::OverrideSignalParam& signal, Sim::Simulator* simulator, QWidget* parent);

protected:
	virtual void resizeEvent(QResizeEvent* event) override;

protected slots:
	void projectUpdated();
	void overrideSignalsChaged(QStringList appSignalIds);	// Added or deleted signal

public:
	QString appSignalId() const;
	const Sim::OverrideSignalParam& signal() const;

	Sim::Simulator* simulator();
	const Sim::Simulator* simulator() const;

	Sim::OverrideSignals& overrideSignals();
	const Sim::OverrideSignals& overrideSignals() const;

private:
	Sim::OverrideSignalParam m_signal;
	Sim::Simulator* m_simulator = nullptr;

	QSize m_prevVisibleSize;		// On show shit happens, if widget has layout it recalculates it, so we need to keep last size
									// and restore it before showing widget (setVisible(true) itself calls resize)

	static std::map<QString, SimOverrideValueWidget*> m_openedDialogs;	// key is AppSignalID
};


