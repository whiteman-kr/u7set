#include "OptionsPointsDialog.h"

#include <QMessageBox>
#include <QHeaderView>

#include "Delegate.h"

// -------------------------------------------------------------------------------------------------------------------

OptionsPointsDialog::OptionsPointsDialog(const LinearityOption& linearity, QWidget *parent) :
    QDialog(parent),
    m_linearity (linearity)
{
    setStyleSheet(".OptionsPointsDialog { border: 1px solid grey } ");

    QHBoxLayout* valueLayout = new QHBoxLayout;

    m_rangeTypeList = new QComboBox;

    for(int t = 0; t < LO_RANGE_TYPE_COUNT; t++)
    {
        m_rangeTypeList->addItem(LinearityRangeTypeStr[t]);
    }

    m_pointCountLabel = new QLabel;
    m_pointCountEdit = new QLineEdit;
    m_lowRangeLabel = new QLabel;
    m_lowRangeEdit = new QLineEdit;
    m_highRangeLabel = new QLabel;
    m_highRangeEdit = new QLineEdit;

    m_pointCountLabel->setText(tr("Count of points:"));
    m_lowRangeLabel->setText(tr("Low,%"));
    m_highRangeLabel->setText(tr("High,%"));

    QRegExp rx( "^[-]{0,1}[0-9]*[.]{1}[0-9]*$" );
    QValidator *validator = new QRegExpValidator(rx, this);

    m_pointCountEdit->setFixedWidth(40);
    m_pointCountEdit->setAlignment(Qt::AlignHCenter);
    m_pointCountEdit->setValidator(validator);
    m_lowRangeEdit->setFixedWidth(40);
    m_lowRangeEdit->setAlignment(Qt::AlignHCenter);
    m_lowRangeEdit->setValidator(validator);
    m_highRangeEdit->setFixedWidth(40);
    m_highRangeEdit->setAlignment(Qt::AlignHCenter);
    m_highRangeEdit->setValidator(validator);

    valueLayout->addWidget(m_rangeTypeList);
    valueLayout->addStretch();
    valueLayout->addWidget(m_pointCountLabel);
    valueLayout->addWidget(m_pointCountEdit);
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

    setLayout(mainLayout);

    setHeaderList();

    connect(m_addButton, &QPushButton::clicked, this, &OptionsPointsDialog::onAddPoint);
    connect(m_editButton, &QPushButton::clicked, this, &OptionsPointsDialog::onEditPoint);
    connect(m_removeButton, &QPushButton::clicked, this, &OptionsPointsDialog::onRemovePoint);
    connect(m_upButton, &QPushButton::clicked, this, &OptionsPointsDialog::onUpPoint);
    connect(m_downButton, &QPushButton::clicked, this, &OptionsPointsDialog::onDownPoint);
    connect(m_rangeTypeList, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &OptionsPointsDialog::onRangeType);
    connect(m_pointCountEdit, &QLineEdit::textChanged, this, &OptionsPointsDialog::onAutomaticCalculatePoints);
    connect(m_lowRangeEdit, &QLineEdit::textChanged, this, &OptionsPointsDialog::onAutomaticCalculatePoints);
    connect(m_highRangeEdit, &QLineEdit::textChanged, this, &OptionsPointsDialog::onAutomaticCalculatePoints);
 }

// -------------------------------------------------------------------------------------------------------------------

OptionsPointsDialog::~OptionsPointsDialog()
{
    clearList();
}

// -------------------------------------------------------------------------------------------------------------------

void OptionsPointsDialog::setHeaderList()
{
    QStringList horizontalHeaderLabels;

    for(int sensor = 0; sensor < POINT_SENSOR_COUNT; sensor++)
    {
        horizontalHeaderLabels.append( LinearityPointSensor[sensor] );
    }

    m_pointList->setColumnCount(horizontalHeaderLabels.count());
    m_pointList->setHorizontalHeaderLabels(horizontalHeaderLabels);

    for(int column = 0; column < m_pointList->columnCount(); column++)
    {
        if (column != POINT_SENSOR_PERCENT)
        {
            m_pointList->horizontalHeaderItem(column)->setTextColor( Qt::darkGray );
        }

        if (column > POINT_SENSOR_I_4_20_MA)
        {
            m_pointList->hideColumn(column);
        }
    }

    connect(m_pointList, &QTableWidget::cellDoubleClicked, this, &OptionsPointsDialog::onEditPoint);
    connect(m_pointList, &QTableWidget::cellChanged, this, &OptionsPointsDialog::cellChanged);
    connect(m_pointList, &QTableWidget::currentCellChanged, this, &OptionsPointsDialog::currentCellChanged);

    // init context menu
    //
    m_pointList->horizontalHeader()->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_pointList->horizontalHeader(), &QHeaderView::customContextMenuRequested, this, &OptionsPointsDialog::onHeaderContextMenu);

    m_headerContextMenu = new QMenu(m_pointList);

    for(int sensor = 0; sensor < POINT_SENSOR_COUNT; sensor++)
    {
        if (sensor == POINT_SENSOR_PERCENT)
        {
            continue;
        }

        m_pAction[sensor] = m_headerContextMenu->addAction(LinearityPointSensor[sensor]);
        if (m_pAction[sensor] != nullptr)
        {
            m_pAction[sensor]->setCheckable(true);
            m_pAction[sensor]->setChecked(sensor > POINT_SENSOR_I_4_20_MA ? false : true);


            connect(m_headerContextMenu, static_cast<void (QMenu::*)(QAction*)>(&QMenu::triggered), this, &OptionsPointsDialog::onAction);
        }
    }

    DoubleDelegate * delegate = new DoubleDelegate(this);
    m_pointList->setItemDelegate(delegate);
}

