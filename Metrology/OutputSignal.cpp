#include "OutputSignal.h"

#include <QClipboard>

#include "MainWindow.h"
#include "Options.h"
#include "ExportData.h"
#include "FindData.h"
#include "SignalList.h"

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

bool OutputSignalTable::m_showCustomID = true;

// -------------------------------------------------------------------------------------------------------------------

OutputSignalTable::OutputSignalTable(QObject*)
{
}

// -------------------------------------------------------------------------------------------------------------------

OutputSignalTable::~OutputSignalTable()
{
    m_signalMutex.lock();

        m_signalList.clear();

    m_signalMutex.unlock();
}

// -------------------------------------------------------------------------------------------------------------------

int OutputSignalTable::columnCount(const QModelIndex&) const
{
    return OUTPUT_SIGNAL_COLUMN_COUNT;
}

// -------------------------------------------------------------------------------------------------------------------

int OutputSignalTable::rowCount(const QModelIndex&) const
{
    return signalCount();
}

// -------------------------------------------------------------------------------------------------------------------

QVariant OutputSignalTable::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
    {
        return QVariant();
    }

    QVariant result = QVariant();

    if(orientation == Qt::Horizontal )
    {
        if (section >= 0 && section < OUTPUT_SIGNAL_COLUMN_COUNT)
        {
            result = OutputSignalColumn[section];
        }
    }

    if(orientation == Qt::Vertical )
    {
        result = QString("%1").arg( section + 1 );
    }

    return result;
}

// -------------------------------------------------------------------------------------------------------------------

QVariant OutputSignalTable::data(const QModelIndex &index, int role) const
{
    if (index.isValid() == false)
    {
        return QVariant();
    }

    int row = index.row();
    if (row < 0 || row >= signalCount())
    {
        return QVariant();
    }

    int column = index.column();
    if (column < 0 || column > OUTPUT_SIGNAL_COLUMN_COUNT)
    {
        return QVariant();
    }

    OutputSignal signal = at(row);
    if(signal.isValid() == false)
    {
        return QVariant();
    }

    if (role == Qt::TextAlignmentRole)
    {
        return Qt::AlignLeft;
    }

    if (role == Qt::FontRole)
    {
        return theOptions.measureView().font();
    }

    if (role == Qt::TextColorRole)
    {
        if ((column == OUTPUT_SIGNAL_COLUMN_IN_CASE || column == OUTPUT_SIGNAL_COLUMN_IN_CAPTION) && signal.param(OUTPUT_SIGNAL_KIND_INPUT).isValid() == false)
        {
            return QColor( Qt::red );
        }

        if ((column == OUTPUT_SIGNAL_COLUMN_OUT_CASE || column == OUTPUT_SIGNAL_COLUMN_OUT_CAPTION) && signal.param(OUTPUT_SIGNAL_KIND_OUTPUT).isValid() == false)
        {
            return QColor( Qt::red );
        }

        return QVariant();
    }


    if (role == Qt::BackgroundColorRole)
    {
        if (column == OUTPUT_SIGNAL_COLUMN_SEPARATOR1 || column == OUTPUT_SIGNAL_COLUMN_SEPARATOR2 || column == OUTPUT_SIGNAL_COLUMN_SEPARATOR3)
        {
            return QColor( Qt::lightGray );
        }

        return QVariant();
    }

    if (role == Qt::DisplayRole || role == Qt::EditRole)
    {
        return text(row, column, signal);
    }

    return QVariant();
}

// -------------------------------------------------------------------------------------------------------------------

