#include "SimOverrideValueWidget.h"
#include <UiLib/CodeEditor.h>

#include <QApplication>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QFormLayout>
#include <QInputDialog>
#include <QJSEngine>
#include <QLineEdit>
#include <QTabWidget>


namespace
{
	static const QString kSavePropertyPrefix{"Sim::OverrideScript::"};
};


namespace SimOverrideUI
{
	DiscreteSpinBox::DiscreteSpinBox(int value, QWidget* parent) :
		QSpinBox{parent}
	{
		setAccelerated(false);
		setRange(0, 1);
		setAlignment(Qt::AlignRight);
		setWrapping(true);

		setValue(value);

		return;
	}

	void DiscreteSpinBox::keyPressEvent(QKeyEvent* event)
	{
		if (event->text() == "0")
		{
			setValue(0);
			event->accept();
			return;
		}

		if (event->text() == "1")
		{
			setValue(1);
			event->accept();
			return;
		}

		return QSpinBox::keyPressEvent(event);
	}

	SInt32SpinBox::SInt32SpinBox(qint32 value, QWidget* parent) :
		QSpinBox(parent)
	{
		setRange(std::numeric_limits<qint32>::lowest(), std::numeric_limits<qint32>::max());
		setWrapping(true);
		setAccelerated(false); // on trends it does not look well
		setAlignment(Qt::AlignRight);
		setKeyboardTracking(false);
		setGroupSeparatorShown(true);

		lineEdit()->setInputMethodHints(lineEdit()->inputMethodHints() | Qt::ImhUppercaseOnly);

		setValue(value);

		return;
	}

	void SInt32SpinBox::keyPressEvent(QKeyEvent* event)
	{
		if (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return)
		{
			int pos = 0;
			QString text = this->cleanText();

			if (this->validate(text, pos) == QValidator::Acceptable)
			{
				this->setValue(valueFromText(text));
				this->selectAll();
				emit returnPressed();

				return;
			}
		}

		return QSpinBox::keyPressEvent(event);
	}


	//
	// OverrideMethodWidget
	//
	OverrideMethodWidget::OverrideMethodWidget(const std::vector<Sim::OverrideSignalParam>& signalss,
											   Sim::Simulator& simulator,
											   SimUi::ISimPropertyStorage& propertyStorage,
											   QWidget* parent) :
		QWidget{parent},
		m_propertyStorage{propertyStorage},
		m_signals{signalss},
		m_simulator{simulator}
	{
		if (m_signals.empty() == true)
		{
			Q_ASSERT(m_signals.empty() == false);
			return;
		}

		return;
	}

	const std::vector<Sim::OverrideSignalParam>& OverrideMethodWidget::signalss() const
	{
		return m_signals;
	}

	std::vector<Sim::OverrideSignalParam>& OverrideMethodWidget::signalss()
	{
		return m_signals;
	}

	void OverrideMethodWidget::setSignals(const std::vector<Sim::OverrideSignalParam>& signalss)
	{
		m_signals = signalss;
	}

	void OverrideMethodWidget::setViewOptions(int base, E::AnalogFormat analogFormat, int precision)
	{
		m_currentBase = base;
		m_analogFormat = analogFormat;
		m_precision = precision;

		return;
	}

	void OverrideMethodWidget::setValue(Sim::OverrideSignalMethod method, QVariant value)
	{
		if (m_signals.empty() == true)
		{
			Q_ASSERT(m_signals.empty() == false);
			return;
		}

		std::vector<Sim::OverrideSetValueData> overrideData;
		overrideData.reserve(m_signals.size());

		for (const auto& signal : m_signals)
		{
			overrideData.emplace_back(signal.appSignalId(), method, value);

			switch (method)
			{
			case Sim::OverrideSignalMethod::Value:
				m_propertyStorage.saveProperty(QString("Sim::OverrideMethod::%1").arg(signal.appSignalId()), QStringLiteral("Value"));
				break;
			case Sim::OverrideSignalMethod::Script:
				m_propertyStorage.saveProperty(QString("Sim::OverrideMethod::%1").arg(signal.appSignalId()), QStringLiteral("Script"));

				m_propertyStorage.saveProperty(QString("Sim::OverrideScript::%1").arg(signal.appSignalId()), value.toString());
				break;
			}
		}

		m_simulator.overrideSignals().setValues(overrideData);

		return;
	}

