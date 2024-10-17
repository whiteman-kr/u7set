#pragma once
#include <Simulator/SimOverrideSignals.h>
#include <Simulator/Simulator.h>
#include <SimulatorUi/ISimPropertyStorage.h>
#include <UiLib/DoubleValidatorEx.h>

#include <QSpinBox>

class QAbstractButton;
class QDialogButtonBox;
class QLabel;
class QTabWidget;

namespace UiLib
{
	class CodeEditor;
}

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

	protected:
		void keyPressEvent(QKeyEvent* event) override;

	signals:
		void returnPressed();
	};


	//
	// OverrideMethodWidget
	//
	class OverrideMethodWidget : public QWidget
	{
		Q_OBJECT

	public:
		explicit OverrideMethodWidget(const std::vector<Sim::OverrideSignalParam>& signalss,
									  Sim::Simulator& simulator,
									  SimUi::ISimPropertyStorage& propertyStorage,
									  QWidget* parent);

	public:
		const std::vector<Sim::OverrideSignalParam>& signalss() const;
		std::vector<Sim::OverrideSignalParam>& signalss();
		void setSignals(const std::vector<Sim::OverrideSignalParam>& signalss);

		virtual void setViewOptions(int base, E::AnalogFormat analogFormat, int precision);

		void setValue(Sim::OverrideSignalMethod method, QVariant value);

	protected:
		SimUi::ISimPropertyStorage& m_propertyStorage;

		std::vector<Sim::OverrideSignalParam> m_signals;
		Sim::Simulator& m_simulator;

		int m_currentBase = 10;                                      // Base for integer signals: 10, 16
		E::AnalogFormat m_analogFormat = E::AnalogFormat::g_9_or_9e; // Current format for floating point signals
		int m_precision = -1;                                        // Current procision for floating point signals
	};


	//
	// ValueMethodWidget
	//
	class ValueMethodWidget : public OverrideMethodWidget
	{
		Q_OBJECT

	public:
		ValueMethodWidget(const std::vector<Sim::OverrideSignalParam>& signalss,
						  Sim::Simulator& simulator,
						  SimUi::ISimPropertyStorage& propertyStorage,
						  QWidget* parent);

		virtual void setViewOptions(int base, E::AnalogFormat analogFormat, int precision) override;

	protected:
		virtual void showEvent(QShowEvent* e) override;

	protected slots:
		void dialogBoxButtonClicked(QAbstractButton* button);
		void valueEntered(double value);

	private:
		QWidget* m_edit = nullptr;

		QLineEdit* m_floatEdit = nullptr;
		QDoubleValidatorEx* m_floatEditValidator = nullptr;

		QDoubleSpinBox* m_doubleSpinBox = nullptr;
		SInt32SpinBox* m_intSpinBox = nullptr;
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
		ScriptMethodWidget(const std::vector<Sim::OverrideSignalParam>& signalss,
						   Sim::Simulator& simulator,
						   SimUi::ISimPropertyStorage& propertyStorage,
						   QWidget* parent);

	protected slots:
		void dialogBoxButtonClicked(QAbstractButton* button);

		void applyScript();
		void showTemplates();
		void loadScript();
		void saveScript();

	private:
		QPushButton* m_templateScriptButton = nullptr;
		QPushButton* m_loadScriptButton = nullptr;
		QPushButton* m_saveScriptButton = nullptr;

		QLabel* m_scriptLabel = nullptr;
		UiLib::CodeEditor* m_scriptEdit = nullptr;

		QDialogButtonBox* m_buttonBox = nullptr;

		const std::map<QString, QString> m_templatesAnalog // Key is template name, value is template filename to load
			{
				{QStringLiteral("Sine"), QStringLiteral(":/Simulator/Templates/Sine.js")},
				{QStringLiteral("Square"), QStringLiteral(":/Simulator/Templates/Square.js")},
				{QStringLiteral("Triangle"), QStringLiteral(":/Simulator/Templates/Triangle.js")},
				{QStringLiteral("Sawtooth Front"), QStringLiteral(":/Simulator/Templates/SawtoothFront.js")},
				{QStringLiteral("Sawtooth Back"), QStringLiteral(":/Simulator/Templates/SawtoothBack.js")},
			};

		const std::map<QString, QString> m_templatesDiscrete // Key is template name, value is template filename to load
			{
				{QStringLiteral("Square"), QStringLiteral(":/Simulator/Templates/DiscreteSquare.js")},
				{QStringLiteral("Series"), QStringLiteral(":/Simulator/Templates/DiscreteSeries.js")},
			};
	};


	// OverrideDialog
	// The Main Windows for override, has some signal info and tab control
	//
	class OverrideDialog : public QDialog
	{
		Q_OBJECT

	private:
		explicit OverrideDialog(const std::vector<Sim::OverrideSignalParam>& overrideSignals,
								Sim::Simulator& simulator,
								SimUi::ISimPropertyStorage& propertyStorage,
								QWidget* parent);
		virtual ~OverrideDialog();

	public:
		static bool showDialog(std::vector<Sim::OverrideSignalParam> overrideSignals,
							   Sim::Simulator& simulator,
							   SimUi::ISimPropertyStorage& propertyStorage,
							   QWidget* parent);
		static void setViewOptions(const QStringList& appSignalIds, int base, E::AnalogFormat analogFormat, int precision);

	protected:
		virtual void resizeEvent(QResizeEvent* event) override;

	protected slots:
		void projectUpdated();
		void overrideSignalsChaged(QStringList appSignalIds); // Added or deleted signal

		void updateSignalsUi();

	public:
		QStringList appSignalIds() const;

		Sim::Simulator* simulator();
		const Sim::Simulator* simulator() const;

		Sim::OverrideSignals& overrideSignals();
		const Sim::OverrideSignals& overrideSignals() const;

	private:
		SimUi::ISimPropertyStorage& m_propertyStorage;

		// All signals in m_overrideSignals must have the same type and data format.
		//
		std::vector<Sim::OverrideSignalParam> m_overrideSignals;

		Sim::Simulator& m_simulator;

		// UI
		//
		QLabel* m_customSiganIdLabel = nullptr;
		QLabel* m_appSiganIdLabel = nullptr;
		QLabel* m_captionLabel = nullptr;
		QLabel* m_typeLabel = nullptr;

		QTabWidget* m_tabWidget = nullptr;

		ValueMethodWidget* m_valueTabWidget = nullptr;
		ScriptMethodWidget* m_scriptTabWidget = nullptr;

		// --
		//
		QSize m_prevVisibleSize; // On show shit happens, if widget has layout it recalculates it, so we need to keep last size
								 // and restore it before showing widget (setVisible(true) itself calls resize)

		static std::map<QString, OverrideDialog*> s_openedDialogs; // key is AppSignalID
	};

} // namespace SimOverrideUI