QString OutputSignalTable::text(const int row, const int column, const OutputSignal& signal) const
{
    if (row < 0 || row >= signalCount())
    {
        return QString();
    }

    if (column < 0 || column > OUTPUT_SIGNAL_COLUMN_COUNT)
    {
        return QString();
    }

    if (signal.isValid() == false)
    {
        return QString();
    }

    MeasureSignalParam inParam = signal.param(OUTPUT_SIGNAL_KIND_INPUT);
    MeasureSignalParam outParam = signal.param(OUTPUT_SIGNAL_KIND_OUTPUT);

    QString result;

    switch (column)
    {
        case OUTPUT_SIGNAL_COLUMN_TYPE:         result = signal.typeStr();                                                                              break;
        case OUTPUT_SIGNAL_COLUMN_SEPARATOR1:   result = QString();                                                                                     break;
        case OUTPUT_SIGNAL_COLUMN_IN_CASE:      inParam.isValid() == false  ?   result = QString("???")     : result = inParam.position().caseStr();    break;
        case OUTPUT_SIGNAL_COLUMN_IN_ID:        inParam.isValid() == false  ?   result = signal.appSignalID(OUTPUT_SIGNAL_KIND_INPUT) : result = m_showCustomID == true ? inParam.customAppSignalID() : inParam.appSignalID();      break;
        case OUTPUT_SIGNAL_COLUMN_IN_CAPTION:   inParam.isValid() == false  ?   result = QString("???")     : result = inParam.caption();               break;
        case OUTPUT_SIGNAL_COLUMN_SEPARATOR2:   result = QString();                                                                                     break;
        case OUTPUT_SIGNAL_COLUMN_OUT_CASE:     outParam.isValid() == false ?   result = QString("???")     : result = outParam.position().caseStr();   break;
        case OUTPUT_SIGNAL_COLUMN_OUT_ID:       outParam.isValid() == false ?   result = signal.appSignalID(OUTPUT_SIGNAL_KIND_OUTPUT) : result =  m_showCustomID == true ? outParam.customAppSignalID() : outParam.appSignalID();  break;
        case OUTPUT_SIGNAL_COLUMN_OUT_CAPTION:  outParam.isValid() == false ?   result = QString("???")     : result = outParam.caption();              break;
        case OUTPUT_SIGNAL_COLUMN_SEPARATOR3:   result = QString();                                                                                     break;
        default:                                assert(0);
    }

    return result;
}

// -------------------------------------------------------------------------------------------------------------------

int OutputSignalTable::signalCount() const
{
    int count = 0;

    m_signalMutex.lock();

        count = m_signalList.count();

    m_signalMutex.unlock();

    return count;
}

// -------------------------------------------------------------------------------------------------------------------

OutputSignal OutputSignalTable::at(const int index) const
{
    if (index < 0 || index >= signalCount())
    {
        return OutputSignal();
    }

    OutputSignal signal;

    m_signalMutex.lock();

        signal = m_signalList[index];

    m_signalMutex.unlock();

    return signal;
}

// -------------------------------------------------------------------------------------------------------------------

void OutputSignalTable::set(const QList<OutputSignal> list_add)
{
    int count = list_add.count();
    if (count == 0)
    {
        return;
    }

    beginInsertRows(QModelIndex(), 0, count - 1);

        m_signalMutex.lock();

            m_signalList = list_add;

        m_signalMutex.unlock();

    endInsertRows();
}

// -------------------------------------------------------------------------------------------------------------------