// -------------------------------------------------------------------------------------------------------------------

void OptionsPointsDialog::updateRangeType()
{
    switch(m_linearity.m_rangeType)
    {
        case LO_RANGE_TYPE_MANUAL:

            m_pointCountLabel->hide();
            m_pointCountEdit->hide();
            m_lowRangeLabel->hide();
            m_lowRangeEdit->hide();
            m_highRangeLabel->hide();
            m_highRangeEdit->hide();

            m_addButton->show();
            m_editButton->show();
            m_removeButton->show();
            m_upButton->show();
            m_downButton->show();

            break;

        case LO_RANGE_TYPE_AUTOMATIC:

            m_pointCountLabel->show();
            m_pointCountEdit->show();
            m_lowRangeLabel->show();
            m_lowRangeEdit->show();
            m_highRangeLabel->show();
            m_highRangeEdit->show();

            m_addButton->hide();
            m_editButton->hide();
            m_removeButton->hide();
            m_upButton->hide();
            m_downButton->hide();

            m_pointCountEdit->setText( QString::number( m_linearity.m_pointBase.count() ) );
            m_lowRangeEdit->setText( QString::number(m_linearity.m_lowLimitRange, 10, 2) );
            m_highRangeEdit->setText( QString::number(m_linearity.m_highLimitRange, 10, 2) );

            break;

        default:
            assert(0);
            break;
    }
}

// -------------------------------------------------------------------------------------------------------------------

void OptionsPointsDialog::updateList()
{
    clearList();

    int rowCount = m_linearity.m_pointBase.count();

    m_pointList->setRowCount(rowCount);

    m_updatingList = true;
    QTableWidgetItem* cell = nullptr;
    QStringList verticalHeaderLabels;
    int selectedRow = m_pointList->currentRow();

    // update list
    //
    for(int index = 0; index < rowCount; index++ )
    {
        LinearityPoint point = m_linearity.m_pointBase.at(index);

        point.setPointID(index);

        verticalHeaderLabels.append(QString("%1").arg( point.pointID() + 1 ));
        m_pointList->setRowHeight(index, 18);

        for(int sensor = 0; sensor < POINT_SENSOR_COUNT; sensor++)
        {
            cell = new QTableWidgetItem( QString::number(point.sensorValue(sensor), 10, 3));
            cell->setTextAlignment(Qt::AlignHCenter);
            m_pointList->setItem(index, sensor, cell);

            if (sensor != POINT_SENSOR_PERCENT)
            {
                cell->setTextColor( Qt::darkGray  );
            }
        }

        m_linearity.m_pointBase.set(index, point);
    }

    m_pointList->setVerticalHeaderLabels(verticalHeaderLabels);
    m_updatingList = false;

    // select row
    //
    if (selectedRow == -1)
    {
        selectedRow = 0;
    }
    else
    {
        if (selectedRow >= m_pointList->rowCount())
        {
            selectedRow = m_pointList->rowCount() - 1;
        }
    }

    m_pointList->selectRow(selectedRow);

    emit updateLinearityPage(true);
}

// -------------------------------------------------------------------------------------------------------------------

void OptionsPointsDialog::clearList()
{
    int columnCount = m_pointList->columnCount();
    int rowCount = m_pointList->rowCount();

    for(int column = 0; column < columnCount; column++ )
    {
        for(int row = 0; row < rowCount; row++ )
        {
            QTableWidgetItem *item = m_pointList->item(row, column);
            if (item != nullptr)
            {
                delete item;
            }
        }
    }
}

// -------------------------------------------------------------------------------------------------------------------

