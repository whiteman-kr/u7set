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

	void OverrideMethodWidget::setValue(QVariant value)
	{
		assert(m_simulator);

		m_simulator->overrideSignals().setValue(m_signal.m_appSignalId, value);

		return;
	}

	//
	// ValueMethodWidget
	//
	ValueMethodWidget::ValueMethodWidget(const Sim::OverrideSignalParam& signal, Sim::Simulator* simulator, QWidget* parent) :
		OverrideMethodWidget(signal, simulator, parent)
	{

		switch (signal.m_signalType)
		{
		case E::SignalType::Analog:
			{
				switch (signal.m_dataFormat)
				{
				case E::AnalogAppSignalFormat::SignedInt32:
					{
						m_intSpinBox = new SInt32SpinBox{signal.m_value.toInt(), this};
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

									setValue(QVariant::fromValue<float>(v));
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
				m_discreteSpinBox = new DiscreteSpinBox{signal.m_value.toInt(), this};
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

		// Ok/Cancel
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

		switch (m_signal.m_signalType)
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
			setValue(newValue);
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
		setWindowTitle(tr("Override %1").arg(m_signal.m_appSignalId));

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

		m_openedDialogs[m_signal.m_appSignalId] = this;

		return;
	}

	OverrideValueWidget::~OverrideValueWidget()
	{
		m_openedDialogs.erase(m_signal.m_appSignalId);
	}


	bool OverrideValueWidget::showDialog(const Sim::OverrideSignalParam& signal, Sim::Simulator* simulator, QWidget* parent)
	{
		OverrideValueWidget* w = nullptr;

		auto it = m_openedDialogs.find(signal.m_appSignalId);
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
		m_customSiganIdLabel->setText(m_signal.m_customSignalId);
		m_appSiganIdLabel->setText(m_signal.m_appSignalId);
		m_captionLabel->setText(m_signal.m_caption);

		// Type/Format
		//
		QString text;
		if (m_signal.m_signalType == E::SignalType::Discrete)
		{
			text = E::valueToString<E::SignalType>(m_signal.m_signalType);
		}

		if (m_signal.m_signalType == E::SignalType::Analog)
		{
			text = QString("%1 (%2)")
				   .arg(E::valueToString<E::SignalType>(m_signal.m_signalType))
				   .arg(E::valueToString<E::AnalogAppSignalFormat>(m_signal.m_dataFormat));
		}

		if (m_signal.m_signalType == E::SignalType::Bus)
		{
			text = QString("%1")
				   .arg(E::valueToString<E::SignalType>(m_signal.m_signalType));
		}

		m_typeLabel->setText(text);

		return;
	}

	QString OverrideValueWidget::appSignalId() const
	{
		return m_signal.m_appSignalId;
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