void OutputSignalTable::clear()
{
    int count = signalCount();
    if (count == 0)
    {
        return;
    }

    beginRemoveRows(QModelIndex(), 0, count - 1 );

        m_signalMutex.lock();

            m_signalList.clear();

        m_signalMutex.unlock();

    endRemoveRows();
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

bool OutputSignalItemDialog::m_showCustomID = true;

// -------------------------------------------------------------------------------------------------------------------

OutputSignalItemDialog::OutputSignalItemDialog(QWidget *parent) :
    QDialog(parent)
{
    createInterface();
    updateSignals();
}


// -------------------------------------------------------------------------------------------------------------------

OutputSignalItemDialog::OutputSignalItemDialog(const OutputSignal& outputSignal, QWidget *parent) :
    QDialog(parent)
{
    m_signal = outputSignal;

    createInterface();
    updateSignals();
}

// -------------------------------------------------------------------------------------------------------------------

OutputSignalItemDialog::~OutputSignalItemDialog()
{
}

// -------------------------------------------------------------------------------------------------------------------

void OutputSignalItemDialog::createInterface()
{
    setWindowFlags(Qt::Dialog);
    setWindowIcon(QIcon(":/icons/InOut.png"));

    if (m_signal.isValid() == false)
    {
        setWindowIcon(QIcon(":/icons/Add.png"));
        setWindowTitle(tr("Append output signal"));
    }
    else
    {
        setWindowIcon(QIcon(":/icons/Edit.png"));
        setWindowTitle(tr("Edit output signal"));
    }

    // Output signal type
    //
    QHBoxLayout *typeLayout = new QHBoxLayout;

    m_pTypeList = new QComboBox(this);

    // Show Custom ID
    //
    m_pShowCustomIDCheck = new QCheckBox(tr("Show Custom ID"), this);
    m_pShowCustomIDCheck->setChecked(m_showCustomID);
    m_pShowCustomIDCheck->setLayoutDirection(Qt::RightToLeft);

    typeLayout->addWidget(  new QLabel(tr("Output signal type"), this) );
    typeLayout->addWidget(  m_pTypeList );
    typeLayout->addStretch();
    typeLayout->addWidget(m_pShowCustomIDCheck);

    // Signals
    //
    QGroupBox* signalGroup = new QGroupBox(QString());
    QVBoxLayout *signalLayout = new QVBoxLayout;

        // Input signal
        //
    QHBoxLayout *inputSignalLayout = new QHBoxLayout;

    QLabel* pInputSignalLabel = new QLabel(tr("Input signal"), this);
    m_pInputSignalIDEdit = new QLineEdit(QString(), this);
    m_pInputSignalCaptionEdit = new QLineEdit(QString(), this);
    m_pInputSignalButton = new QPushButton(tr("Select ..."), this);

    pInputSignalLabel->setFixedWidth(70);
    m_pInputSignalIDEdit->setFixedWidth(200);
    m_pInputSignalCaptionEdit->setFixedWidth(200);

    m_pInputSignalIDEdit->setReadOnly(true);
    m_pInputSignalCaptionEdit->setReadOnly(true);
    m_pInputSignalButton->setEnabled( theSignalBase.signalCount() != 0 );

    inputSignalLayout->addWidget(pInputSignalLabel );
    inputSignalLayout->addWidget(m_pInputSignalIDEdit);
    inputSignalLayout->addWidget(new QLabel(tr(" - "), this));
    inputSignalLayout->addWidget(m_pInputSignalCaptionEdit);
    inputSignalLayout->addWidget(m_pInputSignalButton);

        // Output signal
        //
    QHBoxLayout *outputSignalLayout = new QHBoxLayout;

    QLabel* pOutputSignalLabel = new QLabel(tr("Output signal"), this);
    m_pOutputSignalIDEdit = new QLineEdit(QString(), this);
    m_pOutputSignalCaptionEdit = new QLineEdit(QString(), this);
    m_pOutputSignalButton = new QPushButton(tr("Select ..."), this);

    pOutputSignalLabel->setFixedWidth(70);
    m_pOutputSignalIDEdit->setFixedWidth(200);
    m_pOutputSignalCaptionEdit->setFixedWidth(200);

    m_pOutputSignalIDEdit->setReadOnly(true);
    m_pOutputSignalCaptionEdit->setReadOnly(true);
    m_pOutputSignalButton->setEnabled( theSignalBase.signalCount() != 0 );

    outputSignalLayout->addWidget(pOutputSignalLabel);
    outputSignalLayout->addWidget(m_pOutputSignalIDEdit);
    outputSignalLayout->addWidget(new QLabel(tr(" - "), this));
    outputSignalLayout->addWidget(m_pOutputSignalCaptionEdit);
    outputSignalLayout->addWidget(m_pOutputSignalButton);


    signalLayout->addLayout(inputSignalLayout);
    signalLayout->addLayout(outputSignalLayout);

    signalGroup->setLayout(signalLayout);

    m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);

    // connects
    //
    connect(m_pTypeList, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &OutputSignalItemDialog::selectedType);
    connect(m_pInputSignalButton, &QPushButton::clicked, this, &OutputSignalItemDialog::selectInputSignal);
    connect(m_pOutputSignalButton, &QPushButton::clicked, this, &OutputSignalItemDialog::selectOutputSignal);
    connect(m_pShowCustomIDCheck, &QPushButton::clicked, this, &OutputSignalItemDialog::showCustomID);
    connect(m_buttonBox, &QDialogButtonBox::accepted, this, &OutputSignalItemDialog::onOk);
    connect(m_buttonBox, &QDialogButtonBox::rejected, this, &OutputSignalItemDialog::reject);


    for (int t = 0; t < OUTPUT_SIGNAL_TYPE_COUNT; t++)
    {
        m_pTypeList->addItem( OutputSignalType[t] );
    }

    int type = m_signal.type() ;
    if (type < 0 || type >= OUTPUT_SIGNAL_TYPE_COUNT )
    {
        type = OUTPUT_SIGNAL_TYPE_IN_OUT;
        m_signal.setType( type );
    }

    m_pTypeList->setCurrentIndex( type );

    QVBoxLayout *mainLayout = new QVBoxLayout;

    mainLayout->addLayout(typeLayout);
    mainLayout->addWidget(signalGroup);
    mainLayout->addWidget(m_buttonBox);

    setLayout(mainLayout);
}

