#include "OptionsPointsDialog.h"

#include <QMessageBox>
#include <QToolTip>
// -------------------------------------------------------------------------------------------------------------------

OptionsPointsDialog::OptionsPointsDialog(const LinearityOption& linearity, QWidget *parent) :
    QDialog(parent),
    m_linearity (linearity)
{
   setStyleSheet(".OptionsPointsDialog { border: 1px solid grey } ");

    QHBoxLayout* rangeLayout = new QHBoxLayout;

    m_rangeTypeLabel = new QLabel;
    m_rangeTypeList = new QComboBox;

    m_rangeTypeLabel->setText(tr("Division type"));
    for(int t = 0; t < LO_RANGE_TYPE_COUNT; t++)
    {
        m_rangeTypeList->addItem(LinearityRangeTypeStr[t]);
    }

    rangeLayout->addWidget(m_rangeTypeList);
    rangeLayout->addStretch();


    QHBoxLayout* valueLayout = new QHBoxLayout;

    m_valueLabel = new QLabel;
    m_valueEdit = new QLineEdit;
    m_lowRangeLabel = new QLabel;
    m_lowRangeEdit = new QLineEdit;
    m_highRangeLabel = new QLabel;
    m_highRangeEdit = new QLineEdit;

    m_valueLabel->setText(tr("Input value,%"));
    m_lowRangeLabel->setText(tr("low limit,%"));
    m_highRangeLabel->setText(tr("high limit,%"));

    QRegExp rx( "^[-]{0,1}[0-9]*[.]{1}[0-9]*$" );
    QValidator *validator = new QRegExpValidator(rx, this);

    m_valueEdit->setFixedWidth(50);
    m_valueEdit->setAlignment(Qt::AlignHCenter);
    m_valueEdit->setValidator(validator);
    m_lowRangeEdit->setFixedWidth(50);
    m_lowRangeEdit->setAlignment(Qt::AlignHCenter);
    m_lowRangeEdit->setValidator(validator);
    m_highRangeEdit->setFixedWidth(50);
    m_highRangeEdit->setAlignment(Qt::AlignHCenter);
    m_highRangeEdit->setValidator(validator);

    valueLayout->addWidget(m_valueLabel);
    valueLayout->addWidget(m_valueEdit);
    valueLayout->addStretch();
    valueLayout->addWidget(m_lowRangeLabel);
    valueLayout->addWidget(m_lowRangeEdit);
    valueLayout->addWidget(m_highRangeLabel);
    valueLayout->addWidget(m_highRangeEdit);

    QHBoxLayout* listLayout = new QHBoxLayout;
    m_pointList = new QTableWidget;

    QVBoxLayout* buttonsLayout = new QVBoxLayout;
    m_addButton = new QPushButton(tr("&Add"));
    m_editButton = new QPushButton(tr("&Edit"));
    m_removeButton = new QPushButton(tr("&Remove"));
    m_upButton = new QPushButton(tr("&Up"));
    m_downButton = new QPushButton(tr("&Down"));

    buttonsLayout->addWidget(m_addButton);
    buttonsLayout->addWidget(m_editButton);
    buttonsLayout->addWidget(m_removeButton);
    buttonsLayout->addStretch();
    buttonsLayout->addWidget(m_upButton);
    buttonsLayout->addWidget(m_downButton);

    listLayout->addWidget(m_pointList);
    listLayout->addLayout(buttonsLayout);

    QVBoxLayout* mainLayout = new QVBoxLayout;

    mainLayout->addLayout(valueLayout);
    mainLayout->addLayout(listLayout);
    mainLayout->addLayout(rangeLayout);

    setLayout(mainLayout);

    SetHeaderList();

    connect(m_addButton, &QPushButton::clicked, this, &OptionsPointsDialog::onAddPoint);
    connect(m_editButton, &QPushButton::clicked, this, &OptionsPointsDialog::onEditPoint);
    connect(m_removeButton, &QPushButton::clicked, this, &OptionsPointsDialog::onRemovePoint);
    connect(m_upButton, &QPushButton::clicked, this, &OptionsPointsDialog::onUpPoint);
    connect(m_downButton, &QPushButton::clicked, this, &OptionsPointsDialog::onDownPoint);
    connect(m_rangeTypeList, SIGNAL(currentIndexChanged(int)), this, SLOT(onRangeType(int)));
    connect(m_valueEdit, &QLineEdit::textChanged, this, &OptionsPointsDialog::onAutomaticCalculatePoints);
    connect(m_lowRangeEdit, &QLineEdit::textChanged, this, &OptionsPointsDialog::onAutomaticCalculatePoints);
    connect(m_highRangeEdit, &QLineEdit::textChanged, this, &OptionsPointsDialog::onAutomaticCalculatePoints);
 }