	//
	// ValueMethodWidget
	//
	ValueMethodWidget::ValueMethodWidget(const std::vector<Sim::OverrideSignalParam>& signalss,
										 Sim::Simulator& simulator,
										 SimUi::ISimPropertyStorage& propertyStorage,
										 QWidget* parent) :
		OverrideMethodWidget(signalss, simulator, propertyStorage, parent)
	{
		if (m_signals.empty() == true)
		{
			Q_ASSERT(m_signals.empty() == false);
			return;
		}

		const auto& firstSignal = m_signals.front();

		switch (firstSignal.signalType())
		{
		case E::SignalType::Analog:
			{
				switch (firstSignal.dataFormat())
				{
				case E::AnalogAppSignalFormat::SignedInt32:
					{
						m_intSpinBox = new SInt32SpinBox{firstSignal.value().toInt(), this};
						m_edit = m_intSpinBox;

						m_intSpinBox->setDisplayIntegerBase(m_currentBase);
						m_intSpinBox->setSuffix(m_currentBase == 16 ? " hex" : "");

						connect(m_intSpinBox,
								&SInt32SpinBox::returnPressed,
								[this]()
								{
									valueEntered(m_intSpinBox->value());
								});
					}
					break;
				case E::AnalogAppSignalFormat::Float32:
					{
						m_floatEdit = new QLineEdit{this};
						m_floatEdit->setAlignment(Qt::AlignRight);
						m_floatEdit->setText(firstSignal.valueString(m_currentBase, m_analogFormat, m_precision));

						m_floatEditValidator = new QDoubleValidatorEx{false, this};
						m_floatEditValidator->setDecimals(m_precision);

						m_floatEdit->setValidator(m_floatEditValidator);

						m_edit = m_floatEdit;

						connect(m_floatEdit,
								&QLineEdit::returnPressed,
								[this]()
								{
									bool ok = false;

									float v = QLocale{}.toFloat(m_floatEdit->text(), &ok);
									if (ok == false)
									{
										v = m_floatEdit->text().toFloat(&ok);
									}

									setValue(Sim::OverrideSignalMethod::Value, QVariant::fromValue<float>(v));
								});
					}
					break;
				default:
					assert(false);
					return;
				}
			}
			break;

		case E::SignalType::Discrete:
			{
				m_discreteSpinBox = new DiscreteSpinBox{firstSignal.value().toInt(), this};
				m_edit = m_discreteSpinBox;

				connect(m_discreteSpinBox,
						QOverload<int>::of(&QSpinBox::valueChanged),
						[this, dsp = m_discreteSpinBox](int value)
						{
							valueEntered(value);
							dsp->setValue(value);
							dsp->selectAll();
						});
			}
			break;

		case E::SignalType::Bus:
			break;

		default:
			assert(false);
		}

		// Apply button
		//
		m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Apply, this);

		// --
		//
		QFormLayout* formLayout = new QFormLayout;

		if (m_edit != nullptr)
		{
			formLayout->addRow(tr("&Value:"), m_edit);
			formLayout->addRow(m_buttonBox);
		}

		setLayout(formLayout);

		// --
		//
		connect(m_buttonBox, &QDialogButtonBox::clicked, this, &ValueMethodWidget::dialogBoxButtonClicked);