// -------------------------------------------------------------------------------------------------------------------

void OutputSignalItemDialog::updateSignals()
{
    int type = m_signal.type();
    if (type < 0 || type >= OUTPUT_SIGNAL_TYPE_COUNT )
    {
        return;
    }

    MeasureSignalParam inParam = m_signal.param(OUTPUT_SIGNAL_KIND_INPUT);
    if (inParam.isValid() == true)
    {
        m_pInputSignalIDEdit->setText( m_showCustomID == true ? inParam.customAppSignalID() : inParam.appSignalID()  );
        m_pInputSignalCaptionEdit->setText( inParam.caption() );
    }
    else
    {
        m_pInputSignalIDEdit->setText( m_signal.appSignalID(OUTPUT_SIGNAL_KIND_INPUT) );
        m_pInputSignalCaptionEdit->setText( QString() );
    }

    MeasureSignalParam outParam = m_signal.param(OUTPUT_SIGNAL_KIND_OUTPUT);
    if (outParam.isValid() == true)
    {
        m_pOutputSignalIDEdit->setText( m_showCustomID == true ? outParam.customAppSignalID() : outParam.appSignalID()  );
        m_pOutputSignalCaptionEdit->setText( outParam.caption() );
    }
    else
    {
        m_pOutputSignalIDEdit->setText( m_signal.appSignalID(OUTPUT_SIGNAL_KIND_OUTPUT) );
        m_pOutputSignalCaptionEdit->setText( QString() );
    }


    m_pInputSignalIDEdit->setSelection(0,0);
    m_pInputSignalCaptionEdit->setSelection(0,0);
    m_pOutputSignalIDEdit->setSelection(0,0);
    m_pOutputSignalCaptionEdit->setSelection(0,0);
}

// -------------------------------------------------------------------------------------------------------------------

void OutputSignalItemDialog::selectedType(int)
{
    int type = m_pTypeList->currentIndex() ;
    if (type < 0 || type >= OUTPUT_SIGNAL_TYPE_COUNT )
    {
        return;
    }

    m_signal.setType( type );
}

// -------------------------------------------------------------------------------------------------------------------