// -------------------------------------------------------------------------------------------------------------------

OptionsPointsDialog::~OptionsPointsDialog()
{
}

// -------------------------------------------------------------------------------------------------------------------

void OptionsPointsDialog::SetHeaderList()
{
    QStringList horizontalHeaderLabels;

    for(int c = 0; c < PointsColumnCount; c++)
    {
        horizontalHeaderLabels.append(PointsColumn[c]);
    }

    m_pointList->setColumnCount(PointsColumnCount);
    m_pointList->setHorizontalHeaderLabels(horizontalHeaderLabels);
    m_pointList->setEditTriggers(QAbstractItemView::NoEditTriggers);
}

// -------------------------------------------------------------------------------------------------------------------

void OptionsPointsDialog::updateRangeType()
{
    switch(m_linearity.m_rangeType)
    {
        case LO_RANGE_TYPE_MANUAL:

            m_valueLabel->setText(tr("Input value,%"));

            m_lowRangeLabel->hide();
            m_lowRangeEdit->hide();
            m_highRangeLabel->hide();
            m_highRangeEdit->hide();

            m_addButton->show();
            m_editButton->show();
            m_removeButton->show();
            m_upButton->show();
            m_downButton->show();

            m_valueEdit->setText( "0" );

            break;

        case LO_RANGE_TYPE_AUTOMATIC:

            m_valueLabel->setText(tr("Count of points:"));

            m_lowRangeLabel->show();
            m_lowRangeEdit->show();
            m_highRangeLabel->show();
            m_highRangeEdit->show();

            m_addButton->hide();
            m_editButton->hide();
            m_removeButton->hide();
            m_upButton->hide();
            m_downButton->hide();

            m_valueEdit->setText( QString::number( m_linearity.m_pointBase.count() ) );
            m_lowRangeEdit->setText( QString::number(m_linearity.m_lowLimitRange, 10, 2) );
            m_highRangeEdit->setText( QString::number(m_linearity.m_highLimitRange, 10, 2) );

            break;
    }
}

// -------------------------------------------------------------------------------------------------------------------

void OptionsPointsDialog::updateList()
{
    int rowCount = m_linearity.m_pointBase.count();

    m_pointList->setRowCount(rowCount);

    QTableWidgetItem* item = nullptr;
    QStringList verticalHeaderLabels;

    for(int index = 0; index < rowCount; index++ )
    {
        verticalHeaderLabels.append(QString("%1").arg(index + 1));
        m_pointList->setRowHeight(index, 18);

        LinearityPoint* point = m_linearity.m_pointBase.at(index);

        item = new QTableWidgetItem( QString::number(point->getPrecent(), 10, 2) + " %");
        m_pointList->setItem(index, PointsColumn_Percent, item);
        item->setTextAlignment(Qt::AlignHCenter);

        item = new QTableWidgetItem( QString::number(point->getSensorValue(POINT_SENSOR_I_0_5_MA), 10, 3) + tr(" mA") );
        m_pointList->setItem(index, PointsColumn_0_5mA, item);
        item->setTextAlignment(Qt::AlignHCenter);

        item = new QTableWidgetItem( QString::number(point->getSensorValue(POINT_SENSOR_I_4_20_MA), 10, 3) + tr(" mA") );
        m_pointList->setItem(index, PointsColumn_4_20mA, item);
        item->setTextAlignment(Qt::AlignHCenter);
    }

    m_pointList->setVerticalHeaderLabels(verticalHeaderLabels);

    emit updateLinearityPage(true);
}

// -------------------------------------------------------------------------------------------------------------------

void OptionsPointsDialog::onAddPoint()
{
    QString value = m_valueEdit->text();
    if (value.isEmpty() == true)
    {
        QMessageBox msg;
        msg.setText(tr("Please, input value"));
        msg.exec();

        m_valueEdit->setFocus();

        return;
    }

    LinearityPoint* point = new LinearityPoint(value.toDouble());
    if (point == nullptr)
    {
        return;
    }

    m_linearity.m_pointBase.append( point );

    updateList();
}

