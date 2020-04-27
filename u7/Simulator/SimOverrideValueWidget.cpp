#include "SimOverrideValueWidget.h"


namespace SimOverrideUI
{

	DiscreteSpinBox::DiscreteSpinBox(int value, QWidget* parent) :
		QSpinBox(parent)
	{
		setValue(value);
		setAccelerated(false);
		setRange(0, 1);
		setAlignment(Qt::AlignRight);
		setWrapping(true);

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
		setValue(value);


		setRange(std::numeric_limits<qint32>::lowest(), std::numeric_limits<qint32>::max());
		setWrapping(true);
		setAccelerated(false);			// on trends it does not look well
		setAlignment(Qt::AlignRight);
		setKeyboardTracking(false);
		setGroupSeparatorShown(true);

		lineEdit()->setInputMethodHints(lineEdit()->inputMethodHints() | Qt::ImhUppercaseOnly);

		return;
	}


	//
	// OverrideMethodWidget
	//
	OverrideMethodWidget::OverrideMethodWidget(const Sim::OverrideSignalParam& signal, Sim::Simulator* simulator, QWidget* parent) :
		QWidget(parent),
		m_signal(signal),
		m_simulator(simulator)
	{
		assert(m_simulator);
	}

	const Sim::OverrideSignalParam& OverrideMethodWidget::signal() const
	{
		return m_signal;
	}

	Sim::OverrideSignalParam& OverrideMethodWidget::signal()
	{
		return m_signal;
	}

	void OverrideMethodWidget::setSignal(const Sim::OverrideSignalParam& signal)
	{
		m_signal = signal;
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
		assert(m_simulator);

		m_simulator->overrideSignals().setValue(m_signal.appSignalId(), method, value);

		return;
	}

