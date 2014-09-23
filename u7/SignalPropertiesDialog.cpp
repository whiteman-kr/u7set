#include "SignalPropertiesDialog.h"
#include <QtStringPropertyManager>
#include <QtIntPropertyManager>
#include <QtEnumPropertyManager>
#include <QtDoublePropertyManager>
#include <QtProperty>
#include <QtTreePropertyBrowser>
#include <QtLineEditFactory>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include "../include/Signal.h"

SignalPropertiesDialog::SignalPropertiesDialog(Signal& signal, QVector<DataFormat>& dataFormatInfo, QVector<Unit>& unitInfo, QWidget *parent) :
	QDialog(parent),
	m_signal(signal)
{
	QtGroupPropertyManager *groupManager = new QtGroupPropertyManager(this);
	m_stringManager = new QtStringPropertyManager(this);
	m_enumManager = new QtEnumPropertyManager(this);
	m_intManager = new QtIntPropertyManager(this);
	m_doubleManager = new QtDoublePropertyManager(this);
	QtProperty *signalProperty = groupManager->addProperty(tr("Signal"));

    m_strIDProperty = m_stringManager->addProperty(tr("ID"));
	m_stringManager->setValue(m_strIDProperty, signal.strID());
    signalProperty->addSubProperty(m_strIDProperty);

	m_extStrIDProperty = m_stringManager->addProperty(tr("External ID"));
	m_stringManager->setValue(m_extStrIDProperty, signal.extStrID());
    signalProperty->addSubProperty(m_extStrIDProperty);

	m_nameProperty = m_stringManager->addProperty(tr("Name"));
	m_stringManager->setValue(m_nameProperty, signal.name());
    signalProperty->addSubProperty(m_nameProperty);

	m_dataFormatProperty = m_enumManager->addProperty(tr("Data format"));
	QStringList dataFormatNames;
	for (int i = 0; i < dataFormatInfo.count(); i++)
	{
		dataFormatNames << dataFormatInfo[i].name;
	}
	//dataFormatNames << "LE unsigned" << "LE signed" << "BE unsigned" << "BE signed";
	m_enumManager->setEnumNames(m_dataFormatProperty, dataFormatNames);
	m_enumManager->setValue(m_dataFormatProperty, signal.dataFormat());
	signalProperty->addSubProperty(m_dataFormatProperty);

	m_dataSizeProperty = m_intManager->addProperty("Data size");
	m_intManager->setRange(m_dataSizeProperty, 1, 100);
	m_intManager->setValue(m_dataSizeProperty, signal.dataSize());
	signalProperty->addSubProperty(m_dataSizeProperty);

	m_lowAdcProperty = m_intManager->addProperty("Low ADC");
	m_intManager->setRange(m_lowAdcProperty, 1, 65535);
	m_intManager->setValue(m_lowAdcProperty, signal.lowADC());
	signalProperty->addSubProperty(m_lowAdcProperty);

	m_highAdcProperty = m_intManager->addProperty("High ADC");
	m_intManager->setRange(m_highAdcProperty, 1, 65535);
	m_intManager->setValue(m_highAdcProperty, signal.highADC());
	signalProperty->addSubProperty(m_highAdcProperty);

	m_lowLimitProperty = m_doubleManager->addProperty("Low limit");
	m_doubleManager->setValue(m_lowLimitProperty, signal.lowLimit());
	signalProperty->addSubProperty(m_lowLimitProperty);

	m_highLimitProperty = m_doubleManager->addProperty("High limit");
	m_doubleManager->setValue(m_highLimitProperty, signal.highLimit());
	signalProperty->addSubProperty(m_highLimitProperty);

	m_unitProperty = m_enumManager->addProperty(tr("Unit"));
	QStringList unitNames;
	for (int i = 0; i < unitInfo.count(); i++)
	{
		unitNames << unitInfo[i].nameEn;
	}
	//unitNames << "unit 1" << "unit 2" << "unit 3" << "unit 4";
	m_enumManager->setEnumNames(m_unitProperty, unitNames);
	m_enumManager->setValue(m_unitProperty, signal.unitID());
	signalProperty->addSubProperty(m_unitProperty);

	QtLineEditFactory* lineEditFactory = new QtLineEditFactory(this);
	QtEnumEditorFactory* enumEditFactory = new QtEnumEditorFactory(this);
	QtSpinBoxFactory* spinBoxFactory = new QtSpinBoxFactory(this);
	QtDoubleSpinBoxFactory* doubleSpinBoxFactory = new QtDoubleSpinBoxFactory(this);

	QtAbstractPropertyBrowser *browser = new QtTreePropertyBrowser(this);
	browser->setFactoryForManager(m_stringManager, lineEditFactory);
	browser->setFactoryForManager(m_enumManager, enumEditFactory);
	browser->setFactoryForManager(m_intManager, spinBoxFactory);
	browser->setFactoryForManager(m_doubleManager, doubleSpinBoxFactory);

	browser->addProperty(signalProperty);

	QVBoxLayout* vl = new QVBoxLayout;
	vl->addWidget(browser);

	QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

	connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
	connect(buttonBox, SIGNAL(accepted()), this, SLOT(saveSignal()));
	connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

	vl->addWidget(buttonBox);
	setLayout(vl);

	setFixedHeight(480);
}

void SignalPropertiesDialog::saveSignal()
{
	m_signal.setStrID(m_stringManager->value(m_strIDProperty));
	m_signal.setExtStrID(m_stringManager->value(m_extStrIDProperty));
	m_signal.setName(m_stringManager->value(m_nameProperty));
}