// -------------------------------------------------------------------------------------------------------------------
void OptionsPointsDialog::onEditPoint()
{
    QString value = m_valueEdit->text();
    if (value.isEmpty() == true)
    {
        QMessageBox msg;
        msg.setText(tr("Please, input value"));
        msg.exec();

        m_valueEdit->setFocus();

        return;
    }

    int index = m_pointList->currentRow();
    if (index < 0 || index >= m_linearity.m_pointBase.count())
    {
        QMessageBox msg;
        msg.setText(tr("Please, select point"));
        msg.exec();

        return;
    }

    LinearityPoint* point = m_linearity.m_pointBase.at(index);
    if (point == nullptr)
    {
        return;
    }

    point->setPercent( value.toDouble() );

    m_pointList->setFocus();
    m_pointList->selectRow(index);

    updateList();
}

// -------------------------------------------------------------------------------------------------------------------

void OptionsPointsDialog::onRemovePoint()
{
    int index = m_pointList->currentRow();
    if (index < 0 || index >= m_linearity.m_pointBase.count())
    {
        return;
    }

    m_linearity.m_pointBase.removeAt(index);

    updateList();
}

// -------------------------------------------------------------------------------------------------------------------

void OptionsPointsDialog::onUpPoint()
{
    int index = m_pointList->currentRow();
    if (index < 0 || index >= m_linearity.m_pointBase.count())
    {
        return;
    }

    m_pointList->setFocus();

    if (index - 1 < 0)
    {
        return;
    }

    m_linearity.m_pointBase.swap(index, index - 1);
    m_pointList->selectRow(index - 1);

    updateList();
}

// -------------------------------------------------------------------------------------------------------------------

void OptionsPointsDialog::onDownPoint()
{
    int index = m_pointList->currentRow();
    if (index < 0 || index >= m_linearity.m_pointBase.count())
    {
        return;
    }

    m_pointList->setFocus();

    if (index + 1 >= m_linearity.m_pointBase.count())
    {
        return;
    }

    m_linearity.m_pointBase.swap(index, index + 1);
    m_pointList->selectRow(index + 1);

    updateList();
}


// -------------------------------------------------------------------------------------------------------------------

void OptionsPointsDialog::onRangeType(int type)
{
    if (type < 0 || type >= LO_RANGE_TYPE_COUNT)
    {
        return;
    }

    m_linearity.m_rangeType = type;

    updateRangeType();
}

// -------------------------------------------------------------------------------------------------------------------

void OptionsPointsDialog::onAutomaticCalculatePoints()
{
    if (m_linearity.m_rangeType != LO_RANGE_TYPE_AUTOMATIC)
    {
        return;
    }

    QString value = m_valueEdit->text();
    if (value.isEmpty() == true)
    {
        m_valueEdit->setFocus();
        return;
    }

    QString low = m_lowRangeEdit->text();
    if (low.isEmpty() == true)
    {
        m_lowRangeEdit->setFocus();
        return;
    }


    QString high = m_highRangeEdit->text();
    if (high.isEmpty() == true)
    {
        m_highRangeLabel->setFocus();
        return;
    }

    m_linearity.m_lowLimitRange = low.toDouble();
    m_linearity.m_highLimitRange = high.toDouble();

    m_linearity.recalcPoints( value.toDouble() );

    updateList();
}

// -------------------------------------------------------------------------------------------------------------------

void OptionsPointsDialog::keyPressEvent(QKeyEvent *e)
{
    if (m_linearity.m_rangeType != LO_RANGE_TYPE_MANUAL)
    {
        QWidget::keyPressEvent(e);

        return;
    }

//    if(e->key() == Qt::Key_Return)
//    {
//        onAddPoint();
//    }

    if(e->key() == Qt::Key_Delete)
    {
        onRemovePoint();
    }

    QWidget::keyPressEvent(e);
}

// -------------------------------------------------------------------------------------------------------------------

void OptionsPointsDialog::showEvent(QShowEvent* e)
{
    m_rangeTypeList->setCurrentIndex(m_linearity.m_rangeType);
    updateRangeType();
    updateList();

    QWidget::showEvent(e);
}

// -------------------------------------------------------------------------------------------------------------------

