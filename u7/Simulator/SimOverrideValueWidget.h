#pragma once
#include "../../Simulator/Simulator.h"
#include "../../Simulator/SimOverrideSignals.h"
#include "../../QScintilla/Qt4Qt5/Qsci/qsciscintilla.h"
#include "../../lib/QScintillaLexers/LexerJavaScript.h"
#include "../../lib/DbController.h"
#include "../../lib/QDoublevalidatorEx.h"


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
	class OverrideMethodWidget : public QWidget, protected HasDbController
	{
		Q_OBJECT

	public:
		explicit OverrideMethodWidget(const Sim::OverrideSignalParam& signal,
									  Sim::Simulator* simulator,
									  DbController* dbc,
									  QWidget* parent);

	public:
		const Sim::OverrideSignalParam& signal() const;
		Sim::OverrideSignalParam& signal();
		void setSignal(const Sim::OverrideSignalParam& signal);

		virtual void setViewOptions(int base, E::AnalogFormat analogFormat, int precision);

		void setValue(Sim::OverrideSignalMethod method, QVariant value);

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
		ValueMethodWidget(const Sim::OverrideSignalParam& signal,
						  Sim::Simulator* simulator,
						  DbController* dbc,
						  QWidget* parent);

		virtual void setViewOptions(int base, E::AnalogFormat analogFormat, int precision) override;

	protected:
		virtual void showEvent(QShowEvent* e) override;

	protected slots:
		void dialogBoxButtonClicked(QAbstractButton* button);
		void valueEntered(double value);

	private:


	private:
		QWidget* m_edit = nullptr;

		QLineEdit* m_floatEdit = nullptr;
		QDoubleValidatorEx* m_floatEditValidator = nullptr;

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
		ScriptMethodWidget(const Sim::OverrideSignalParam& signal,
						   Sim::Simulator* simulator,
						   DbController* dbc,
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
		QsciScintilla* m_scriptEdit = nullptr;
		LexerJavaScript m_lexer;

		QDialogButtonBox* m_buttonBox = nullptr;

		const std::map<QString, QString> m_templatesAnalog	// Key is template name, value is template filename to load
		{
			{QStringLiteral("Sine"), QStringLiteral(":/Simulator/Templates/Sine.js")},
			{QStringLiteral("Square"), QStringLiteral(":/Simulator/Templates/Square.js")},
			{QStringLiteral("Triangle"), QStringLiteral(":/Simulator/Templates/Triangle.js")},
			{QStringLiteral("Sawtooth Front"), QStringLiteral(":/Simulator/Templates/SawtoothFront.js")},
			{QStringLiteral("Sawtooth Back"), QStringLiteral(":/Simulator/Templates/SawtoothBack.js")},
		};

		const std::map<QString, QString> m_templatesDiscrete	// Key is template name, value is template filename to load
		{
			{QStringLiteral("Square"), QStringLiteral(":/Simulator/Templates/DiscreteSquare.js")},
			{QStringLiteral("Series"), QStringLiteral(":/Simulator/Templates/DiscreteSeries.js")},
		};

		const QString saveProperty = {"Sim::OverrideScript::Saves::"};
	};


	// OverrideValueWidget
	// The Main Windows for override, has some signal info and tab control
	//
	class OverrideValueWidget : public QDialog, protected HasDbController
	{
		Q_OBJECT

	private:
		explicit OverrideValueWidget(const Sim::OverrideSignalParam& signal,
									 Sim::Simulator* simulator,
									 DbController* dbc,
									 QWidget* parent);
		virtual ~OverrideValueWidget();

	public:
		static bool showDialog(const Sim::OverrideSignalParam& signal, Sim::Simulator* simulator, DbController* dbc, QWidget* parent);
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

		ValueMethodWidget* m_valueTabWidget = nullptr;
		ScriptMethodWidget* m_scriptTabWidget = nullptr;

		// --
		//
		QSize m_prevVisibleSize;		// On show shit happens, if widget has layout it recalculates it, so we need to keep last size
		// and restore it before showing widget (setVisible(true) itself calls resize)

		static std::map<QString, OverrideValueWidget*> m_openedDialogs;	// key is AppSignalID
	};

}