	//
	// ValueMethodWidget
	//
	ValueMethodWidget::ValueMethodWidget(const Sim::OverrideSignalParam& signal, Sim::Simulator* simulator, QWidget* parent) :
		OverrideMethodWidget(signal, simulator, parent)
	{

		switch (signal.signalType())
		{
		case E::SignalType::Analog:
			{
				switch (signal.dataFormat())
				{
				case E::AnalogAppSignalFormat::SignedInt32:
					{
						m_intSpinBox = new SInt32SpinBox{signal.value().toInt(), this};
						m_edit = m_intSpinBox;

						m_intSpinBox->setDisplayIntegerBase(m_currentBase);
						m_intSpinBox->setSuffix(m_currentBase == 16 ? " hex" : "");

						connect(m_intSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
								[this](int value)
								{
									this->valueEntered(value);
								});
					}
					break;
				case E::AnalogAppSignalFormat::Float32:
					{
						m_floatEdit = new QLineEdit{this};
						m_floatEdit->setAlignment(Qt::AlignRight);
						m_floatEdit->setText(signal.valueString(m_currentBase, m_analogFormat, m_precision));

						m_floatEditValidator = new QDoubleValidator{this};
						m_floatEditValidator->setDecimals(m_precision);

						m_floatEdit->setValidator(m_floatEditValidator);

						m_edit = m_floatEdit;

						connect(m_floatEdit, &QLineEdit::editingFinished,
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
				m_discreteSpinBox = new DiscreteSpinBox{signal.value().toInt(), this};
				m_edit = m_discreteSpinBox;

				connect(m_discreteSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
						[this, dsp = m_discreteSpinBox](int value)
						{
							this->valueEntered(value);
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
		}
	}

	void ValueMethodWidget::valueEntered(double value)
	{
		// Set this value to override signals
		//
		QVariant newValue;

		switch (m_signal.signalType())
		{
		case E::SignalType::Discrete:
			{
				int intValue = static_cast<int>(value);
				newValue = std::clamp(intValue, 0, 1);
			}
			break;

		case E::SignalType::Analog:
			{
				newValue = value;
			}
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
	ScriptMethodWidget::ScriptMethodWidget(const Sim::OverrideSignalParam& signal, Sim::Simulator* simulator, QWidget* parent) :
		OverrideMethodWidget(signal, simulator, parent)
	{
		m_scriptLabel = new QLabel(tr("Override Value Script:"));

		m_scriptEdit = new QsciScintilla(this);
		m_scriptEdit->setUtf8(true);
		m_scriptEdit->setMarginType(0, QsciScintilla::NumberMargin);
		m_scriptEdit->setMarginWidth(0, 40);
		m_scriptEdit->setTabWidth(4);
		m_scriptEdit->setAutoIndent(true);
#if defined(Q_OS_WIN)
		QFont f = QFont("Consolas");
#else
		QFont f = QFont("Courier");
#endif
		m_scriptEdit->setFont(f);
		m_lexer.setFont(f);
		m_scriptEdit->setLexer(&m_lexer);

		m_scriptEdit->setText(signal.script());
		m_scriptEdit->setModified(false);

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
			// Validate script here
			//
			QString script = m_scriptEdit->text();

			QJSEngine jsEngine;
			QJSValue scriptValue =  jsEngine.evaluate(script);

			if (scriptValue.isError() == true)
			{
				qDebug() << "Script evaluate error at line " << scriptValue.property("lineNumber").toInt();
				qDebug() << "\tClass: " << metaObject()->className();
				qDebug() << "\tStack: " << scriptValue.property("stack").toString();
				qDebug() << "\tMessage: " << scriptValue.toString();

				QMessageBox::critical(this,
									  QApplication::applicationDisplayName(),
									  tr("Script evaluate error at line %1:\n%2")
										  .arg(scriptValue.property("lineNumber").toInt())
										  .arg(scriptValue.toString()));
				return;
			}

			// Set script to signal
			//
			setValue(Sim::OverrideSignalMethod::Script, script);
		}

		if (button == m_templateScriptButton)
		{
			showTemplates();
		}

		return;
	}

	void ScriptMethodWidget::showTemplates()
	{
		QMenu m(tr("Script Templates:"));

		const std::map<QString, QString>* templates = nullptr;

		switch (m_signal.signalType())
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
		for (auto[name, fileName] : *templates)
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


	//
	// OverrideValueWidget
	//
	std::map<QString, OverrideValueWidget*> OverrideValueWidget::m_openedDialogs;

	OverrideValueWidget::OverrideValueWidget(const Sim::OverrideSignalParam& signal, Sim::Simulator* simulator, QWidget* parent) :
		QDialog(parent),
		m_signal(signal),
		m_simulator(simulator)
	{
		assert(m_simulator);

		// --
		//
		setWindowTitle(tr("Override %1").arg(m_signal.appSignalId()));

		setWindowFlag(Qt::WindowContextHelpButtonHint, false);
		setWindowFlag(Qt::WindowMinimizeButtonHint, false);
		setWindowFlag(Qt::WindowMaximizeButtonHint, false);


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
		m_tabWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

		m_tabWidget->addTab(m_valueWidget, tr("Value"));
		m_tabWidget->addTab(m_scriptWidget, tr("Script"));

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
		connect(m_simulator, &Sim::Simulator::projectUpdated, this, &OverrideValueWidget::projectUpdated);
		connect(&m_simulator->overrideSignals(), &Sim::OverrideSignals::signalsChanged, this, &OverrideValueWidget::overrideSignalsChaged);

		m_openedDialogs[m_signal.appSignalId()] = this;

		return;
	}

	OverrideValueWidget::~OverrideValueWidget()
	{
		m_openedDialogs.erase(m_signal.appSignalId());
	}


	bool OverrideValueWidget::showDialog(const Sim::OverrideSignalParam& signal, Sim::Simulator* simulator, QWidget* parent)
	{
		OverrideValueWidget* w = nullptr;

		auto it = m_openedDialogs.find(signal.appSignalId());
		if (it == m_openedDialogs.end())
		{
			w = new OverrideValueWidget(signal, simulator, parent);
			w->show();	// show as modalless dialog
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

	void OverrideValueWidget::setViewOptions(QString appSignalId, int base, E::AnalogFormat analogFormat, int precision)
	{
		auto it = m_openedDialogs.find(appSignalId);
		if (it != m_openedDialogs.end())
		{
			OverrideValueWidget* w = it->second;

			assert(w->m_valueWidget);
			w->m_valueWidget->setViewOptions(base, analogFormat, precision);

			assert(w->m_scriptWidget);
			w->m_scriptWidget->setViewOptions(base, analogFormat, precision);
		}

		return;
	}

	void OverrideValueWidget::resizeEvent(QResizeEvent* event)
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

	void OverrideValueWidget::projectUpdated()
	{
		if (m_simulator->isLoaded() == true)
		{
			std::optional<Sim::OverrideSignalParam> s = m_simulator->overrideSignals().overrideSignal(appSignalId());

			if (s.has_value() == true)
			{
				m_signal = s.value();
				updateSignalsUi();
			}
			else
			{
				// Signal has been removed?
				//
				setAttribute(Qt::WA_DeleteOnClose, true);
				close();
			}
		}

		return;
	}

	void OverrideValueWidget::overrideSignalsChaged(QStringList /*appSignalIds*/)	// Added or deleted signal
	{
		if (m_simulator->isLoaded() == true)
		{
			std::optional<Sim::OverrideSignalParam> s = m_simulator->overrideSignals().overrideSignal(appSignalId());

			if (s.has_value() == false)
			{
				// Signal has been removed
				//
				setAttribute(Qt::WA_DeleteOnClose, true);
				close();
			}
		}

		return;
	}


	void OverrideValueWidget::updateSignalsUi()
	{
		m_customSiganIdLabel->setText(m_signal.customSignalId());
		m_appSiganIdLabel->setText(m_signal.appSignalId());
		m_captionLabel->setText(m_signal.caption());

		// Type/Format
		//
		QString text;
		if (m_signal.signalType() == E::SignalType::Discrete)
		{
			text = E::valueToString<E::SignalType>(m_signal.signalType());
		}

		if (m_signal.signalType() == E::SignalType::Analog)
		{
			text = QString("%1 (%2)")
				   .arg(E::valueToString<E::SignalType>(m_signal.signalType()))
				   .arg(E::valueToString<E::AnalogAppSignalFormat>(m_signal.dataFormat()));
		}

		if (m_signal.signalType() == E::SignalType::Bus)
		{
			text = QString("%1")
				   .arg(E::valueToString<E::SignalType>(m_signal.signalType()));
		}

		m_typeLabel->setText(text);

		return;
	}

	QString OverrideValueWidget::appSignalId() const
	{
		return m_signal.appSignalId();
	}

	const Sim::OverrideSignalParam& OverrideValueWidget::signal() const
	{
		return m_signal;
	}

	Sim::Simulator* OverrideValueWidget::simulator()
	{
		return m_simulator;
	}

	const Sim::Simulator* OverrideValueWidget::simulator() const
	{
		return m_simulator;
	}

	Sim::OverrideSignals& OverrideValueWidget::overrideSignals()
	{
		return m_simulator->overrideSignals();
	}

	const Sim::OverrideSignals& OverrideValueWidget::overrideSignals() const
	{
		return m_simulator->overrideSignals();
	}

}