void OutputSignalItemDialog::selectInputSignal()
{
    if (theSignalBase.signalCount() == 0)
    {
        return;
    }

    SignalListDialog dialog(true, this);
    if (dialog.exec() != QDialog::Accepted)
    {
        return;
    }

    Hash selectedSignalHash = dialog.selectedSignalHash();
    if (selectedSignalHash == 0)
    {
        assert(selectedSignalHash != 0);
        return;
    }

    MeasureSignalParam param = theSignalBase.signalParam(selectedSignalHash);
    if (param.isValid() == false)
    {
        return;
    }

    m_signal.setParam(OUTPUT_SIGNAL_KIND_INPUT, param);

    updateSignals();
}

// -------------------------------------------------------------------------------------------------------------------

void OutputSignalItemDialog::selectOutputSignal()
{
    if (theSignalBase.signalCount() == 0)
    {
        return;
    }

    SignalListDialog dialog(true, this);
    if (dialog.exec() != QDialog::Accepted)
    {
        return;
    }

    Hash selectedSignalHash = dialog.selectedSignalHash();
    if (selectedSignalHash == 0)
    {
        assert(selectedSignalHash != 0);
        return;
    }

    MeasureSignalParam param = theSignalBase.signalParam(selectedSignalHash);
    if (param.isValid() == false)
    {
        return;
    }

    m_signal.setParam(OUTPUT_SIGNAL_KIND_OUTPUT, param);

    updateSignals();
}

// -------------------------------------------------------------------------------------------------------------------

void OutputSignalItemDialog::showCustomID()
{
    m_showCustomID = !m_showCustomID;

    updateSignals();
}

// -------------------------------------------------------------------------------------------------------------------

