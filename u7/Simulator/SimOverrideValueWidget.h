#pragma once
#include "../../Simulator/Simulator.h"


namespace SimOverrideUI
{

	class DiscreteSpinBox : public QSpinBox
	{
		Q_OBJECT

	public:
		DiscreteSpinBox(int value, QWidget* parent);

	protected:
		virtual void keyPressEvent(QKeyEvent* event) override;
	};


	class SInt32SpinBox : public QSpinBox
	{
		Q_OBJECT

	public:
		SInt32SpinBox(qint32 value, QWidget* parent);
	};


	//
	// OverrideMethodWidget
	//
	class OverrideMethodWidget : public QWidget
	{
		Q_OBJECT

	public:
		explicit OverrideMethodWidget(const Sim::OverrideSignalParam& signal, Sim::Simulator* simulator, QWidget* parent);

	public:
		const Sim::OverrideSignalParam& signal() const;
		Sim::OverrideSignalParam& signal();
		void setSignal(const Sim::OverrideSignalParam& signal);

		virtual void setViewOptions(int base, E::AnalogFormat analogFormat, int precision);

		void setValue(QVariant value);

	protected:
		Sim::OverrideSignalParam m_signal;
		Sim::Simulator* m_simulator = nullptr;

		int m_currentBase = 10;											// Base for integer signals: 10, 16
		E::AnalogFormat m_analogFormat = E::AnalogFormat::g_9_or_9e;	// Current format for floating point signals
		int m_precision = -1;											// Current procision for floating point signals
	};

	//
	// ValueMethodWidget
	//
	class ValueMethodWidget : public OverrideMethodWidget
	{
		Q_OBJECT

	public:
		ValueMethodWidget(const Sim::OverrideSignalParam& signal, Sim::Simulator* simulator, QWidget* parent);

		virtual void setViewOptions(int base, E::AnalogFormat analogFormat, int precision) override;

	protected slots:
		void dialogBoxButtonClicked(QAbstractButton* button);
		void valueEntered(double value);

	private:


	private:
		//QLineEdit* m_edit = nullptr;

		QWidget* m_edit = nullptr;

		QLineEdit* m_floatEdit = nullptr;
		QDoubleValidator* m_floatEditValidator = nullptr;

		QDoubleSpinBox* m_doubleSpinBox = nullptr;
		QSpinBox* m_intSpinBox = nullptr;
		QSpinBox* m_discreteSpinBox = nullptr;

		QDialogButtonBox* m_buttonBox = nullptr;
	};


	//
	// ScriptMethodWidget
	//
	class ScriptMethodWidget : public OverrideMethodWidget
	{
		Q_OBJECT

	public:
		ScriptMethodWidget(const Sim::OverrideSignalParam& signal, Sim::Simulator* simulator, QWidget* parent);

	};


	//
	// OverrideValueWidget
	//
	class OverrideValueWidget : public QDialog
	{
		Q_OBJECT

	private:
		explicit OverrideValueWidget(const Sim::OverrideSignalParam& signal, Sim::Simulator* simulator, QWidget* parent);
		virtual ~OverrideValueWidget();

	public:
		static bool showDialog(const Sim::OverrideSignalParam& signal, Sim::Simulator* simulator, QWidget* parent);
		static void setViewOptions(QString appSignalId, int base, E::AnalogFormat analogFormat, int precision);

	protected:
		virtual void resizeEvent(QResizeEvent* event) override;

	protected slots:
		void projectUpdated();
		void overrideSignalsChaged(QStringList appSignalIds);	// Added or deleted signal

		void updateSignalsUi();

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

		// UI
		//
		QLabel* m_customSiganIdLabel = nullptr;
		QLabel* m_appSiganIdLabel = nullptr;
		QLabel* m_captionLabel = nullptr;
		QLabel* m_typeLabel = nullptr;

		QTabWidget* m_tabWidget = new QTabWidget{this};

		ValueMethodWidget* m_valueWidget = new ValueMethodWidget{m_signal, m_simulator, nullptr};
		ScriptMethodWidget* m_scriptWidget = new ScriptMethodWidget{m_signal, m_simulator, nullptr};

		// --
		//
		QSize m_prevVisibleSize;		// On show shit happens, if widget has layout it recalculates it, so we need to keep last size
		// and restore it before showing widget (setVisible(true) itself calls resize)

		static std::map<QString, OverrideValueWidget*> m_openedDialogs;	// key is AppSignalID
	};

}