void OptionsPointsDialog::hideColumn(int column, bool hide)
{
    if (column < 0 || column >= m_pointList->columnCount())
    {
        return;
    }

    if (hide == true)
    {
        m_pointList->hideColumn(column);
    }
    else
    {
        m_pointList->showColumn(column);
    }
}

// -------------------------------------------------------------------------------------------------------------------

void OptionsPointsDialog::onAddPoint()
{
    int index = m_pointList->currentRow();
    if (index == -1)
    {
        index = m_linearity.m_pointBase.count() - 1;
    }

    m_linearity.m_pointBase.insert(index + 1, LinearityPoint() );

    updateList();

    QTableWidgetItem *item = m_pointList->item(index + 1, POINT_SENSOR_PERCENT);
    if (item == nullptr)
    {
        return;
    }

    m_pointList->editItem(item);


}

// -------------------------------------------------------------------------------------------------------------------

void OptionsPointsDialog::cellChanged(int row, int column)
{
    if (m_updatingList == true)
    {
        return;
    }

    if (column != POINT_SENSOR_PERCENT)
    {
        return;
    }

    int index = row;
    if (index < 0 || index >= m_linearity.m_pointBase.count())
    {
        QMessageBox::information(this, windowTitle(), tr("Please, select point"));
        return;
    }

    QString value = m_pointList->item(row, column)->text();

    LinearityPoint point = m_linearity.m_pointBase.at(index);
    point.setPercent( value.toDouble() );
    m_linearity.m_pointBase.set(index, point);

    updateList();

    m_pointList->setFocus();
    m_pointList->selectRow(index);
}


// -------------------------------------------------------------------------------------------------------------------

void OptionsPointsDialog::currentCellChanged(int, int column, int, int)
{
    if (column == POINT_SENSOR_PERCENT)
    {
        m_pointList->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::AnyKeyPressed);
    }
    else
    {
        m_pointList->setEditTriggers(QAbstractItemView::NoEditTriggers);
    }
}

// -------------------------------------------------------------------------------------------------------------------

void OptionsPointsDialog::onEditPoint()
{
    int row = m_pointList->currentRow();
    if (row == -1)
    {
        return;
    }

    QTableWidgetItem* item = m_pointList->item( row, POINT_SENSOR_PERCENT);
    if(item == nullptr)
    {
        return;
    }

    m_pointList->editItem(item);
}

// -------------------------------------------------------------------------------------------------------------------

void OptionsPointsDialog::onRemovePoint()
{
    int index = m_pointList->currentRow();
    if (index < 0 || index >= m_linearity.m_pointBase.count())
    {
        return;
    }

    m_linearity.m_pointBase.remove(index);

    updateList();

    m_pointList->setFocus();
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

    m_pointList->setFocus();
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

    m_pointList->setFocus();
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

    updateList();
}

// -------------------------------------------------------------------------------------------------------------------

void OptionsPointsDialog::onAutomaticCalculatePoints()
{
    if (m_linearity.m_rangeType != LO_RANGE_TYPE_AUTOMATIC)
    {
        return;
    }

    int cursorPosition = -1;

    QWidget* pEdit = focusWidget();
    if (pEdit != nullptr)
    {
        if(QString::compare(pEdit->metaObject()->className(),"QLineEdit") == 0 )
        {
            cursorPosition = ((QLineEdit*)pEdit)->cursorPosition();
        }
    }

    QString value = m_pointCountEdit->text();
    if (value.isEmpty() == true)
    {
        m_pointCountEdit->setFocus();
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

    if (pEdit != nullptr)
    {
        pEdit->setFocus();
        if (cursorPosition != -1)
        {
            ((QLineEdit*)pEdit)->setCursorPosition(cursorPosition);
        }
    }
}

// -------------------------------------------------------------------------------------------------------------------

void OptionsPointsDialog::onHeaderContextMenu(QPoint)
{
    if (m_headerContextMenu == nullptr)
    {
        return;
    }

    m_headerContextMenu->exec(QCursor::pos());
}

// -------------------------------------------------------------------------------------------------------------------

void OptionsPointsDialog::onAction(QAction* action)
{
    if (action == nullptr)
    {
        return;
    }

    for(int sensor = 0; sensor < POINT_SENSOR_COUNT; sensor++)
    {
        if (m_pAction[sensor] == action)
        {
            hideColumn(sensor,  !action->isChecked());

            break;
        }
    }
}

// -------------------------------------------------------------------------------------------------------------------

void OptionsPointsDialog::keyPressEvent(QKeyEvent *e)
{
    if(e->key() == Qt::Key_Return)
    {
        onEditPoint();

        return;
    }

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