void OutputSignalItemDialog::onOk()
{
    int type = m_signal.type();
    if (type < 0 || type >= OUTPUT_SIGNAL_TYPE_COUNT )
    {
        QMessageBox::information(this, windowTitle(), tr("Please, select output signal type!"));
        m_pTypeList->setFocus();
        return;
    }

    if ( m_signal.param(OUTPUT_SIGNAL_KIND_INPUT).isValid() == false)
    {
        QMessageBox::information(this, windowTitle(), tr("Please, select input signal!"));
        m_pInputSignalButton->setFocus();
        return;
    }

    if (m_signal.param(OUTPUT_SIGNAL_KIND_OUTPUT).isValid() == false)
    {
        QMessageBox::information(this, windowTitle(), tr("Please, select output signal!"));
        m_pOutputSignalButton->setFocus();
        return;
    }

    accept();
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

int OutputSignalDialog::m_columnWidth[OUTPUT_SIGNAL_COLUMN_COUNT] =
{
    100,    // OUTPUT_SIGNAL_COLUMN_TYPE
      3,    // OUTPUT_SIGNAL_COLUMN_SEPARATOR1
    100,    // OUTPUT_SIGNAL_COLUMN_IN_CASE
    250,    // OUTPUT_SIGNAL_COLUMN_IN_ID
    150,    // OUTPUT_SIGNAL_COLUMN_IN_CAPTION
      3,    // OUTPUT_SIGNAL_COLUMN_SEPARATOR2
    100,    // OUTPUT_SIGNAL_COLUMN_OUT_CASE
    250,    // OUTPUT_SIGNAL_COLUMN_OUT_ID
    150,    // OUTPUT_SIGNAL_COLUMN_OUT_CAPTION
      3,    // OUTPUT_SIGNAL_COLUMN_SEPARATOR3
};

// -------------------------------------------------------------------------------------------------------------------

OutputSignalDialog::OutputSignalDialog(QWidget *parent) :
    QDialog(parent)
{
    MainWindow* pMainWindow = dynamic_cast<MainWindow*> (parent);
    if (pMainWindow != nullptr && pMainWindow->m_pSignalSocket != nullptr)
    {
        connect(pMainWindow->m_pSignalSocket, &SignalSocket::signalsLoaded, this, &OutputSignalDialog::updateList, Qt::QueuedConnection);
        connect(pMainWindow->m_pSignalSocket, &SignalSocket::socketDisconnected, this, &OutputSignalDialog::updateList, Qt::QueuedConnection);
    }

    m_signalBase = theOutputSignalBase;

    createInterface();
    updateList();
}

// -------------------------------------------------------------------------------------------------------------------

OutputSignalDialog::~OutputSignalDialog()
{
}

// -------------------------------------------------------------------------------------------------------------------

void OutputSignalDialog::createInterface()
{
    setWindowFlags(Qt::Window | Qt::WindowMaximizeButtonHint | Qt::WindowCloseButtonHint);
    setWindowIcon(QIcon(":/icons/InOut.png"));
    setWindowTitle(tr("Output signals"));
    resize(QApplication::desktop()->availableGeometry().width() - 700, 500);
    move(QApplication::desktop()->availableGeometry().center() - rect().center());

    m_pMenuBar = new QMenuBar(this);
    m_pSignalMenu = new QMenu(tr("&Signal"), this);
    m_pEditMenu = new QMenu(tr("&Edit"), this);
    m_pViewMenu = new QMenu(tr("&View"), this);

    m_pAddAction = m_pSignalMenu->addAction(tr("&Add ..."));
    m_pAddAction->setIcon(QIcon(":/icons/Add.png"));
    m_pAddAction->setShortcut(Qt::Key_Insert);

    m_pEditAction = m_pSignalMenu->addAction(tr("&Edit ..."));
    m_pEditAction->setIcon(QIcon(":/icons/Edit.png"));

    m_pRemoveAction = m_pSignalMenu->addAction(tr("&Remove"));
    m_pRemoveAction->setIcon(QIcon(":/icons/Remove.png"));
    m_pRemoveAction->setShortcut(Qt::Key_Delete);

    m_pSignalMenu->addSeparator();

    m_pImportAction = m_pSignalMenu->addAction(tr("&Import ..."));
    m_pImportAction->setIcon(QIcon(":/icons/Import.png"));
    m_pImportAction->setShortcut(Qt::CTRL + Qt::Key_I);

    m_pExportAction = m_pSignalMenu->addAction(tr("&Export ..."));
    m_pExportAction->setIcon(QIcon(":/icons/Export.png"));
    m_pExportAction->setShortcut(Qt::CTRL + Qt::Key_E);

    m_pFindAction = m_pEditMenu->addAction(tr("&Find ..."));
    m_pFindAction->setIcon(QIcon(":/icons/Find.png"));
    m_pFindAction->setShortcut(Qt::CTRL + Qt::Key_F);

    m_pEditMenu->addSeparator();

    m_pCopyAction = m_pEditMenu->addAction(tr("&Copy"));
    m_pCopyAction->setIcon(QIcon(":/icons/Copy.png"));
    m_pCopyAction->setShortcut(Qt::CTRL + Qt::Key_C);

    m_pSelectAllAction = m_pEditMenu->addAction(tr("Select &All"));
    m_pSelectAllAction->setIcon(QIcon(":/icons/SelectAll.png"));
    m_pSelectAllAction->setShortcut(Qt::CTRL + Qt::Key_A);

    m_pShowCustomIDAction = m_pViewMenu->addAction(tr("Show Custom ID"));
    m_pShowCustomIDAction->setCheckable(true);
    m_pShowCustomIDAction->setChecked(m_signalTable.showCustomID());
    m_pShowCustomIDAction->setShortcut(Qt::CTRL + Qt::Key_Tab);

    m_pMenuBar->addMenu(m_pSignalMenu);
    m_pMenuBar->addMenu(m_pEditMenu);
    m_pMenuBar->addMenu(m_pViewMenu);

    connect(m_pAddAction, &QAction::triggered, this, &OutputSignalDialog::addSignal);
    connect(m_pEditAction, &QAction::triggered, this, &OutputSignalDialog::editSignal);
    connect(m_pRemoveAction, &QAction::triggered, this, &OutputSignalDialog::removeSignal);
    connect(m_pImportAction, &QAction::triggered, this, &OutputSignalDialog::importSignal);
    connect(m_pExportAction, &QAction::triggered, this, &OutputSignalDialog::exportSignal);

    connect(m_pFindAction, &QAction::triggered, this, &OutputSignalDialog::find);
    connect(m_pCopyAction, &QAction::triggered, this, &OutputSignalDialog::copy);
    connect(m_pSelectAllAction, &QAction::triggered, this, &OutputSignalDialog::selectAll);

    connect(m_pShowCustomIDAction, &QAction::triggered, this, &OutputSignalDialog::showCustomID);


    m_pView = new QTableView(this);
    m_pView->setModel(&m_signalTable);
    QSize cellSize = QFontMetrics( theOptions.measureView().font() ).size(Qt::TextSingleLine,"A");
    m_pView->verticalHeader()->setDefaultSectionSize(cellSize.height());

    for(int column = 0; column < OUTPUT_SIGNAL_COLUMN_COUNT; column++)
    {
        m_pView->setColumnWidth(column, m_columnWidth[column]);
    }

    m_pView->setSelectionBehavior(QAbstractItemView::SelectRows);

    connect(m_pView, &QTableView::doubleClicked , this, &OutputSignalDialog::onListDoubleClicked);

    QVBoxLayout *mainLayout = new QVBoxLayout;

    m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);

    connect(m_buttonBox, &QDialogButtonBox::accepted, this, &OutputSignalDialog::onOk);
    connect(m_buttonBox, &QDialogButtonBox::rejected, this, &OutputSignalDialog::reject);

    mainLayout->setMenuBar(m_pMenuBar);
    mainLayout->addWidget(m_pView);
    mainLayout->addWidget(m_buttonBox);

    setLayout(mainLayout);

    createContextMenu();
}

