#include "SignalPropertiesDialog.h"
#include <qtpropertymanager.h>
#include <QtProperty>
#include <QtTreePropertyBrowser>
#include <QtLineEditFactory>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include "../include/Signal.h"

SignalPropertiesDialog::SignalPropertiesDialog(Signal& signal, QWidget *parent) :
	QDialog(parent),
	m_signal(signal)
{
	QtGroupPropertyManager *groupManager = new QtGroupPropertyManager(this);
	m_stringManager = new QtStringPropertyManager(this);
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

	QtLineEditFactory *lineEditFactory = new QtLineEditFactory(this);

	QtAbstractPropertyBrowser *browser = new QtTreePropertyBrowser(this);
	browser->setFactoryForManager(m_stringManager, lineEditFactory);
	browser->addProperty(signalProperty);

	QVBoxLayout* vl = new QVBoxLayout;
	vl->addWidget(browser);

	QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

	connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
	connect(buttonBox, SIGNAL(accepted()), this, SLOT(saveSignal()));
	connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

	vl->addWidget(buttonBox);
	setLayout(vl);
}

void SignalPropertiesDialog::saveSignal()
{
	m_signal.setStrID(m_stringManager->value(m_strIDProperty));
	m_signal.setExtStrID(m_stringManager->value(m_extStrIDProperty));
	m_signal.setName(m_stringManager->value(m_nameProperty));
}