		return;
	}

	void ValueMethodWidget::setViewOptions(int base, E::AnalogFormat analogFormat, int precision)
	{
		OverrideMethodWidget::setViewOptions(base, analogFormat, precision);

		if (m_doubleSpinBox != nullptr)
		{
			m_doubleSpinBox->setDecimals(precision);
		}

		if (m_intSpinBox != nullptr)
		{
			m_intSpinBox->setDisplayIntegerBase(base);
			m_intSpinBox->setSuffix(m_currentBase == 16 ? " hex" : "");
		}

		if (m_floatEditValidator != nullptr)
		{
			m_floatEditValidator->setDecimals(precision);
		}

		return;
	}

	void ValueMethodWidget::showEvent(QShowEvent* e)
	{
		if (m_edit != nullptr)
		{
			m_edit->setFocus();
		}

		return OverrideMethodWidget::showEvent(e);
	}

	void ValueMethodWidget::dialogBoxButtonClicked(QAbstractButton* button)
	{
		if (button == m_buttonBox->button(QDialogButtonBox::Apply))
		{
			if (m_discreteSpinBox != nullptr)
			{
				valueEntered(m_discreteSpinBox->value());
				return;
			}

			if (m_intSpinBox != nullptr)
			{
				valueEntered(m_intSpinBox->value());
				return;
			}

			if (m_doubleSpinBox != nullptr)
			{
				valueEntered(m_doubleSpinBox->value());
				return;
			}

			if (m_floatEdit != nullptr)
			{
				m_floatEdit->returnPressed();
			}
		}
	}

	void ValueMethodWidget::valueEntered(double value)
	{
		if (m_signals.empty() == true)
		{
			Q_ASSERT(m_signals.empty() == false);
			return;
		}

		const auto& firstSignal = m_signals.front();

		// Set this value to override signals
		//
		QVariant newValue;

		switch (firstSignal.signalType())
		{
		case E::SignalType::Discrete:
			newValue = std::clamp(static_cast<int>(value), 0, 1);
			break;

		case E::SignalType::Analog:
			newValue = value;
			break;

		default:
			assert(false);
			return;
		}

		if (newValue.isValid() == true)
		{
			setValue(Sim::OverrideSignalMethod::Value, newValue);
		}
		else
		{
			assert(newValue.isValid());
		}

		return;
	}


	//
	// ScriptMethodWidget
	//
	ScriptMethodWidget::ScriptMethodWidget(const std::vector<Sim::OverrideSignalParam>& signalss,
										   Sim::Simulator& simulator,
										   SimUi::ISimPropertyStorage& propertyStorage,
										   QWidget* parent) :
		OverrideMethodWidget(signalss, simulator, propertyStorage, parent)
	{
		if (m_signals.empty() == true)
		{
			Q_ASSERT(m_signals.empty() == false);
			return;
		}

		const auto& firstSignal = m_signals.front();

		m_scriptLabel = new QLabel(tr("Override Value Script:"));

		m_scriptEdit = new UiLib::CodeEditor(this);
#if defined(Q_OS_WIN)
		QFont f = QFont("Consolas");
#else
		QFont f = QFont("Courier");
#endif
		m_scriptEdit->setFont(f);

		UiLib::JsHighlighter::createJsHighlighter(m_scriptEdit);

		QString lastScript = m_propertyStorage.loadProperty(QString("Sim::OverrideScript::%1").arg(firstSignal.appSignalId()));

		if (lastScript.isEmpty() == false)
		{
			m_scriptEdit->setText(lastScript);
		}
		else
		{
			m_scriptEdit->setText(firstSignal.script());
		}

		// Apply button
		//
		m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Apply, this);

		m_templateScriptButton = m_buttonBox->addButton(tr("Templates..."), QDialogButtonBox::ButtonRole::ResetRole);
		m_loadScriptButton = m_buttonBox->addButton(tr("Load..."), QDialogButtonBox::ButtonRole::ResetRole);
		m_saveScriptButton = m_buttonBox->addButton(tr("Save..."), QDialogButtonBox::ButtonRole::ResetRole);

		// --
		//
		QGridLayout* gridLayout = new QGridLayout;

		gridLayout->addWidget(m_scriptLabel, 0, 0);
		gridLayout->addWidget(m_scriptEdit, 1, 0);

		gridLayout->addWidget(m_buttonBox, 2, 0);

		setLayout(gridLayout);

		// --
		//
		connect(m_buttonBox, &QDialogButtonBox::clicked, this, &ScriptMethodWidget::dialogBoxButtonClicked);

		return;
	}

	void ScriptMethodWidget::dialogBoxButtonClicked(QAbstractButton* button)
	{
		if (button == m_buttonBox->button(QDialogButtonBox::Apply))
		{
			applyScript();
		}

		if (button == m_templateScriptButton)
		{
			showTemplates();
		}

		if (button == m_loadScriptButton)
		{
			loadScript();
		}

		if (button == m_saveScriptButton)
		{
			saveScript();
		}

		return;
	}

	void ScriptMethodWidget::applyScript()
	{
		// Validate script here
		//
		QString script = m_scriptEdit->text();

		QJSEngine jsEngine;
		QJSValue scriptValue = jsEngine.evaluate(script);

		if (scriptValue.isError() == true)
		{
			qDebug() << "Script evaluate error at line " << scriptValue.property("lineNumber").toInt();
			qDebug() << "\tClass: " << metaObject()->className();
			qDebug() << "\tStack: " << scriptValue.property("stack").toString();
			qDebug() << "\tMessage: " << scriptValue.toString();

			QMessageBox::critical(
				this,
				QApplication::applicationDisplayName(),
				tr("Script evaluate error at line %1:\n%2").arg(scriptValue.property("lineNumber").toInt()).arg(scriptValue.toString()));
			return;
		}

		// Set script to signal
		//
		setValue(Sim::OverrideSignalMethod::Script, script);

		return;
	}

	void ScriptMethodWidget::showTemplates()
	{
		if (m_signals.empty() == true)
		{
			Q_ASSERT(m_signals.empty() == false);
			return;
		}

		const auto& firstSignal = m_signals.front();

		QMenu m(tr("Script Templates:"));

		const std::map<QString, QString>* templates = nullptr;

		switch (firstSignal.signalType())
		{
		case E::SignalType::Analog:
			templates = &m_templatesAnalog;
			break;
		case E::SignalType::Discrete:
			templates = &m_templatesDiscrete;
			break;
		default:
			return;
		}

		assert(templates);
		for (auto [name, fileName] : *templates)
		{
			QAction* a = m.addAction(name);
			a->setData(name);
		}

		QAction* a = m.exec(m_templateScriptButton->mapToGlobal(m_templateScriptButton->geometry().bottomLeft()));

		if (a != nullptr)
		{
			QString templateName = a->data().toString();


			if (templates->find(templateName) == templates->end())
			{
				assert(templates->find(templateName) != templates->end());
			}
			else
			{
				QString templateFileName = templates->at(templateName);

				auto loadTemplateFunc = [this](QString templateFileName)
				{
					QFile f(templateFileName);

					if (f.open(QIODevice::ReadOnly) == false)
					{
						this->m_scriptEdit->setText(tr("Cannot open file %1").arg(templateFileName));
					}
					else
					{
						QString script = QString{f.readAll()};
						this->m_scriptEdit->setText(script);

						this->m_scriptEdit->setModified(false);
					}
				};


				if (m_scriptEdit->isModified() == true)
				{
					QMessageBox mb(this);
					mb.setText(tr("The document has been modified."));
					mb.setInformativeText(tr("Do you want to overwrite it?"));
					mb.setStandardButtons(QMessageBox::Discard | QMessageBox::Yes);
					mb.setDefaultButton(QMessageBox::Discard);

					int result = mb.exec();

					switch (result)
					{
					case QMessageBox::Discard:
						break;
					case QMessageBox::Yes:
						loadTemplateFunc(templateFileName);
						return;
					default:
						Q_ASSERT(false);
					}
				}
				else
				{
					loadTemplateFunc(templateFileName);
				}
			}
		}

		return;
	}

	void ScriptMethodWidget::loadScript()
	{
		QMenu m;

		QStringList savedProperties =
			m_propertyStorage.getPropertyNames().filter(kSavePropertyPrefix).replaceInStrings(kSavePropertyPrefix, "");

		for (const QString& s : savedProperties)
		{
			auto loadScriptFunc = [s, this]()
			{
				int r = QMessageBox::question(this,
											  qAppName(),
											  tr("All current changes will be lost."),
											  QMessageBox::StandardButton::Cancel | QMessageBox::Default | QMessageBox::Escape,
											  QMessageBox::StandardButton::Ok);

				if (r != QMessageBox::StandardButton::Ok)
				{
					return;
				}

				QString script = m_propertyStorage.loadProperty(kSavePropertyPrefix + s);
				m_scriptEdit->setText(script);
			};

			m.addAction(s, loadScriptFunc);
		}

		if (savedProperties.isEmpty() == false)
		{
			m.addSeparator();
		}

		auto loadFromFileFunc = [this]()
		{
			static QString path{"."};
			QString fileName = QFileDialog::getOpenFileName(this, "", path, tr("Scripts (*.js *.script);;All Files(*.*)"));
			if (fileName.isEmpty() == true)
			{
				return;
			}
			path = QFileInfo(fileName).path(); // store path for next time

			int r = QMessageBox::question(this,
										  qAppName(),
										  tr("All current changes will be lost."),
										  QMessageBox::StandardButton::Cancel | QMessageBox::Default | QMessageBox::Escape,
										  QMessageBox::StandardButton::Ok);

			if (r != QMessageBox::StandardButton::Ok)
			{
				return;
			}

			QFile file(fileName);
			if (file.open(QIODevice::ReadOnly | QIODevice::Text) == false)
			{
				return;
			}

			QString script = file.readAll();
			m_scriptEdit->setText(script);
		};

		m.addAction(tr("Load from File..."), loadFromFileFunc);

		m.exec(QCursor::pos());

		return;
	}

	void ScriptMethodWidget::saveScript()
	{
		QMenu m;

		QStringList savesProperties =
			m_propertyStorage.getPropertyNames().filter(kSavePropertyPrefix).replaceInStrings(kSavePropertyPrefix, "");

		for (const QString& s : savesProperties)
		{
			auto saveFunc = [savePropertyName = s, this]()
			{
				int r = QMessageBox::question(this,
											  qAppName(),
											  tr("Record %1 already exists. Do you want to overwrite it?").arg(savePropertyName),
											  QMessageBox::StandardButton::No | QMessageBox::Default | QMessageBox::Escape,
											  QMessageBox::StandardButton::Yes);

				if (r == QMessageBox::StandardButton::No)
				{
					return;
				}

				m_propertyStorage.saveProperty(kSavePropertyPrefix + savePropertyName, this->m_scriptEdit->text());
			};

			m.addAction(s, saveFunc);
		}

		m.addAction(tr("Save as..."),
					[this, &savesProperties]()
					{
						static int rn = 1;
						bool ok = false;
						QString text = QInputDialog::getText(this,
															 qAppName(),
															 tr("Name:"),
															 QLineEdit::EchoMode::Normal,
															 QString("New Record %1").arg(savesProperties.size() + (rn++)),
															 &ok);

						if (ok == false || text.isEmpty() == true)
						{
							return;
						}

						if (savesProperties.contains(text) == true)
						{
							int r = QMessageBox::question(this,
														  qAppName(),
														  tr("Record %1 already exists. Do you want to overwrite it?").arg(text),
														  QMessageBox::StandardButton::No | QMessageBox::Default | QMessageBox::Escape,
														  QMessageBox::StandardButton::Yes);

							if (r == QMessageBox::StandardButton::No)
							{
								return;
							}
						}

						m_propertyStorage.saveProperty(kSavePropertyPrefix + text, this->m_scriptEdit->text());
					});

		if (savesProperties.isEmpty() == false)
		{
			m.addSeparator();
			QMenu* removeMenu = m.addMenu(tr("Remove Saves"));

			for (const QString& s : savesProperties)
			{
				auto removeSaveFunc = [this, s]()
				{
					int r = QMessageBox::question(this,
												  qAppName(),
												  tr("Record %1 will be removed.").arg(s),
												  QMessageBox::StandardButton::Cancel | QMessageBox::Default | QMessageBox::Escape,
												  QMessageBox::StandardButton::Ok);

					if (r == QMessageBox::StandardButton::Ok)
					{
						m_propertyStorage.removeProperty(kSavePropertyPrefix + s);
					}
				};

				removeMenu->addAction(tr("Remove %1").arg(s), removeSaveFunc);
			}
		}

		m.addSeparator();

		m.addAction(tr("Save to File..."),
					[this]()
					{
						static QString path{"."};
						QString fileName =
							QFileDialog::getSaveFileName(this, "", path + QDir::separator(), tr("Scripts (*.js *.script);;All Files(*.*)"));
						if (fileName.isEmpty() == false)
						{
							path = QFileInfo(fileName).path(); // store path for next time
							QFile file(fileName);
							if (file.open(QIODevice::WriteOnly | QIODevice::Text) == false)
							{
								return;
							}

							QTextStream out(&file);
							out << this->m_scriptEdit->text();
						}
					});

		m.exec(QCursor::pos());

		return;
	}


	//
	// OverrideValueWidget
	//
	std::map<QString, OverrideDialog*> OverrideDialog::s_openedDialogs;


	OverrideDialog::OverrideDialog(const std::vector<Sim::OverrideSignalParam>& overrideSignals,
								   Sim::Simulator& simulator,
								   SimUi::ISimPropertyStorage& propertyStorage,
								   QWidget* parent) :
		QDialog{parent},
		m_propertyStorage{propertyStorage},
		m_overrideSignals{overrideSignals},
		m_simulator{simulator}
	{
		setAttribute(Qt::WA_DeleteOnClose);
		setWindowFlag(Qt::WindowContextHelpButtonHint, false);
		setWindowFlag(Qt::WindowMinimizeButtonHint, false);
		setWindowFlag(Qt::WindowMaximizeButtonHint, false);

		// --
		//
		if (m_overrideSignals.empty() == true)
		{
			Q_ASSERT(m_overrideSignals.empty() == false);
			return;
		}

		// --
		//
		if (m_overrideSignals.size() == 1)
		{
			setWindowTitle(tr("Override %1").arg(m_overrideSignals.front().appSignalId()));
		}
		else
		{
			setWindowTitle(
				tr("Override %1, ... %2 more signal(s)").arg(m_overrideSignals.front().appSignalId()).arg(m_overrideSignals.size() - 1));
		}

		// Check that all signals have the same type and data format.
		//
		const auto& firstSignal = m_overrideSignals.front();

		for (const auto& signal : m_overrideSignals)
		{
			if (firstSignal.sameType(signal) == false)
			{
				Q_ASSERT(firstSignal.sameType(signal));
				return;
			}
		}

		// --
		//


		// CustomSignalID/AppSignalID
		//
		m_customSiganIdLabel = new QLabel(this);
		m_customSiganIdLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
		m_customSiganIdLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);

		m_appSiganIdLabel = new QLabel(this);
		m_appSiganIdLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
		m_appSiganIdLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);

		// Caption
		//
		m_captionLabel = new QLabel(this);
		m_captionLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
		m_captionLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);

		// Type/Format
		//
		m_typeLabel = new QLabel(this);
		m_typeLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
		m_typeLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);

		updateSignalsUi();

		// Tab Widget
		//
		m_tabWidget = new QTabWidget{this};

		m_valueTabWidget = new ValueMethodWidget{m_overrideSignals, m_simulator, m_propertyStorage, nullptr};
		m_scriptTabWidget = new ScriptMethodWidget{m_overrideSignals, m_simulator, m_propertyStorage, nullptr};

		m_tabWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

		m_tabWidget->addTab(m_valueTabWidget, tr("Value"));
		m_tabWidget->addTab(m_scriptTabWidget, tr("Script"));

		// --
		//
		QGridLayout* layout = new QGridLayout;
		setLayout(layout);

		layout->addWidget(new QLabel("CustomSignalID:"), 0, 0);
		layout->addWidget(m_customSiganIdLabel, 0, 1);

		layout->addWidget(new QLabel("AppSignalID:"), 1, 0);
		layout->addWidget(m_appSiganIdLabel, 1, 1);

		layout->addWidget(new QLabel("Caption:"), 2, 0);
		layout->addWidget(m_captionLabel, 2, 1);

		layout->addWidget(new QLabel("Type:"), 3, 0);
		layout->addWidget(m_typeLabel, 3, 1);

		layout->addWidget(m_tabWidget, 4, 0, 1, 2);

		// --
		//
		connect(&m_simulator, &Sim::Simulator::projectUpdated, this, &OverrideDialog::projectUpdated);
		connect(&m_simulator.overrideSignals(), &Sim::OverrideSignals::signalsChanged, this, &OverrideDialog::overrideSignalsChaged);

		for (const auto& signal : m_overrideSignals)
		{
			s_openedDialogs[signal.appSignalId()] = this;
		}

		// Select last selected method
		//
		QString lastMethod;
		QString firstSignalId = m_overrideSignals.front().appSignalId();

		QString key = QString("OverrideMethodWidget/%1").arg(firstSignalId);

		lastMethod = m_propertyStorage.loadProperty(key + "OverrideMethod", u"Value");

		if (lastMethod == "Value")
		{
			m_tabWidget->setCurrentIndex(0);
		}

		if (lastMethod == "Script")
		{
			m_tabWidget->setCurrentIndex(1);
		}

		return;
	}

	OverrideDialog::~OverrideDialog()
	{
		std::erase_if(s_openedDialogs,
					  [this](const auto& pair)
					  {
						  return pair.second == this;
					  });

		return;
	}

	bool OverrideDialog::showDialog(std::vector<Sim::OverrideSignalParam> overrideSignals,
									Sim::Simulator& simulator,
									SimUi::ISimPropertyStorage& propertyStorage,
									QWidget* parent)
	{
		if (overrideSignals.empty() == true)
		{
			return false;
		}

		if (overrideSignals.size() == 1)
		{
			QString signalId = overrideSignals.front().appSignalId();
			OverrideDialog* w = nullptr;

			auto it = s_openedDialogs.find(signalId);
			if (it == s_openedDialogs.end())
			{
				w = new OverrideDialog{overrideSignals, simulator, propertyStorage, parent};
				w->show(); // show as modalless dialog
				w->layout()->update();
			}
			else
			{
				w = it->second;
			}

			assert(w);
			if (w->isHidden() == true)
			{
				w->setVisible(true);
			}

			if (w->isActiveWindow() == false)
			{
				w->activateWindow();
			}

			w->raise();

			return true;
		}

		// overrideSignals.size() > 1
		//

		// Check that all signals have the same type and data format.
		//
		const auto& firstSignal = overrideSignals.front();
		for (const auto& signal : overrideSignals)
		{
			if (firstSignal.sameType(signal) == false)
			{
				return false;
			}
		}

		{
			// Create a windows for multiple signals.
			//

			OverrideDialog* w = new OverrideDialog{overrideSignals, simulator, propertyStorage, parent};
			w->show(); // show as modalless dialog
			w->layout()->update();
		}

		return true;
	}

	void OverrideDialog::setViewOptions(const QStringList& appSignalIds, int base, E::AnalogFormat analogFormat, int precision)
	{
		for (const auto& appSignalId : appSignalIds)
		{
			auto it = s_openedDialogs.find(appSignalId);
			if (it != s_openedDialogs.end())
			{
				OverrideDialog* w = it->second;

				assert(w->m_valueTabWidget);
				w->m_valueTabWidget->setViewOptions(base, analogFormat, precision);

				assert(w->m_scriptTabWidget);
				w->m_scriptTabWidget->setViewOptions(base, analogFormat, precision);
			}
		}

		return;
	}

	void OverrideDialog::resizeEvent(QResizeEvent* event)
	{
		if (isVisible() == true)
		{
			m_prevVisibleSize = event->size();
		}
		else
		{
			if (m_prevVisibleSize.isValid() == true)
			{
				resize(m_prevVisibleSize);
			}
		}

		return;
	}

	void OverrideDialog::projectUpdated()
	{
		if (m_simulator.isLoaded() == false)
		{
			return;
		}

		QStringList signalIds = appSignalIds();

		for (const QString& signalId : signalIds)
		{
			std::optional<Sim::OverrideSignalParam> s = m_simulator.overrideSignals().overrideSignal(signalId);

			if (s.has_value() == true)
			{
				auto findPredicat = [&signalId](const auto& os)
				{
					return os.appSignalId() == signalId;
				};

				auto sit = std::find_if(m_overrideSignals.begin(), m_overrideSignals.end(), findPredicat);
				Q_ASSERT(sit != m_overrideSignals.end());

				if (sit != m_overrideSignals.end())
				{
					*sit = s.value();
				}
			}
			else
			{
				// Signal has been removed?
				//
				setAttribute(Qt::WA_DeleteOnClose, true);
				close();

				return;
			}
		}

		updateSignalsUi();
		return;
	}

	void OverrideDialog::overrideSignalsChaged(QStringList /*appSignalIds*/) // Added or deleted signal
	{
		if (m_simulator.isLoaded() == false)
		{
			return;
		}

		QStringList signalIds = appSignalIds();

		for (const QString& signalId : signalIds)
		{
			std::optional<Sim::OverrideSignalParam> s = m_simulator.overrideSignals().overrideSignal(signalId);

			if (s.has_value() == false)
			{
				// At least one signal was removed, close this dialog.
				//
				setAttribute(Qt::WA_DeleteOnClose, true);
				close();

				break;
			}
		}

		return;
	}


	void OverrideDialog::updateSignalsUi()
	{
		size_t signalCount = m_overrideSignals.size();
		if (signalCount < 1)
		{
			Q_ASSERT(signalCount > 0);
			return;
		}

		// All signals must have the same type, we can take types and data formats form the first signal.
		//
		const auto& firstSignal = m_overrideSignals.front();

		if (signalCount == 1)
		{
			m_customSiganIdLabel->setText(firstSignal.customSignalId());
			m_appSiganIdLabel->setText(firstSignal.appSignalId());
			m_captionLabel->setText(firstSignal.caption());
		}
		else
		{
			m_customSiganIdLabel->setText(firstSignal.customSignalId() + QString{", ... + %1 signal(s)"}.arg(signalCount - 1));
			m_appSiganIdLabel->setText(firstSignal.appSignalId() + QString{", ... + %1 signal(s)"}.arg(signalCount - 1));
			m_captionLabel->setText(firstSignal.caption() + QString{", ... + %1 signal(s)"}.arg(signalCount - 1));
		}

		// Type/Format
		//
		QString text;
		if (firstSignal.signalType() == E::SignalType::Discrete)
		{
			text = E::valueToString<E::SignalType>(firstSignal.signalType());
		}

		if (firstSignal.signalType() == E::SignalType::Analog)
		{
			text = QString("%1 (%2)")
					   .arg(E::valueToString<E::SignalType>(firstSignal.signalType()))
					   .arg(E::valueToString<E::AnalogAppSignalFormat>(firstSignal.dataFormat()));
		}

		if (firstSignal.signalType() == E::SignalType::Bus)
		{
			text = QString("%1").arg(E::valueToString<E::SignalType>(firstSignal.signalType()));
		}

		m_typeLabel->setText(text);

		return;
	}

	QStringList OverrideDialog::appSignalIds() const
	{
		QStringList result;
		result.reserve(m_overrideSignals.size());

		for (const auto& signal : m_overrideSignals)
		{
			result.push_back(signal.appSignalId());
		}

		return result;
	}

	Sim::Simulator* OverrideDialog::simulator()
	{
		return &m_simulator;
	}

	const Sim::Simulator* OverrideDialog::simulator() const
	{
		return &m_simulator;
	}

	Sim::OverrideSignals& OverrideDialog::overrideSignals()
	{
		return m_simulator.overrideSignals();
	}

	const Sim::OverrideSignals& OverrideDialog::overrideSignals() const
	{
		return m_simulator.overrideSignals();
	}

} // namespace SimOverrideUI