// -------------------------------------------------------------------------------------------------------------------

void OutputSignalDialog::createContextMenu()
{
    // create context menu
    //
    m_pContextMenu = new QMenu(tr(""), this);

    m_pContextMenu->addAction(m_pAddAction);
    m_pContextMenu->addAction(m_pEditAction);
    m_pContextMenu->addAction(m_pRemoveAction);
    m_pContextMenu->addSeparator();
    m_pContextMenu->addAction(m_pCopyAction);

    // init context menu
    //
    m_pView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_pView, &QTableWidget::customContextMenuRequested, this, &OutputSignalDialog::onContextMenu);
}

// -------------------------------------------------------------------------------------------------------------------

void OutputSignalDialog::updateList()
{
    m_signalTable.clear();

    m_signalBase.sort();

    QList<OutputSignal> signalList;

    int count = m_signalBase.signalCount();
    for(int i = 0; i < count; i++)
    {
        OutputSignal outputSignal = m_signalBase.signal(i);
        if (outputSignal.isValid() == false)
        {
            continue;
        }

        outputSignal.updateParam();

        signalList.append( outputSignal );
    }

    m_signalTable.set(signalList);
}

// -------------------------------------------------------------------------------------------------------------------

void OutputSignalDialog::addSignal()
{
    OutputSignalItemDialog dialog;
    if (dialog.exec() != QDialog::Accepted)
    {
        return;
    }

    OutputSignal signal = dialog.signal();
    if (signal.isValid() == false)
    {
        return;
    }

    int foundIndex = m_signalBase.find(signal);
    if (foundIndex != -1)
    {
        m_pView->setCurrentIndex(m_signalTable.index(foundIndex, OUTPUT_SIGNAL_COLUMN_TYPE));

        QMessageBox::information(this, windowTitle(), tr("Signal already exist!"));
        return;
    }

    m_signalBase.appendSignal(signal);

    updateList();

    m_pView->setCurrentIndex(m_signalTable.index(m_signalBase.signalCount() - 1, OUTPUT_SIGNAL_COLUMN_TYPE));
}

// -------------------------------------------------------------------------------------------------------------------

void OutputSignalDialog::editSignal()
{
    int index = m_pView->currentIndex().row();
    if (index < 0 || index >= m_signalTable.signalCount())
    {
        QMessageBox::information(this, windowTitle(), tr("Please, select signal for edit!"));
        return;
    }

    OutputSignal signal = m_signalTable.at(index);

    OutputSignalItemDialog dialog(signal);
    if (dialog.exec() != QDialog::Accepted)
    {
        return;
    }

    signal = dialog.signal();
    if (signal.isValid() == false)
    {
        return;
    }

    m_signalBase.setSignal(index, signal);

    updateList();

    m_pView->setCurrentIndex(m_signalTable.index(index, OUTPUT_SIGNAL_COLUMN_TYPE));
}

// -------------------------------------------------------------------------------------------------------------------

void OutputSignalDialog::removeSignal()
{
    int selectedSignalCount = m_pView->selectionModel()->selectedRows().count();
    if (selectedSignalCount == 0)
    {
        QMessageBox::information(this, windowTitle(), tr("Please, select signal for remove!"));
        return;
    }

    if (QMessageBox::question(this, windowTitle(), tr("Do you want delete %1 signal(s)?").arg(selectedSignalCount) ) == QMessageBox::No)
    {
        return;
    }

    int count = m_signalTable.signalCount();
    for(int index = count - 1; index >= 0; index -- )
    {
        if (m_pView->selectionModel()->isRowSelected(index, QModelIndex() ) == true)
        {
            if (index >= 0 && index < m_signalTable.signalCount())
            {
                m_signalBase.remove(index);
            }
        }
    }

    updateList();
}

// -------------------------------------------------------------------------------------------------------------------

void OutputSignalDialog::importSignal()
{

}

// -------------------------------------------------------------------------------------------------------------------

void OutputSignalDialog::exportSignal()
{
    ExportData* dialog = new ExportData(m_pView, tr("OutputSignal"));
    dialog->exec();
}

// -------------------------------------------------------------------------------------------------------------------

void OutputSignalDialog::find()
{
    FindData* dialog = new FindData(m_pView);
    dialog->exec();
}

// -------------------------------------------------------------------------------------------------------------------

void OutputSignalDialog::copy()
{
    QString textClipboard;

    int rowCount = m_pView->model()->rowCount();
    int columnCount = m_pView->model()->columnCount();

    for(int row = 0; row < rowCount; row++)
    {
        if (m_pView->selectionModel()->isRowSelected(row, QModelIndex() ) == false)
        {
            continue;
        }

        for(int column = 0; column < columnCount; column++)
        {
            if (m_pView->isColumnHidden(column) == true)
            {
                continue;
            }

            textClipboard.append(m_pView->model()->data( m_pView->model()->index(row, column)).toString() + "\t");
        }

        textClipboard.replace(textClipboard.length() - 1, 1, "\n");
    }

    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(textClipboard);
}

// -------------------------------------------------------------------------------------------------------------------

void OutputSignalDialog::showCustomID()
{
    m_signalTable.setShowCustomID( m_pShowCustomIDAction->isChecked() );

    updateList();
}

// -------------------------------------------------------------------------------------------------------------------

void OutputSignalDialog::onContextMenu(QPoint)
{
    m_pContextMenu->exec(QCursor::pos());
}

// -------------------------------------------------------------------------------------------------------------------

void OutputSignalDialog::onOk()
{
    theOutputSignalBase.clear();
    theOutputSignalBase = m_signalBase;

    accept();
}

// -------------------------------------------------------------------------------------------------------------------
