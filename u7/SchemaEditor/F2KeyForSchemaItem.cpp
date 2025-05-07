#include "F2KeyForSchemaItem.h"

#include <UiLib/CodeEditor.h>
#include <UiLib/DoubleValidatorEx.h>
#include <VFrame30/LogicSchema.h>
#include <VFrame30/SchemaItemAfb.h>
#include <VFrame30/SchemaItemBus.h>
#include <VFrame30/SchemaItemConnection.h>
#include <VFrame30/SchemaItemConst.h>
#include <VFrame30/SchemaItemImageValue.h>
#include <VFrame30/SchemaItemLineEdit.h>
#include <VFrame30/SchemaItemLoopback.h>
#include <VFrame30/SchemaItemPushButton.h>
#include <VFrame30/SchemaItemRect.h>
#include <VFrame30/SchemaItemSignal.h>
#include <VFrame30/SchemaItemValue.h>
#include <VFrame30/SchemaItemVduImageValue.h>
#include <VFrame30/SchemaItemVduRect.h>
#include <VFrame30/SchemaItemVduValue.h>

#include "../AppSignalSetProvider.h"
#include "../Builder/ConnectionStorage.h"
#include "../EditEngine/EditEngine.h"
#include "../Ui/TextEditCompleter.h"
#include "DbChooseItemsDialog.h"


namespace
{
	class ResizedDialog : public QDialog
	{
	public:
		ResizedDialog(QString title, QWidget* parent, Qt::WindowFlags f = Qt::WindowFlags()) :
			QDialog{parent, f}
		{
			setObjectName("F2ResizeDialog");
			setWindowTitle(title);

			QPoint pos = QSettings{}.value(objectName() + "/pos").toPoint();
			if (pos.isNull() == false)
			{
				move(pos);
			}

			QSize size = QSettings{}.value(objectName() + "/size").toSize();
			if (size.isNull() == false)
			{
				resize(size);
			}

			ensureVisible();

			return;
		}

		virtual void closeEvent(QCloseEvent* event) override
		{
			if (isMaximized() == false)
			{
				QSettings{}.setValue(objectName() + "/pos", pos());
				QSettings{}.setValue(objectName() + "/size", size());
			}

			QDialog::closeEvent(event);
		}

		virtual void hideEvent(QHideEvent* event) override
		{
			if (isMaximized() == false)
			{
				QSettings{}.setValue(objectName() + "/pos", pos());
				QSettings{}.setValue(objectName() + "/size", size());
			}

			QDialog::hideEvent(event);
		}

		void ensureVisible()
		{
			if (QScreen* screen = QGuiApplication::screenAt(geometry().center()); screen == nullptr)
			{
				QScreen* newScreen = QGuiApplication::screenAt(geometry().topLeft());
				if (newScreen == nullptr)
				{
					newScreen = QGuiApplication::screenAt(geometry().topRight());
				}
				if (newScreen == nullptr)
				{
					newScreen = QGuiApplication::screenAt(geometry().bottomLeft());
				}
				if (newScreen == nullptr)
				{
					newScreen = QGuiApplication::screenAt(geometry().bottomRight());
				}
				if (newScreen == nullptr && parentWidget() != nullptr)
				{
					newScreen = QGuiApplication::screenAt(parentWidget()->geometry().center());
				}
				if (newScreen == nullptr)
				{
					newScreen = QGuiApplication::screens().at(0);
				}

				if (QScreen* parentScreen = parentWidget()->window()->windowHandle()->screen(); parentScreen != nullptr)
				{
					newScreen = parentScreen;
				}

				QRect screenGeometry = newScreen->geometry();

				move(screenGeometry.left() + screenGeometry.width() / 2 - width() / 2,
					 screenGeometry.top() + screenGeometry.height() / 2 - height() / 2);
			}

			return;
		}
	};
} // namespace


F2KeyForSchemaItem::F2KeyForSchemaItem(DbController* db, EditEngine::EditEngine* editEngine, QWidget* view, QWidget* parent) :
	m_db{db},
	m_editEngine{editEngine},
	m_view{view},
	m_parent{parent}
{
	Q_ASSERT(m_db);
	Q_ASSERT(m_editEngine);
	Q_ASSERT(m_view);
	return;
}

void F2KeyForSchemaItem::show(SchemaItemPtr schemaItem)
{
	if (schemaItem->isType<VFrame30::SchemaItemRect>() == true)
	{
		f2KeyForRect(schemaItem);
		return;
	}

	if (schemaItem->isType<VFrame30::SchemaItemSignal>() == true)
	{
		f2KeyForSignal(schemaItem);
		return;
	}

	if (schemaItem->isType<VFrame30::SchemaItemConst>() == true)
	{
		f2KeyForConst(schemaItem);
		return;
	}

	if (schemaItem->isType<VFrame30::SchemaItemReceiver>() == true)
	{
		auto schema = schemaItem->parentSchema();
		assert(schema);

		auto logicSchema = schema->toLogicSchema();
		if (logicSchema == nullptr)
		{
			return;
		}

		f2KeyForReceiver(schemaItem, *logicSchema, true);
		return;
	}

	if (schemaItem->isType<VFrame30::SchemaItemTransmitter>() == true)
	{
		auto schema = schemaItem->parentSchema();
		assert(schema);

		auto logicSchema = schema->toLogicSchema();
		if (logicSchema == nullptr)
		{
			return;
		}

		f2KeyForTransmitter(schemaItem, *logicSchema, true);
		return;
	}

	if (schemaItem->isType<VFrame30::SchemaItemLoopback>() == true)
	{
		f2KeyForLoopback(schemaItem);
		return;
	}

	if (schemaItem->isType<VFrame30::SchemaItemValue>() == true)
	{
		f2KeyForValue(schemaItem);
		return;
	}

	if (schemaItem->isType<VFrame30::SchemaItemImageValue>() == true)
	{
		f2KeyForImageValue(schemaItem);
		return;
	}

	if (schemaItem->isType<VFrame30::SchemaItemBus>() == true)
	{
		f2KeyForBus(schemaItem);
		return;
	}

	if (schemaItem->isType<VFrame30::SchemaItemAfb>() == true)
	{
		f2KeyForAfb(schemaItem);
		return;
	}

	if (schemaItem->isType<VFrame30::SchemaItemVduValue>() == true)
	{
		f2KeyForVduValue(schemaItem);
		return;
	}

	if (schemaItem->isType<VFrame30::SchemaItemVduImageValue>() == true)
	{
		f2KeyForVduImageValue(schemaItem);
		return;
	}

	if (schemaItem->isType<VFrame30::SchemaItemVduRect>() == true)
	{
		f2KeyForVduRect(schemaItem);
		return;
	}

	if (schemaItem->isType<VFrame30::SchemaItemPushButton>() == true)
	{
		f2KeyForPushButton(schemaItem);
		return;
	}

	if (schemaItem->isType<VFrame30::SchemaItemLineEdit>() == true)
	{
		f2KeyForLineEdit(schemaItem);
		return;
	}

	return;
}

bool F2KeyForSchemaItem::f2KeyForReceiver(SchemaItemPtr item, const VFrame30::LogicSchema& logicSchema, bool setViaEditEngine)
{
	if (item == nullptr)
	{
		Q_ASSERT(item);
		return false;
	}

	VFrame30::SchemaItemReceiver* receiver = dynamic_cast<VFrame30::SchemaItemReceiver*>(item.get());
	if (receiver == nullptr)
	{
		Q_ASSERT(receiver);
		return false;
	}

	QString recConnectionIds = receiver->connectionIds();
	QString appSignalId = receiver->appSignalIds();

	// Get all connections
	//
	Builder::ConnectionStorage connections(m_db);

	QString errorMessage;

	bool ok = connections.load(&errorMessage);
	if (ok == false)
	{
		QMessageBox::critical(m_parent, qAppName(), errorMessage);
		return false;
	}

	QStringList connectionIds = connections.filterByMoudules(logicSchema.equipmentIdList());

	// Show input dialog
	//
	ResizedDialog d{tr("Set Receiver Params"), m_parent};
	d.setWindowFlags((d.windowFlags() & ~Qt::WindowMinimizeButtonHint & ~Qt::WindowMaximizeButtonHint & ~Qt::WindowContextHelpButtonHint) |
					 Qt::CustomizeWindowHint);

	QLabel* connectionIdLabel = new QLabel("ConnectionID:");

	QTextEditCompleter* connectionIdControl = new QTextEditCompleter(&d);
	connectionIdControl->setPlaceholderText("Enter ConnectionID(s). Press Ctrl+E to show completer.");
	connectionIdControl->setPlainText(recConnectionIds);

	QCompleter* connectionsCompleter = new QCompleter(connectionIds, &d);
	connectionsCompleter->setFilterMode(Qt::MatchContains);
	connectionsCompleter->setCaseSensitivity(Qt::CaseSensitive);
	connectionsCompleter->setMaxVisibleItems(20);
	connectionIdControl->setCompleter(connectionsCompleter);

	// QCompleter for signals
	//
	AppSignalSetProvider* signalSetProvider = AppSignalSetProvider::getInstance();
	Q_ASSERT(signalSetProvider);

	QStringList appSignalIdsCompleterList;
	signalSetProvider->signalSet().appSignalIdsListSorted(true, &appSignalIdsCompleterList);

	QCompleter* appSignalsCompleter = new QCompleter(appSignalIdsCompleterList, &d);
	appSignalsCompleter->setFilterMode(Qt::MatchContains);
	appSignalsCompleter->setCaseSensitivity(Qt::CaseSensitive);
	appSignalsCompleter->setMaxVisibleItems(20);

	// AppSignalIDs widgets
	//
	QLabel* appSignalIdLabel = new QLabel("AppSignalID:");
	QTextEditCompleter* appSignalIdEdit = new QTextEditCompleter(&d);
	appSignalIdEdit->setPlaceholderText("Enter AppSchemaIDs separated by lines. Press Ctrl+E to show completer.");
	appSignalIdEdit->setPlainText(appSignalId);
	appSignalIdEdit->setCompleter(appSignalsCompleter);

	QWidget* spacer = new QWidget;
	spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

	QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

	// --
	//
	QVBoxLayout* layout = new QVBoxLayout;

	layout->addWidget(connectionIdLabel);
	layout->addWidget(connectionIdControl);

	layout->addWidget(appSignalIdLabel);
	layout->addWidget(appSignalIdEdit);

	layout->addWidget(spacer);

	layout->addWidget(buttonBox);

	d.setLayout(layout);

	connect(buttonBox, &QDialogButtonBox::accepted, &d, &QDialog::accept);
	connect(buttonBox, &QDialogButtonBox::rejected, &d, &QDialog::reject);

	// --
	//
	int result = d.exec();

	if (result == QDialog::Accepted)
	{
		QString newConnectionId = connectionIdControl->toPlainText();
		QString newAppSignalId = appSignalIdEdit->toPlainText().trimmed();

		if (newConnectionId != recConnectionIds || newAppSignalId != appSignalId)
		{
			if (setViaEditEngine == true)
			{
				ok = m_editEngine->startBatch();

				if (ok == true)
				{
					m_editEngine->runSetProperty(VFrame30::PropertyNames::connectionId, QVariant(newConnectionId), item);
					m_editEngine->runSetProperty(VFrame30::PropertyNames::appSignalIDs, QVariant(newAppSignalId), item);

					m_editEngine->endBatch();
				}
			}
			else
			{
				receiver->setConnectionIds(newConnectionId);
				receiver->setAppSignalIds(newAppSignalId);
			}
		}

		m_view->update();
		return true;
	}

	return false;
}

bool F2KeyForSchemaItem::f2KeyForTransmitter(SchemaItemPtr item, const VFrame30::LogicSchema& logicSchema, bool setViaEditEngine)
{
	if (item == nullptr)
	{
		Q_ASSERT(item);
		return false;
	}

	VFrame30::SchemaItemTransmitter* transmitter = dynamic_cast<VFrame30::SchemaItemTransmitter*>(item.get());
	if (transmitter == nullptr)
	{
		Q_ASSERT(transmitter);
		return false;
	}

	QString transmitterConnectionIds = transmitter->connectionIds();

	// Get all connections
	//
	Builder::ConnectionStorage connections(m_db);

	QString errorMessage;

	bool ok = connections.load(&errorMessage);
	if (ok == false)
	{
		QMessageBox::critical(m_parent, qAppName(), errorMessage);
		return false;
	}

	QStringList connectionIds = connections.filterByMoudules(logicSchema.equipmentIdList());

	// Show input dialog
	//
	ResizedDialog d(tr("Set Transmitter Params"), m_parent);

	d.setWindowFlags((d.windowFlags() & ~Qt::WindowMinimizeButtonHint & ~Qt::WindowMaximizeButtonHint & ~Qt::WindowContextHelpButtonHint) |
					 Qt::CustomizeWindowHint);

	QLabel* connectionIdLabel = new QLabel("ConnectionID(s):");

	QTextEditCompleter* connectionIdControl = new QTextEditCompleter(&d);
	connectionIdControl->setPlaceholderText("Enter ConnectionID(s). Press Ctrl+E to show completer.");
	connectionIdControl->setPlainText(transmitterConnectionIds);

	QCompleter* completer = new QCompleter(connectionIds, &d);
	completer->setFilterMode(Qt::MatchContains);
	completer->setCaseSensitivity(Qt::CaseSensitive);
	completer->setMaxVisibleItems(20);
	connectionIdControl->setCompleter(completer);

	QWidget* spacer = new QWidget;
	spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

	QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

	// --
	//
	QVBoxLayout* layout = new QVBoxLayout;

	layout->addWidget(connectionIdLabel);
	layout->addWidget(connectionIdControl);

	layout->addWidget(spacer);

	layout->addWidget(buttonBox);

	d.setLayout(layout);

	connect(buttonBox, &QDialogButtonBox::accepted, &d, &QDialog::accept);
	connect(buttonBox, &QDialogButtonBox::rejected, &d, &QDialog::reject);

	// --
	//
	int result = d.exec();

	if (result == QDialog::Accepted)
	{
		QString newConnectionId = connectionIdControl->toPlainText(); // connectionIdControl->currentText().trimmed();

		if (newConnectionId != transmitterConnectionIds)
		{
			if (setViaEditEngine == true)
			{
				m_editEngine->runSetProperty(VFrame30::PropertyNames::connectionId, QVariant(newConnectionId), item);
			}
			else
			{
				transmitter->setConnectionIds(newConnectionId);
			}
		}

		m_view->update();
		return true;
	}

	return false;
}

bool F2KeyForSchemaItem::loadBusses(DbController* db, std::vector<AppSignalLib::Bus>* out, QWidget* parentWidget)
{
	if (db == nullptr || out == nullptr)
	{
		assert(db);
		assert(out);
		return false;
	}

	out->clear();

#if 0
	BusStorage busStorage{db};

	QString error;
	busStorage.load(&error);

	if (error.isEmpty() == false)
	{
		QMessageBox::critical(parentWidget, qAppName(), error);
		return false;
	}

	std::copy(busStorage.begin(), busStorage.end(), std::back_inserter(*out));
	return true;
#else
	// Get Busses
	//
	DbFileTree filesTree;

	bool ok = db->getFileListTree(&filesTree, DbDir::BusTypesDir, QString(".") + File::BusFileExtension, true, parentWidget);
	if (ok == false)
	{
		return false;
	}

	const auto& fileMap = filesTree.files();

	std::vector<DbFileInfo> fileList;
	fileList.reserve(fileMap.size());

	int busTypesFileId = db->systemFileId(DbDir::BusTypesDir);

	for (auto& [fileId, fileInfo] : fileMap)
	{
		if (fileId != busTypesFileId)
		{
			fileList.push_back(*fileInfo);
		}
	}

	if (fileList.empty() == true)
	{
		return true; // It is not error, just no any busses
	}

	// Get Busses latest version from the DB
	//
	std::vector<std::shared_ptr<DbFile>> files;

	ok = db->getLatestVersion(fileList, &files, parentWidget);
	if (ok == false)
	{
		return false;
	}

	// Parse files, create actual Busses
	//
	std::vector<AppSignalLib::Bus> busses;
	busses.reserve(files.size());

	for (const std::shared_ptr<DbFile>& f : files)
	{
		if (f->deleted() == true || f->action() == E::VcsItemAction::Deleted)
		{
			continue;
		}

		AppSignalLib::Bus bus;
		ok = bus.Load(f->data());

		if (ok == false)
		{
			QMessageBox::critical(parentWidget, qAppName(), "Load file " + f->fileName() + " error.");
			return false;
		}

		busses.push_back(bus);
	}

	std::sort(busses.begin(),
			  busses.end(),
			  [](const AppSignalLib::Bus& b1, const AppSignalLib::Bus& b2) -> bool
			  {
				  return b1.busTypeId() < b2.busTypeId();
			  });

	std::swap(busses, *out);
	return true;
#endif
}

void F2KeyForSchemaItem::f2KeyForRect(SchemaItemPtr item)
{
	if (item == nullptr)
	{
		Q_ASSERT(item);
		return;
	}

	VFrame30::SchemaItemRect* rectItem = dynamic_cast<VFrame30::SchemaItemRect*>(item.get());
	if (rectItem == nullptr)
	{
		Q_ASSERT(rectItem);
		return;
	}

	QString text = rectItem->text();

	// Show input dialog
	//
	ResizedDialog d{tr("Set text"), m_parent};
	d.setWindowFlags((d.windowFlags() & ~Qt::WindowMinimizeButtonHint & ~Qt::WindowMaximizeButtonHint & ~Qt::WindowContextHelpButtonHint) |
					 Qt::CustomizeWindowHint);

	// --
	//
	QString labelText;
	switch (rectItem->textFormat())
	{
	case E::TextFormat::PlainText:
		labelText = tr("PlainText - Manual formatting.");
		break;
	case E::TextFormat::Markdown:
		labelText = tr("Markdown - Markdown formatting, supports GitHub-style Markdown.");
		break;
	case E::TextFormat::HtmlSubset:
		labelText = tr("HtmlSubset - HTML-formatted text in the html string. Support of the limited HTML Subset.");
		break;
	}

	QLabel* label = new QLabel{labelText, &d};

	QTextEdit* textEdit = new QTextEdit{&d};
	textEdit->setAcceptRichText(false);
	textEdit->setPlainText(text);

	QDialogButtonBox* buttonBox = new QDialogButtonBox{QDialogButtonBox::Apply | QDialogButtonBox::Ok | QDialogButtonBox::Cancel};

	// --
	//
	QVBoxLayout* layout = new QVBoxLayout;

	layout->addWidget(label);
	layout->addWidget(textEdit);
	layout->addWidget(buttonBox);

	d.setLayout(layout);

	connect(buttonBox,
			&QDialogButtonBox::clicked,
			[item, this, buttonBox, textEdit, &text](QAbstractButton* button)
			{
				if (buttonBox->buttonRole(button) == QDialogButtonBox::ApplyRole)
				{
					QString newValue = textEdit->toPlainText();

					if (newValue != text)
					{
						m_editEngine->runSetProperty(VFrame30::PropertyNames::text, QVariant(newValue), item);
						m_view->update();
					}
				}
			});

	connect(buttonBox, &QDialogButtonBox::accepted, &d, &QDialog::accept);
	connect(buttonBox, &QDialogButtonBox::rejected, &d, &QDialog::reject);

	if (int result = d.exec(); result == QDialog::Accepted)
	{
		QString newValue = textEdit->toPlainText();

		if (newValue != text)
		{
			m_editEngine->runSetProperty(VFrame30::PropertyNames::text, QVariant(newValue), item);
			m_view->update();
		}
	}

	return;
}

void F2KeyForSchemaItem::f2KeyForConst(SchemaItemPtr item)
{
	if (item == nullptr)
	{
		assert(item);
		return;
	}

	VFrame30::SchemaItemConst* constItem = dynamic_cast<VFrame30::SchemaItemConst*>(item.get());
	if (constItem == nullptr)
	{
		assert(constItem);
		return;
	}

	VFrame30::SchemaItemConst::ConstType type = constItem->type();

	Afb::AfbParamValue intValue = constItem->signedInt32Value();
	Afb::AfbParamValue floatValue = constItem->floatValue();
	Afb::AfbParamValue discreteValue = constItem->discreteValue();

	// Show input dialog
	//
	ResizedDialog d(tr("Set Const Params"), m_parent);

	d.setWindowFlags((d.windowFlags() & ~Qt::WindowMinimizeButtonHint & ~Qt::WindowMaximizeButtonHint & ~Qt::WindowContextHelpButtonHint) |
					 Qt::CustomizeWindowHint);

	// Type Items
	//
	QLabel* typeLabel = new QLabel("Const Type:");

	auto buttonSignedInt32 = new QPushButton{"SignedInt32"};
	auto buttonFloat32 = new QPushButton{"Float"};
	auto buttonDiscrete = new QPushButton{"Discrete"};

	buttonSignedInt32->setCheckable(true);
	buttonFloat32->setCheckable(true);
	buttonDiscrete->setCheckable(true);

	QButtonGroup* typeGroup = new QButtonGroup{};

	typeGroup->addButton(buttonSignedInt32, static_cast<int>(VFrame30::SchemaItemConst::ConstType::IntegerType));
	typeGroup->addButton(buttonFloat32, static_cast<int>(VFrame30::SchemaItemConst::ConstType::FloatType));
	typeGroup->addButton(buttonDiscrete, static_cast<int>(VFrame30::SchemaItemConst::ConstType::Discrete));

	switch (type)
	{
	case VFrame30::SchemaItemConst::ConstType::IntegerType:
		buttonSignedInt32->setChecked(true);
		break;
	case VFrame30::SchemaItemConst::ConstType::FloatType:
		buttonFloat32->setChecked(true);
		break;
	case VFrame30::SchemaItemConst::ConstType::Discrete:
		buttonDiscrete->setChecked(true);
		break;
	default:
		Q_ASSERT(false);
	}

	// IntItems
	//
	QLabel* intValueLabel = new QLabel("IntegerValue:");
	QString intInitString = intValue.hasReference() ? intValue.reference() : QString::number(constItem->signedInt32NativeValue());
	QLineEdit* intValueEdit = new QLineEdit{intInitString};
	intValueEdit->setValidator(new QIntValidatorEx(std::numeric_limits<int>::min(),
												   std::numeric_limits<int>::max(),
												   item->parentSchema()->isUfbSchema(),
												   intValueEdit));

	if (type != VFrame30::SchemaItemConst::ConstType::IntegerType)
	{
		intValueLabel->setEnabled(false);
		intValueEdit->setEnabled(false);
	}

	// FloatItems
	//
	QLocale locale;

	QLabel* floatValueLabel = new QLabel("FloatValue:");
	QString floatInitString = floatValue.hasReference() ? floatValue.reference() : locale.toString(constItem->floatNativeValue());
	QLineEdit* floatValueEdit = new QLineEdit(floatInitString);
	floatValueEdit->setValidator(new QDoubleValidatorEx(std::numeric_limits<float>::lowest(),
														std::numeric_limits<float>::max(),
														1000,
														item->parentSchema()->isUfbSchema(),
														floatValueEdit));

	if (type != VFrame30::SchemaItemConst::ConstType::FloatType)
	{
		floatValueLabel->setEnabled(false);
		floatValueEdit->setEnabled(false);
	}

	// DiscreteItems
	//
	QLabel* discreteValueLabel = new QLabel("DiscreteValue (0 or 1):");
	QString discreteInitString =
		discreteValue.hasReference() ? discreteValue.reference() : QString::number(constItem->discreteNativeValue());
	QLineEdit* discreteValueEdit = new QLineEdit(discreteInitString);
	discreteValueEdit->setValidator(new QIntValidatorEx(0, 1, item->parentSchema()->isUfbSchema(), discreteValueEdit));

	if (type != VFrame30::SchemaItemConst::ConstType::Discrete)
	{
		discreteValueLabel->setEnabled(false);
		discreteValueEdit->setEnabled(false);
	}

	// Spacer
	//
	QWidget* spacer = new QWidget;
	spacer->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);

	// --
	//
	QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

	QGridLayout* layout = new QGridLayout;

	layout->addWidget(typeLabel, 0, 0);

	typeGroup->setParent(layout);
	layout->addWidget(buttonSignedInt32, 0, 1);
	layout->addWidget(buttonFloat32, 0, 2);
	layout->addWidget(buttonDiscrete, 0, 3);

	layout->addWidget(intValueLabel, 1, 0);
	layout->addWidget(intValueEdit, 1, 1, 1, 3);

	layout->addWidget(floatValueLabel, 2, 0);
	layout->addWidget(floatValueEdit, 2, 1, 1, 3);

	layout->addWidget(discreteValueLabel, 3, 0);
	layout->addWidget(discreteValueEdit, 3, 1, 1, 3);

	layout->addWidget(spacer, 4, 0, 1, 2);
	layout->addWidget(buttonBox, 5, 0, 1, 4);

	d.setLayout(layout);

	connect(buttonBox, &QDialogButtonBox::accepted, &d, &QDialog::accept);
	connect(buttonBox, &QDialogButtonBox::rejected, &d, &QDialog::reject);

	connect(typeGroup,
			&QButtonGroup::idToggled,
			[intValueLabel, intValueEdit, floatValueLabel, floatValueEdit, discreteValueLabel, discreteValueEdit](int id, bool checked)
			{
				VFrame30::SchemaItemConst::ConstType type = static_cast<VFrame30::SchemaItemConst::ConstType>(id);

				if (type == VFrame30::SchemaItemConst::ConstType::IntegerType)
				{
					intValueLabel->setEnabled(checked);
					intValueEdit->setEnabled(checked);

					if (checked == true)
					{
						intValueEdit->setFocus(Qt::OtherFocusReason);
					}
				}

				if (type == VFrame30::SchemaItemConst::ConstType::FloatType)
				{
					floatValueLabel->setEnabled(checked);
					floatValueEdit->setEnabled(checked);

					if (checked == true)
					{
						floatValueEdit->setFocus(Qt::OtherFocusReason);
					}
				}

				if (type == VFrame30::SchemaItemConst::ConstType::Discrete)
				{
					discreteValueLabel->setEnabled(checked);
					discreteValueEdit->setEnabled(checked);

					if (checked == true)
					{
						discreteValueEdit->setFocus(Qt::OtherFocusReason);
					}
				}
			});

	switch (type)
	{
	case VFrame30::SchemaItemConst::ConstType::IntegerType:
		intValueEdit->setFocus(Qt::OtherFocusReason);
		break;
	case VFrame30::SchemaItemConst::ConstType::FloatType:
		floatValueEdit->setFocus(Qt::OtherFocusReason);
		break;
	case VFrame30::SchemaItemConst::ConstType::Discrete:
		discreteValueEdit->setFocus(Qt::OtherFocusReason);
		break;
	default:
		Q_ASSERT(false);
	}

	// --
	//
	int result = d.exec();

	if (result == QDialog::Accepted)
	{
		VFrame30::SchemaItemConst::ConstType newType = static_cast<VFrame30::SchemaItemConst::ConstType>(typeGroup->checkedId());

		bool batchOk = m_editEngine->startBatch();
		if (batchOk == false)
		{
			return;
		}

		if (newType != type)
		{
			m_editEngine->runSetProperty(VFrame30::PropertyNames::type, QVariant::fromValue(newType), item);
		}

		QRegularExpression rx("^\\$\\(([A-Za-z0-9_]+\\.)*[A-Za-z0-9_]+\\)$"); // $(AA.BB.CC)

		switch (newType)
		{
		case VFrame30::SchemaItemConst::ConstType::IntegerType:
			{
				Afb::AfbParamValue param = intValue;

				QString text = intValueEdit->text().trimmed();
				if (rx.match(text).hasMatch() == true)
				{
					param.setReference(text);
				}
				else
				{
					param.setReference({});

					bool convertOk = false;
					qint32 nativeValue = text.toInt(&convertOk);

					if (convertOk == true)
					{
						param.setValue(nativeValue);
					}
				}

				if (param != intValue)
				{
					m_editEngine->runSetProperty(VFrame30::PropertyNames::valueInteger, QVariant::fromValue(param), item);
				}
			}
			break;
		case VFrame30::SchemaItemConst::ConstType::FloatType:
			{
				Afb::AfbParamValue param = floatValue;

				QString text = floatValueEdit->text().trimmed();
				if (rx.match(text).hasMatch() == true)
				{
					param.setReference(text);
				}
				else
				{
					param.setReference({});

					bool convertOk = false;
					float nativeValue = locale.toFloat(text, &convertOk);

					if (convertOk == true)
					{
						param.setValue(nativeValue);
					}
				}

				if (param != floatValue)
				{
					m_editEngine->runSetProperty(VFrame30::PropertyNames::valueFloat, QVariant::fromValue(param), item);
				}
			}
			break;
		case VFrame30::SchemaItemConst::ConstType::Discrete:
			{
				Afb::AfbParamValue param = discreteValue;

				QString text = discreteValueEdit->text().trimmed();
				if (rx.match(text).hasMatch() == true)
				{
					param.setReference(text);
				}
				else
				{
					param.setReference({});

					bool convertOk = false;
					quint16 nativeValue = static_cast<quint16>(text.toUInt(&convertOk));

					if (convertOk == true)
					{
						param.setValue(nativeValue);
					}
				}

				if (param != discreteValue)
				{
					m_editEngine->runSetProperty(VFrame30::PropertyNames::valueDiscrete, QVariant::fromValue(param), item);
				}
			}
			break;
		default:
			Q_ASSERT(false);
		}

		m_editEngine->endBatch();

		m_view->update();
	}

	return;
}

void F2KeyForSchemaItem::f2KeyForSignal(SchemaItemPtr item)
{
	if (item == nullptr)
	{
		assert(item);
		return;
	}

	VFrame30::SchemaItemSignal* signalItem = dynamic_cast<VFrame30::SchemaItemSignal*>(item.get());
	if (signalItem == nullptr)
	{
		assert(signalItem);
		return;
	}

	QString appSignalIds = signalItem->appSignalIds();
	QString impactAppSignalIds = signalItem->impactAppSignalIds();

	// Show input dialog
	//
	ResizedDialog d(tr("SchemaItemSignal"), m_parent);

	d.setWindowFlags((d.windowFlags() & ~Qt::WindowMinimizeButtonHint & ~Qt::WindowContextHelpButtonHint) | Qt::CustomizeWindowHint |
					 Qt::WindowMaximizeButtonHint);

	// QCompleter for signals
	//
	AppSignalSetProvider* signalSetProvider = AppSignalSetProvider::getInstance();
	Q_ASSERT(signalSetProvider);

	QStringList appSignalIdsCompleterList;
	signalSetProvider->signalSet().appSignalIdsListSorted(true, &appSignalIdsCompleterList);

	QCompleter* completer = new QCompleter(appSignalIdsCompleterList, &d);
	completer->setFilterMode(Qt::MatchContains);
	completer->setCaseSensitivity(Qt::CaseSensitive);
	completer->setMaxVisibleItems(20);


	// AppSchemaIDs
	//
	QLabel* appSignalIdsLabel = new QLabel("AppSignalIDs:", &d);

	QTextEditCompleter* appSignalIdsEdit = new QTextEditCompleter(&d);
	appSignalIdsEdit->setPlaceholderText("Enter AppSignalIDs separated by lines. Press Ctrl+E to show completer.");
	appSignalIdsEdit->setPlainText(appSignalIds);
	appSignalIdsEdit->setCompleter(completer);

	// ImpactAppSignalIDs
	//
	QLabel* impactAppSignalIdsLabel = new QLabel("ImpactAppSignalIDs:", &d);

	QTextEditCompleter* impactAppSignalIdsEdit = new QTextEditCompleter(&d);
	impactAppSignalIdsEdit->setPlaceholderText("Enter Impact AppSchemaIDs separated by lines. Press Ctrl+E to show completer.");
	impactAppSignalIdsEdit->setPlainText(impactAppSignalIds);
	impactAppSignalIdsEdit->setCompleter(completer);

	// Tags
	//
	QString tags = signalItem->tagsAsList().join(QChar(' '));

	QLabel* tagsLabel = new QLabel("SchemaItem Tags:", &d);

	QLineEdit* tagsEdit = new QLineEdit(&d);
	tagsEdit->setPlaceholderText("Enter tags separated by spaces.");
	tagsEdit->setText(tags);

	QPushButton* tagsEditorButton = new QPushButton(tr("Tags..."), &d);
	tagsEditorButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);

	connect(
		tagsEditorButton,
		&QPushButton::clicked,
		[this, &d, tagsEdit]()
		{
			ResizedDialog tagsSelectorDialog{tr("Tags"), &d, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint};

			std::unique_ptr<DbChooseItemsDialog> te(DbChooseItemsDialog::tagsEditor(this->m_db, &d));
			te->setText(tagsEdit->text());

			connect(te.get(), &DbChooseItemsDialog::okPressed, &tagsSelectorDialog, &QDialog::accept);
			connect(te.get(), &DbChooseItemsDialog::cancelPressed, &tagsSelectorDialog, &QDialog::reject);

			QHBoxLayout l;
			l.addWidget(te.get());
			tagsSelectorDialog.setLayout(&l);

			if (tagsSelectorDialog.exec() == QDialog::Accepted)
			{
				tagsEdit->setText(te->text());
			}
		});

	// --
	//
	QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &d);
	buttonBox->button(QDialogButtonBox::Ok)->setDefault(true);

	QGridLayout* layout = new QGridLayout(&d);

	layout->addWidget(appSignalIdsLabel, 0, 0, 1, 4);
	layout->addWidget(appSignalIdsEdit, 1, 0, 1, 4);

	layout->addWidget(impactAppSignalIdsLabel, 2, 0, 1, 4);
	layout->addWidget(impactAppSignalIdsEdit, 3, 0, 1, 4);

	layout->addWidget(tagsLabel, 4, 0, 1, 3);
	layout->addWidget(tagsEdit, 5, 0, 1, 3);
	layout->addWidget(tagsEditorButton, 5, 3, 1, 1);

	layout->addWidget(buttonBox, 6, 0, 1, 4);

	d.setLayout(layout);

	connect(buttonBox, &QDialogButtonBox::accepted, &d, &QDialog::accept);
	connect(buttonBox, &QDialogButtonBox::rejected, &d, &QDialog::reject);

	// --
	//
	int result = d.exec();

	if (result == QDialog::Accepted)
	{
		QString newAppSignaIds = appSignalIdsEdit->toPlainText();
		QString newImpactAppSignaIds = impactAppSignalIdsEdit->toPlainText();
		QString newTags = tagsEdit->text();

		if (newAppSignaIds != appSignalIds || newImpactAppSignaIds != impactAppSignalIds || newTags.trimmed() != tags.trimmed())
		{
			if (bool ok = m_editEngine->startBatch(); ok == true)
			{
				m_editEngine->runSetProperty(VFrame30::PropertyNames::appSignalIDs, QVariant(newAppSignaIds), item);
				m_editEngine->runSetProperty(VFrame30::PropertyNames::impactAppSignalIDs, QVariant(newImpactAppSignaIds), item);
				m_editEngine->runSetProperty(VFrame30::PropertyNames::tags, QVariant(newTags), item);

				m_editEngine->endBatch();
			}
		}
	}

	return;
}

void F2KeyForSchemaItem::f2KeyForLoopback(SchemaItemPtr item)
{
	if (item == nullptr)
	{
		Q_ASSERT(item);
		return;
	}

	VFrame30::SchemaItemLoopback* loopbackItem = dynamic_cast<VFrame30::SchemaItemLoopback*>(item.get());
	if (loopbackItem == nullptr)
	{
		Q_ASSERT(loopbackItem);
		return;
	}

	QString loopbackId = loopbackItem->loopbackId();

	// Show input dialog
	//
	QInputDialog d{m_parent};

	d.setWindowFlags((d.windowFlags() & ~Qt::WindowMinimizeButtonHint & ~Qt::WindowMaximizeButtonHint & ~Qt::WindowContextHelpButtonHint) |
					 Qt::CustomizeWindowHint);

	d.setWindowTitle(tr("Set LoopbackID"));
	d.setLabelText(tr("LoopbackID:"));
	d.setInputMode(QInputDialog::TextInput);
	d.setTextValue(loopbackId);

	int width = QSettings().value("f2KeyForLoopback\\width").toInt();
	int height = QSettings().value("f2KeyForLoopback\\height").toInt();
	d.resize(width, height);

	if (int result = d.exec(); result == QDialog::Accepted)
	{
		QString newValue = d.textValue();

		if (newValue.isEmpty() == false && newValue != loopbackId)
		{
			m_editEngine->runSetProperty(VFrame30::PropertyNames::loopbackId, QVariant(newValue), item);
			m_view->update();

			F2KeyForSchemaItem::m_lastUsedLoopbackId = newValue;
		}
	}

	QSettings().setValue("f2KeyForLoopback\\width", d.width());
	QSettings().setValue("f2KeyForLoopback\\height", d.height());

	return;
}

void F2KeyForSchemaItem::f2KeyForValue(SchemaItemPtr item)
{
	if (item == nullptr)
	{
		assert(item);
		return;
	}

	VFrame30::SchemaItemValue* valueItem = dynamic_cast<VFrame30::SchemaItemValue*>(item.get());
	if (valueItem == nullptr)
	{
		assert(valueItem);
		return;
	}

	QString appSignalIds = valueItem->signalIdsString(nullptr);
	QString text = valueItem->text();
	QString preDrawScript = valueItem->preDrawScript();

	// Show input dialog
	//
	ResizedDialog d(tr("SchemaItemValue"), m_parent);

	d.setWindowFlags((d.windowFlags() & ~Qt::WindowMinimizeButtonHint & ~Qt::WindowContextHelpButtonHint) | Qt::CustomizeWindowHint |
					 Qt::WindowMaximizeButtonHint);

	// AppSchemaIDs
	//
	QLabel* appSignalIdsLabel = new QLabel("AppSchemaIDs:", &d);

	QPlainTextEdit* appSignalIdsEdit = new QPlainTextEdit(&d);
	appSignalIdsEdit->setPlaceholderText("Enter AppSchemaIDs separated by lines");
	appSignalIdsEdit->setPlainText(appSignalIds);

	auto appSingalPad = new QWidget;
	auto appSingalPadLayout = new QVBoxLayout;
	appSingalPadLayout->setContentsMargins(0, 0, 0, 0);
	appSingalPad->setLayout(appSingalPadLayout);

	appSingalPadLayout->addWidget(appSignalIdsLabel);
	appSingalPadLayout->addWidget(appSignalIdsEdit);

	// Text
	//
	QLabel* textLabel = new QLabel("Text:", &d);
	textLabel->setToolTip(VFrame30::PropertyNames::textValuePropDescription);

	QPlainTextEdit* textEdit = new QPlainTextEdit(&d);
	textEdit->setPlaceholderText(VFrame30::PropertyNames::textValuePropDescription);
	textEdit->setToolTip(VFrame30::PropertyNames::textValuePropDescription);
	textEdit->setPlainText(text);

	auto textPad = new QWidget;
	auto textPadLayout = new QVBoxLayout;
	textPadLayout->setContentsMargins(0, 0, 0, 0);
	textPad->setLayout(textPadLayout);

	textPadLayout->addWidget(textLabel);
	textPadLayout->addWidget(textEdit);

	// PreDrawScript
	//
	QLabel* preDrawScriptLabel = new QLabel("PreDrawScript:", &d);

	auto preDrawScriptEdit = new UiLib::CodeEditor(&d);
	UiLib::JsHighlighter::createJsHighlighter(preDrawScriptEdit);

	preDrawScriptEdit->setText(preDrawScript);

#if defined(Q_OS_WIN)
	preDrawScriptEdit->setFont(QFont("Consolas"));
#else
	preDrawScriptEdit->setFont(QFont("Courier"));
#endif

	QPushButton* preDrawScriptTemplate = new QPushButton(tr("Paste PreDrawScript Template"), &d);

	auto drawScriptPad = new QWidget;
	auto drawScriptPadLayout = new QGridLayout;
	drawScriptPadLayout->setContentsMargins(0, 0, 0, 0);
	drawScriptPad->setLayout(drawScriptPadLayout);

	drawScriptPadLayout->addWidget(preDrawScriptLabel, 4, 0, 1, 3);
	drawScriptPadLayout->addWidget(preDrawScriptEdit, 5, 0, 1, 3);
	drawScriptPadLayout->addWidget(preDrawScriptTemplate, 6, 0, 1, 1);

	// Add horizontal splitter.
	// Upper part is for appSignalIdsLabel, appSignalIdsEdit, textLabel, textLabel
	// Lower part is for preDrawScriptLabel, preDrawScriptEdit, preDrawScriptTemplate
	//
	QSplitter* splitter = new QSplitter(Qt::Vertical, &d);
	splitter->setChildrenCollapsible(false);

	splitter->addWidget(appSingalPad);
	splitter->addWidget(textPad);
	splitter->addWidget(drawScriptPad);

	// --
	//
	QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &d);

	// Set main layout
	//
	QGridLayout* layout = new QGridLayout(&d);

	layout->addWidget(splitter, 1, 0, 1, 3);
	layout->addWidget(buttonBox, 7, 0, 1, 3);

	d.setLayout(layout);

	// Restore splitter state.
	//
	QByteArray splitterState = QSettings().value("f2KeyForValue\\splitterState").toByteArray();
	if (splitterState.isEmpty() == false)
	{
		splitter->restoreState(splitterState);
	}

	// RAW STRING TEMPLATE FOR PreDrawScript
	//
	QString preDrawScriptTemplateString = R"((function(schemaItemValue)
{
	// let appSignalId = schemaItemValue.signalIDs[0];

	// Get data from AppDataService
	// let signalParam = signals.signalParam(appSignalId);
	// let signalState = signals.signalState(appSignalId);

	// Get data from TuningService
	// let signalParam = tuning.signalParam(appSignalId);
	// let signalState = tuning.signalState(appSignalId);

	// Get signal state
	// if (signalState.Valid == true)
	// {
	//		schemaItemValue.text = signalState.value;
	//		schemaItemValue.textColor = "white";
	//		schemaItemValue.fillColor = schemaItemValue.blinkPhase ? "black" : "#A00000";
	//		schemaItemValue.lineColor = "#000000";
	// }
}))";

	connect(preDrawScriptTemplate,
			&QPushButton::clicked,
			&d,
			[preDrawScriptEdit, preDrawScriptTemplateString]()
			{
				preDrawScriptEdit->setText(preDrawScriptTemplateString);
			});

	connect(buttonBox, &QDialogButtonBox::accepted, &d, &QDialog::accept);
	connect(buttonBox, &QDialogButtonBox::rejected, &d, &QDialog::reject);

	// --
	//
	int result = d.exec();

	if (result == QDialog::Accepted)
	{
		QString newAppSignaIds = appSignalIdsEdit->toPlainText();
		QString newText = textEdit->toPlainText();
		QString newPreDrawScript = preDrawScriptEdit->text();

		if (newAppSignaIds != appSignalIds || newText != text || newPreDrawScript != preDrawScript)
		{
			if (bool ok = m_editEngine->startBatch(); ok == true)
			{
				m_editEngine->runSetProperty(VFrame30::PropertyNames::appSignalIDs, QVariant(newAppSignaIds), item);
				m_editEngine->runSetProperty(VFrame30::PropertyNames::text, QVariant(newText), item);
				m_editEngine->runSetProperty(VFrame30::PropertyNames::preDrawScript, QVariant(newPreDrawScript), item);

				m_editEngine->endBatch();
			}
		}
	}

	// Save splitter state for next time.
	//
	QSettings().setValue("f2KeyForValue\\splitterState", splitter->saveState());

	return;
}

void F2KeyForSchemaItem::f2KeyForImageValue(SchemaItemPtr item)
{
	if (item == nullptr)
	{
		assert(item);
		return;
	}

	VFrame30::SchemaItemImageValue* valueItem = dynamic_cast<VFrame30::SchemaItemImageValue*>(item.get());
	if (valueItem == nullptr)
	{
		assert(valueItem);
		return;
	}

	QString appSignalIds = valueItem->signalIdsString(nullptr);
	QString currentImageId = valueItem->currentImageId();
	QString preDrawScript = valueItem->preDrawScript();

	// Show input dialog
	//
	ResizedDialog d(tr("SchemaItemImageValue"), m_parent);

	d.setWindowFlags((d.windowFlags() & ~Qt::WindowMinimizeButtonHint & ~Qt::WindowContextHelpButtonHint) | Qt::CustomizeWindowHint |
					 Qt::WindowMaximizeButtonHint);

	// AppSchemaIDs
	//
	QLabel* appSignalIdsLabel = new QLabel("AppSchemaIDs:", &d);

	QTextEdit* appSignalIdsEdit = new QTextEdit(&d);
	appSignalIdsEdit->setPlaceholderText("Enter AppSchemaIDs separated by lines");
	appSignalIdsEdit->setPlainText(appSignalIds);

	// CurrentImageID
	//
	QLabel* currentImageIdLabel = new QLabel("CurrentImageID:", &d);

	QLineEdit* currentImageIdEdit = new QLineEdit(&d);
	currentImageIdEdit->setText(currentImageId);

	// PreDrawScript
	//
	QLabel* preDrawScriptLabel = new QLabel("PreDrawScript:", &d);

	auto preDrawScriptEdit = new UiLib::CodeEditor(&d);
	UiLib::JsHighlighter::createJsHighlighter(preDrawScriptEdit);

	preDrawScriptEdit->setText(preDrawScript);

#if defined(Q_OS_WIN)
	preDrawScriptEdit->setFont(QFont("Consolas"));
#else
	preDrawScriptEdit->setFont(QFont("Courier"));
#endif

	QPushButton* preDrawScriptTemplate = new QPushButton(tr("Paste PreDrawScript Template"), &d);

	// --
	//
	QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &d);

	QGridLayout* layout = new QGridLayout(&d);

	layout->addWidget(appSignalIdsLabel, 0, 0, 1, 3);
	layout->addWidget(appSignalIdsEdit, 1, 0, 1, 3);

	layout->addWidget(currentImageIdLabel, 2, 0, 1, 3);
	layout->addWidget(currentImageIdEdit, 3, 0, 1, 3);

	layout->addWidget(preDrawScriptLabel, 4, 0, 1, 3);
	layout->addWidget(preDrawScriptEdit, 5, 0, 1, 3);
	layout->addWidget(preDrawScriptTemplate, 6, 0, 1, 1);

	layout->addWidget(buttonBox, 7, 0, 1, 3);

	layout->setRowStretch(5, 1); // preDrawScriptEdit

	d.setLayout(layout);

	// RAW STRING TEMPLATE FOR PreDrawScript
	//
	QString preDrawScriptTemplateString = R"((function(schemaItemImageValue)
{
	// Get signal id by index from schema item
	let appSignalId = schemaItemImageValue.signalIDs[0];

	// Get data from AppDataService or TuningService sources
	// let signalState = tuning.signalState(appSignalId);
	let signalState = signals.signalState(appSignalId);

	if (signalState.Valid == false)
	{
		schemaItemImageValue.currentImageID = "IMAGEID_NOT_VALID";
		return;
	}

	if (signalState.Value == 0)
		schemaItemImageValue.currentImageID = "IMAGEID_OFF";
	else
		schemaItemImageValue.currentImageID = "IMAGEID_ON";
}))";

	connect(preDrawScriptTemplate,
			&QPushButton::clicked,
			&d,
			[preDrawScriptEdit, preDrawScriptTemplateString]()
			{
				preDrawScriptEdit->setText(preDrawScriptTemplateString);
			});

	connect(buttonBox, &QDialogButtonBox::accepted, &d, &QDialog::accept);
	connect(buttonBox, &QDialogButtonBox::rejected, &d, &QDialog::reject);

	// --
	//
	int result = d.exec();

	if (result == QDialog::Accepted)
	{
		QString newAppSignaIds = appSignalIdsEdit->toPlainText();
		QString newCurrentImageId = currentImageIdEdit->text();
		QString newPreDrawScript = preDrawScriptEdit->text();

		if (newAppSignaIds != appSignalIds || newCurrentImageId != currentImageId || newPreDrawScript != preDrawScript)
		{
			if (bool ok = m_editEngine->startBatch(); ok == true)
			{
				m_editEngine->runSetProperty(VFrame30::PropertyNames::appSignalIDs, QVariant(newAppSignaIds), item);
				m_editEngine->runSetProperty(VFrame30::PropertyNames::currentImageId, QVariant(newCurrentImageId), item);
				m_editEngine->runSetProperty(VFrame30::PropertyNames::preDrawScript, QVariant(newPreDrawScript), item);

				m_editEngine->endBatch();
			}
		}
	}

	return;
}


void F2KeyForSchemaItem::f2KeyForBus(SchemaItemPtr item)
{
	if (item == nullptr)
	{
		assert(item);
		return;
	}

	VFrame30::SchemaItemBus* busItem = dynamic_cast<VFrame30::SchemaItemBus*>(item.get());
	if (busItem == nullptr)
	{
		assert(busItem);
		return;
	}

	QString text = busItem->busTypeId();

	// Get Bus list
	//
	std::vector<AppSignalLib::Bus> busses;

	bool ok = loadBusses(m_db, &busses, m_parent);

	if (ok == false)
	{
		return;
	}

	// Show input dialog
	//
	ResizedDialog d(tr("Set BusType"), m_parent);

	d.setWindowFlags((d.windowFlags() & ~Qt::WindowMinimizeButtonHint & ~Qt::WindowMaximizeButtonHint & ~Qt::WindowContextHelpButtonHint) |
					 Qt::CustomizeWindowHint);

	// Type Items
	//
	QLabel* busTypeLabel = new QLabel("Select BusType:");

	QComboBox* busTypeCombo = new QComboBox();

	for (int i = 0; i < static_cast<int>(busses.size()); i++)
	{
		busTypeCombo->addItem(busses[i].busTypeId(), QVariant(i));
	}

	int dataIndex = busTypeCombo->findText(text);
	if (dataIndex != -1)
	{
		busTypeCombo->setCurrentIndex(dataIndex);
	}

	// Spacer
	//
	QWidget* spacer = new QWidget;
	spacer->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);

	// --
	//
	QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

	QVBoxLayout* layout = new QVBoxLayout;

	layout->addWidget(busTypeLabel);
	layout->addWidget(busTypeCombo);
	layout->addWidget(spacer);
	layout->addWidget(buttonBox);

	d.setLayout(layout);

	connect(buttonBox, &QDialogButtonBox::accepted, &d, &QDialog::accept);
	connect(buttonBox, &QDialogButtonBox::rejected, &d, &QDialog::reject);

	// --
	//
	int result = d.exec();

	if (result == QDialog::Accepted && text != busTypeCombo->currentText())
	{
		int selectedBusIndex = busTypeCombo->currentData().toInt();
		const AppSignalLib::Bus& newBus = busses[selectedBusIndex];

		QByteArray oldState;
		busItem->saveToByteArray(&oldState);

		busItem->setBusType(newBus);

		QByteArray newState;
		busItem->saveToByteArray(&newState);

		// Return object to prev state, it is not neccessary indeed, as it will be loaded into the new state in edit engine
		//
		busItem->Load(oldState);

		// Run command
		//
		m_editEngine->runSetObject(oldState, newState, item);

		m_view->update();
	}

	return;
}

void F2KeyForSchemaItem::f2KeyForAfb(SchemaItemPtr item)
{
	if (item->isSchemaItemAfb() == false)
	{
		assert(item->isSchemaItemAfb() == true);
		return;
	}

	// Create a deep copy of the item, as it must not share Properties with the original one.
	//
	QByteArray itemData;
	item->saveToByteArray(&itemData);

	auto itemCopy = VFrame30::SchemaItem::Create(itemData);
	Q_ASSERT(itemCopy);

	// Only edit essential and category Parameters,
	// remove all others.
	//
	auto properties = itemCopy->properties();

	for (auto& property : properties)
	{
		if (property->essential() != true && property->category() != VFrame30::PropertyNames::parametersCategory)
		{
			itemCopy->removeProperty(property->caption());
		}
	}

	properties = itemCopy->properties();

	// Show input dialog.
	//
	ResizedDialog d(tr("Set AFB properties"), m_parent);

	d.setWindowFlags((d.windowFlags() & ~Qt::WindowMinimizeButtonHint & ~Qt::WindowMaximizeButtonHint & ~Qt::WindowContextHelpButtonHint) |
					 Qt::CustomizeWindowHint);

	auto label = new QLabel("AFB Item Properties:");

	auto propertyEditor = new ExtWidgets::PropertyEditor{&d};
	propertyEditor->setObject(itemCopy);

	QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

	QVBoxLayout* layout = new QVBoxLayout;
	layout->addWidget(label);
	layout->addWidget(propertyEditor);
	layout->addWidget(buttonBox);
	d.setLayout(layout);

	connect(buttonBox, &QDialogButtonBox::accepted, &d, &QDialog::accept);
	connect(buttonBox, &QDialogButtonBox::rejected, &d, &QDialog::reject);

	propertyEditor->autoAdjustSplitterPosition();
	propertyEditor->setReadOnly(m_editEngine->readOnly());

	// Show modal dialog.
	//
	if (int result = d.exec(); result != QDialog::Accepted)
	{
		return;
	}

	// Update only changed properties
	//
	bool batchWasStarted = false;

	for (const auto& property : properties)
	{
		auto oldProperty = item->propertyByCaption(property->caption());
		if (oldProperty == nullptr)
		{
			Q_ASSERT(oldProperty);
			continue;
		}

		if (oldProperty->value() == property->value())
		{
			// Property value was not changed, skip it.
			//
			continue;
		}

		if (batchWasStarted == false)
		{
			// This is the first changed property, start batch.
			//
			batchWasStarted = m_editEngine->startBatch();

			if (batchWasStarted == false)
			{
				// Batch was not started, skip all changes.
				// It is possible if the schema is read only.
				//
				break;
			}
		}

		m_editEngine->runSetProperty(property->caption(), property->value(), item);
	}

	if (batchWasStarted == true)
	{
		// If batch was started, then at least one property has been changed.
		//
		m_editEngine->endBatch();
	}

	return;
}

void F2KeyForSchemaItem::f2KeyForVduValue(SchemaItemPtr item)
{
	VFrame30::SchemaItemVduValue* valueItem = dynamic_cast<VFrame30::SchemaItemVduValue*>(item.get());
	if (valueItem == nullptr)
	{
		assert(valueItem);
		return;
	}

	QString objectName = valueItem->objectName();
	QString appSignalIds = valueItem->appSignalIdsString();
	QString text = valueItem->text();
	QString preDrawScript = valueItem->preDrawScript();

	// Show input dialog
	//
	ResizedDialog d(tr("SchemaItemVduValue"), m_parent);

	d.setWindowFlags((d.windowFlags() & ~Qt::WindowMinimizeButtonHint & ~Qt::WindowContextHelpButtonHint) | Qt::CustomizeWindowHint |
					 Qt::WindowMaximizeButtonHint);

	QGridLayout* layout = new QGridLayout(&d);

	// ObjectName
	//
	QLabel* objectNameEditLabel = new QLabel{"ObjectName:", &d};
	objectNameEditLabel->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);

	auto objectNameEdit = new QLineEdit{&d};
	objectNameEdit->setText(objectName);

	layout->addWidget(objectNameEditLabel, 0, 0);
	layout->addWidget(objectNameEdit, 0, 1, 1, 2);

	// Text and scripts
	//
	QSplitter* splitter = new QSplitter(Qt::Vertical, &d);
	splitter->setChildrenCollapsible(false);

	auto createControlsFunc = [&d, splitter, item]<typename EditType>(QString propertyName, QString defaultScript) -> EditType*
	{
		auto property = item->propertyByCaption(propertyName);
		if (property == nullptr)
		{
			assert(property);
			return nullptr;
		}

		auto edit = new EditType{&d};

		// Check if template argument EditType is ::UiLib::CodeEditor, then call createJsHighlighter
		//
		if constexpr (std::is_same_v<EditType, UiLib::CodeEditor>)
		{
			edit->setText(property->value().toString());
			UiLib::JsHighlighter::createJsHighlighter(edit);
#if defined(Q_OS_WIN)
			edit->setFont(QFont("Consolas"));
#else
			edit->setFont(QFont("Courier"));
#endif
		}
		else
		{
			edit->setPlainText(property->value().toString());
		}

		edit->setToolTip(property->description() + "\n" + defaultScript);

		auto pad = new QWidget;
		auto padLayout = new QGridLayout;
		padLayout->setContentsMargins(0, 0, 0, 0);
		pad->setLayout(padLayout);

		padLayout->addWidget(new QLabel(propertyName + ":", &d), 0, 0, 1, 3);
		padLayout->addWidget(edit, 1, 0, 1, 3);

		splitter->addWidget(pad);
		return edit;
	};

	auto appSignalIdsEdit = createControlsFunc.operator()<QPlainTextEdit>(VFrame30::PropertyNames::appSignalIDs, "");
	auto textEdit =
		createControlsFunc.operator()<QPlainTextEdit>(VFrame30::PropertyNames::text, VFrame30::PropertyNames::textVduItemValueDescription);

	auto preDrawScriptEdit = createControlsFunc.operator()<UiLib::CodeEditor>(VFrame30::PropertyNames::preDrawScript, "");

	// --
	//
	QDialogButtonBox* buttonBox = new QDialogButtonBox{QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &d};

	// Set main layout
	//
	layout->addWidget(splitter, 1, 0, 1, 3);
	layout->addWidget(buttonBox, 7, 0, 1, 3);

	d.setLayout(layout);

	// Restore splitter state.
	//
	QByteArray splitterState = QSettings().value("f2KeyForVduValue\\splitterState").toByteArray();
	if (splitterState.isEmpty() == false)
	{
		splitter->restoreState(splitterState);
	}

	connect(buttonBox, &QDialogButtonBox::accepted, &d, &QDialog::accept);
	connect(buttonBox, &QDialogButtonBox::rejected, &d, &QDialog::reject);

	// --
	//
	int result = d.exec();

	if (result == QDialog::Accepted)
	{
		QString newObjectName = objectNameEdit->text();
		QString newAppSignaIds = appSignalIdsEdit->toPlainText();
		QString newText = textEdit->toPlainText();
		QString newPreDrawScript = preDrawScriptEdit->text();

		if (newObjectName != objectName || newAppSignaIds != appSignalIds || newText != text || newPreDrawScript != preDrawScript)
		{
			if (bool ok = m_editEngine->startBatch(); ok == true)
			{
				m_editEngine->runSetProperty(VFrame30::PropertyNames::objectName, QVariant(newObjectName), item);
				m_editEngine->runSetProperty(VFrame30::PropertyNames::appSignalIDs, QVariant(newAppSignaIds), item);
				m_editEngine->runSetProperty(VFrame30::PropertyNames::text, QVariant(newText), item);
				m_editEngine->runSetProperty(VFrame30::PropertyNames::preDrawScript, QVariant(newPreDrawScript), item);

				m_editEngine->endBatch();
			}
		}
	}

	// Save splitter state for next time.
	//
	QSettings().setValue("f2KeyForVduValue\\splitterState", splitter->saveState());

	return;
}

void F2KeyForSchemaItem::f2KeyForVduRect(SchemaItemPtr item)
{
	auto rectItem = dynamic_cast<VFrame30::SchemaItemVduRect*>(item.get());
	if (rectItem == nullptr)
	{
		assert(rectItem);
		return;
	}

	QString objectName = rectItem->objectName();
	QString text = rectItem->text();
	QString clickScript = rectItem->clickScript();
	QString preDrawScript = rectItem->preDrawScript();

	// Show input dialog
	//
	ResizedDialog d{tr("SchemaItemVduRect"), m_parent};
	d.setWindowFlags((d.windowFlags() & ~Qt::WindowMinimizeButtonHint & ~Qt::WindowContextHelpButtonHint) | Qt::CustomizeWindowHint |
					 Qt::WindowMaximizeButtonHint);

	QGridLayout* layout = new QGridLayout{&d};

	// ObjectName
	//
	QLabel* objectNameEditLabel = new QLabel{"ObjectName:", &d};
	objectNameEditLabel->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);

	auto objectNameEdit = new QLineEdit{&d};
	objectNameEdit->setText(objectName);

	layout->addWidget(objectNameEditLabel, 0, 0);
	layout->addWidget(objectNameEdit, 0, 1, 1, 2);

	// Text and scripts
	//
	QSplitter* splitter = new QSplitter(Qt::Vertical, &d);
	splitter->setChildrenCollapsible(false);

	auto createControlsFunc = [&d, splitter, item]<typename EditType>(QString propertyName, QString defaultScript) -> EditType*
	{
		auto property = item->propertyByCaption(propertyName);
		if (property == nullptr)
		{
			assert(property);
			return nullptr;
		}

		auto edit = new EditType{&d};

		// Check if template argument EditType is ::UiLib::CodeEditor, then call createJsHighlighter
		//
		if constexpr (std::is_same_v<EditType, UiLib::CodeEditor>)
		{
			edit->setText(property->value().toString());
			UiLib::JsHighlighter::createJsHighlighter(edit);
#if defined(Q_OS_WIN)
			edit->setFont(QFont("Consolas"));
#else
			edit->setFont(QFont("Courier"));
#endif
		}
		else
		{
			edit->setPlainText(property->value().toString());
		}

		edit->setToolTip(property->description() + "\n" + defaultScript);

		auto pad = new QWidget;
		auto padLayout = new QGridLayout;
		padLayout->setContentsMargins(0, 0, 0, 0);
		pad->setLayout(padLayout);

		padLayout->addWidget(new QLabel(propertyName + ":", &d), 0, 0, 1, 3);
		padLayout->addWidget(edit, 1, 0, 1, 3);

		splitter->addWidget(pad);
		return edit;
	};

	auto textEdit = createControlsFunc.operator()<QPlainTextEdit>(VFrame30::PropertyNames::text, "");
	auto clickScriptEdit = createControlsFunc.operator()<UiLib::CodeEditor>(VFrame30::PropertyNames::clickScript, "");
	auto preDrawEdit = createControlsFunc.operator()<UiLib::CodeEditor>(VFrame30::PropertyNames::preDrawScript, "");

	// --
	//
	QDialogButtonBox* buttonBox = new QDialogButtonBox{QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &d};

	// Set main layout
	//
	layout->addWidget(splitter, 1, 0, 1, 3);
	layout->addWidget(buttonBox, 7, 0, 1, 3);

	d.setLayout(layout);

	// Restore splitter state.
	//
	QByteArray splitterState = QSettings().value("f2KeyForVduRect\\splitterState").toByteArray();
	if (splitterState.isEmpty() == false)
	{
		splitter->restoreState(splitterState);
	}

	connect(buttonBox, &QDialogButtonBox::accepted, &d, &QDialog::accept);
	connect(buttonBox, &QDialogButtonBox::rejected, &d, &QDialog::reject);

	// --
	//
	int result = d.exec();

	if (result == QDialog::Accepted)
	{
		QString newObjectName = objectNameEdit->text();
		QString newText = textEdit->toPlainText();
		QString newClickScript = clickScriptEdit->toPlainText();
		QString newPreDrawScript = preDrawEdit->toPlainText();

		if (newObjectName != objectName || newText != text || newClickScript != clickScript || newPreDrawScript != preDrawScript)
		{
			if (bool ok = m_editEngine->startBatch(); ok == true)
			{
				m_editEngine->runSetProperty(VFrame30::PropertyNames::objectName, QVariant(newObjectName), item);
				m_editEngine->runSetProperty(VFrame30::PropertyNames::clickScript, QVariant(newClickScript), item);
				m_editEngine->runSetProperty(VFrame30::PropertyNames::text, QVariant(newText), item);
				m_editEngine->runSetProperty(VFrame30::PropertyNames::preDrawScript, QVariant(newPreDrawScript), item);

				m_editEngine->endBatch();
			}
		}
	}

	// Save splitter state for next time.
	//
	QSettings().setValue("f2KeyForVduRect\\splitterState", splitter->saveState());

	return;
}

void F2KeyForSchemaItem::f2KeyForVduImageValue(SchemaItemPtr item)
{
	auto valueItem = dynamic_cast<VFrame30::SchemaItemVduImageValue*>(item.get());
	if (valueItem == nullptr)
	{
		assert(valueItem);
		return;
	}

	QString objectName = valueItem->objectName();
	QString appSignalIds = valueItem->signalIdsString();
	QString clickScript = valueItem->clickScript();
	QString preDrawScript = valueItem->preDrawScript();

	// Show input dialog
	//
	ResizedDialog d{tr("SchemaItemVduImageValue"), m_parent};

	d.setWindowFlags((d.windowFlags() & ~Qt::WindowMinimizeButtonHint & ~Qt::WindowContextHelpButtonHint) | Qt::CustomizeWindowHint |
					 Qt::WindowMaximizeButtonHint);

	QGridLayout* layout = new QGridLayout{&d};

	// ObjectName
	//
	QLabel* objectNameEditLabel = new QLabel{"ObjectName:", &d};
	objectNameEditLabel->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);

	auto objectNameEdit = new QLineEdit{&d};
	objectNameEdit->setText(objectName);

	layout->addWidget(objectNameEditLabel, 0, 0);
	layout->addWidget(objectNameEdit, 0, 1, 1, 2);

	// Text and scripts
	//
	QSplitter* splitter = new QSplitter(Qt::Vertical, &d);
	splitter->setChildrenCollapsible(false);

	auto createControlsFunc = [&d, splitter, item]<typename EditType>(QString propertyName, QString defaultScript) -> EditType*
	{
		auto property = item->propertyByCaption(propertyName);
		if (property == nullptr)
		{
			assert(property);
			return nullptr;
		}

		auto edit = new EditType{&d};

		// Check if template argument EditType is ::UiLib::CodeEditor, then call createJsHighlighter
		//
		if constexpr (std::is_same_v<EditType, UiLib::CodeEditor>)
		{
			edit->setText(property->value().toString());
			UiLib::JsHighlighter::createJsHighlighter(edit);
#if defined(Q_OS_WIN)
			edit->setFont(QFont("Consolas"));
#else
			edit->setFont(QFont("Courier"));
#endif
		}
		else
		{
			edit->setPlainText(property->value().toString());
		}

		edit->setToolTip(property->description() + "\n" + defaultScript);

		auto pad = new QWidget;
		auto padLayout = new QGridLayout;
		padLayout->setContentsMargins(0, 0, 0, 0);
		pad->setLayout(padLayout);

		padLayout->addWidget(new QLabel(propertyName + ":", &d), 0, 0, 1, 3);
		padLayout->addWidget(edit, 1, 0, 1, 3);

		splitter->addWidget(pad);
		return edit;
	};

	auto appSignalIdsEdit = createControlsFunc.operator()<QPlainTextEdit>(VFrame30::PropertyNames::appSignalIDs, "");
	auto clickScriptEdit = createControlsFunc.operator()<UiLib::CodeEditor>(VFrame30::PropertyNames::clickScript, "");
	auto preDrawScriptEdit = createControlsFunc.operator()<UiLib::CodeEditor>(VFrame30::PropertyNames::preDrawScript, "");

	// --
	//
	QDialogButtonBox* buttonBox = new QDialogButtonBox{QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &d};

	// Set main layout
	//
	layout->addWidget(splitter, 1, 0, 1, 3);
	layout->addWidget(buttonBox, 7, 0, 1, 3);

	d.setLayout(layout);

	// Restore splitter state.
	//
	QByteArray splitterState = QSettings().value("f2KeyForVduImageValue\\splitterState").toByteArray();
	if (splitterState.isEmpty() == false)
	{
		splitter->restoreState(splitterState);
	}

	connect(buttonBox, &QDialogButtonBox::accepted, &d, &QDialog::accept);
	connect(buttonBox, &QDialogButtonBox::rejected, &d, &QDialog::reject);

	// --
	//
	int result = d.exec();

	if (result == QDialog::Accepted)
	{
		QString newObjectName = objectNameEdit->text();
		QString newAppSignaIds = appSignalIdsEdit->toPlainText();
		QString newClickScript = clickScriptEdit->toPlainText();
		QString newPreDrawScript = preDrawScriptEdit->text();

		if (newObjectName != objectName || newAppSignaIds != appSignalIds || newClickScript != clickScript ||
			newPreDrawScript != preDrawScript)
		{
			if (bool ok = m_editEngine->startBatch(); ok == true)
			{
				m_editEngine->runSetProperty(VFrame30::PropertyNames::objectName, QVariant(newObjectName), item);
				m_editEngine->runSetProperty(VFrame30::PropertyNames::appSignalIDs, QVariant(newAppSignaIds), item);
				m_editEngine->runSetProperty(VFrame30::PropertyNames::clickScript, QVariant(newClickScript), item);
				m_editEngine->runSetProperty(VFrame30::PropertyNames::preDrawScript, QVariant(newPreDrawScript), item);

				m_editEngine->endBatch();
			}
		}
	}

	// Save splitter state for next time.
	//
	QSettings().setValue("f2KeyForVduImageValue\\splitterState", splitter->saveState());

	return;
}

void F2KeyForSchemaItem::f2KeyForPushButton(SchemaItemPtr item)
{
	auto buttonItem = dynamic_cast<VFrame30::SchemaItemPushButton*>(item.get());
	if (buttonItem == nullptr)
	{
		assert(buttonItem);
		return;
	}

	QString objectName = item->objectName();
	QString text = buttonItem->text();
	QString scriptClicked = buttonItem->scriptClicked();
	QString preDrawScript = buttonItem->preDrawScript();

	// Show input dialog
	//
	ResizedDialog d{tr("SchemaItemPushButton"), m_parent};
	d.setWindowFlags((d.windowFlags() & ~Qt::WindowMinimizeButtonHint & ~Qt::WindowContextHelpButtonHint) | Qt::CustomizeWindowHint |
					 Qt::WindowMaximizeButtonHint);

	QGridLayout* layout = new QGridLayout{&d};

	// ObjectName
	//
	QLabel* objectNameEditLabel = new QLabel{"ObjectName:", &d};
	objectNameEditLabel->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);

	auto objectNameEdit = new QLineEdit{&d};
	objectNameEdit->setText(objectName);

	layout->addWidget(objectNameEditLabel, 0, 0);
	layout->addWidget(objectNameEdit, 0, 1, 1, 2);

	// Text
	//
	QLabel* textLabel = new QLabel{"Text:", &d};
	textLabel->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);

	auto textEdit = new QLineEdit{&d};
	textEdit->setText(text);

	layout->addWidget(textLabel, 1, 0);
	layout->addWidget(textEdit, 1, 1, 1, 2);

	// Scripts
	//
	QSplitter* splitter = new QSplitter(Qt::Vertical, &d);
	splitter->setChildrenCollapsible(false);

	auto createScriptControls = [&d, splitter, item](QString propertyName, QString defaultScript) -> UiLib::CodeEditor*
	{
		auto property = item->propertyByCaption(propertyName);
		if (property == nullptr)
		{
			assert(property);
			return nullptr;
		}

		auto edit = new UiLib::CodeEditor(&d);
		UiLib::JsHighlighter::createJsHighlighter(edit);

#if defined(Q_OS_WIN)
		edit->setFont(QFont("Consolas"));
#else
		edit->setFont(QFont("Courier"));
#endif
		edit->setText(property->value().toString());
		edit->setToolTip(property->description() + "\n" + defaultScript);

		auto pad = new QWidget;
		auto padLayout = new QGridLayout;
		padLayout->setContentsMargins(0, 0, 0, 0);
		pad->setLayout(padLayout);

		padLayout->addWidget(new QLabel(propertyName + ":", &d), 0, 0, 1, 3);
		padLayout->addWidget(edit, 1, 0, 1, 3);

		splitter->addWidget(pad);
		return edit;
	};

	auto scriptClickedEdit = createScriptControls(VFrame30::PropertyNames::clicked, VFrame30::PropertyNames::pushButtonDefaultEventScript);
	auto preDrawEdit = createScriptControls(VFrame30::PropertyNames::preDrawScript, VFrame30::PropertyNames::preDrawScriptDefault);

	// --
	//
	QDialogButtonBox* buttonBox = new QDialogButtonBox{QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &d};

	// Set main layout
	//
	layout->addWidget(splitter, 2, 0, 1, 3);
	layout->addWidget(buttonBox, 7, 0, 1, 3);

	d.setLayout(layout);

	// Restore splitter state.
	//
	QByteArray splitterState = QSettings().value("f2KeyForPushButton\\splitterState").toByteArray();
	if (splitterState.isEmpty() == false)
	{
		splitter->restoreState(splitterState);
	}

	connect(buttonBox, &QDialogButtonBox::accepted, &d, &QDialog::accept);
	connect(buttonBox, &QDialogButtonBox::rejected, &d, &QDialog::reject);

	// --
	//
	int result = d.exec();

	if (result == QDialog::Accepted)
	{
		QString newObjectName = objectNameEdit->text();
		QString newText = textEdit->text();
		QString newScriptClicked = scriptClickedEdit->toPlainText();
		QString newPreDrawScript = preDrawEdit->toPlainText();

		if (newObjectName != objectName || newText != text || newScriptClicked != scriptClicked || newPreDrawScript != preDrawScript)
		{
			if (bool ok = m_editEngine->startBatch(); ok == true)
			{
				m_editEngine->runSetProperty(VFrame30::PropertyNames::objectName, QVariant(newObjectName), item);
				m_editEngine->runSetProperty(VFrame30::PropertyNames::clicked, QVariant(newScriptClicked), item);
				m_editEngine->runSetProperty(VFrame30::PropertyNames::text, QVariant(newText), item);
				m_editEngine->runSetProperty(VFrame30::PropertyNames::preDrawScript, QVariant(newPreDrawScript), item);

				m_editEngine->endBatch();
			}
		}
	}

	// Save splitter state for next time.
	//
	QSettings().setValue("f2KeyForPushButton\\splitterState", splitter->saveState());

	return;
}

void F2KeyForSchemaItem::f2KeyForLineEdit(SchemaItemPtr item)
{
	auto editItem = dynamic_cast<VFrame30::SchemaItemLineEdit*>(item.get());
	if (editItem == nullptr)
	{
		assert(editItem);
		return;
	}

	QString objectName = item->objectName();
	QString text = editItem->text();

	QString scriptAfterCreate = editItem->scriptAfterCreate();
	QString scriptPreDrawCreate = editItem->preDrawScript();
	QString scriptReturnPressed = editItem->scriptReturnPressed();

	// Show input dialog
	//
	ResizedDialog d{tr("SchemaItemLineEdit"), m_parent};
	d.setWindowFlags((d.windowFlags() & ~Qt::WindowMinimizeButtonHint & ~Qt::WindowContextHelpButtonHint) | Qt::CustomizeWindowHint |
					 Qt::WindowMaximizeButtonHint);

	QGridLayout* layout = new QGridLayout{&d};

	// ObjectName
	//
	QLabel* objectNameEditLabel = new QLabel{"ObjectName:", &d};
	objectNameEditLabel->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);

	auto objectNameEdit = new QLineEdit{&d};
	objectNameEdit->setText(objectName);

	layout->addWidget(objectNameEditLabel, 0, 0);
	layout->addWidget(objectNameEdit, 0, 1, 1, 2);

	// Text
	//
	QLabel* textLabel = new QLabel{"Text:", &d};
	textLabel->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);

	auto textEdit = new QLineEdit{&d};
	textEdit->setText(text);

	layout->addWidget(textLabel, 1, 0);
	layout->addWidget(textEdit, 1, 1, 1, 2);

	// Scripts
	//
	QSplitter* splitter = new QSplitter(Qt::Vertical, &d);
	splitter->setChildrenCollapsible(false);

	auto createScriptControls = [&d, splitter, editItem](QString propertyName, QString defaultScript) -> UiLib::CodeEditor*
	{
		auto property = editItem->propertyByCaption(propertyName);
		if (property == nullptr)
		{
			assert(property);
			return nullptr;
		}

		auto edit = new UiLib::CodeEditor(&d);
		UiLib::JsHighlighter::createJsHighlighter(edit);

#if defined(Q_OS_WIN)
		edit->setFont(QFont("Consolas"));
#else
		edit->setFont(QFont("Courier"));
#endif
		edit->setText(property->value().toString());
		edit->setToolTip(property->description() + "\n" + defaultScript);

		auto pad = new QWidget;
		auto padLayout = new QGridLayout;
		padLayout->setContentsMargins(0, 0, 0, 0);
		pad->setLayout(padLayout);

		padLayout->addWidget(new QLabel(propertyName + ":", &d), 0, 0, 1, 3);
		padLayout->addWidget(edit, 1, 0, 1, 3);

		splitter->addWidget(pad);
		return edit;
	};

	auto afterCreateEdit = createScriptControls(VFrame30::PropertyNames::afterCreate, VFrame30::PropertyNames::lineEditDefaultEventScript);
	auto preDrawEdit = createScriptControls(VFrame30::PropertyNames::preDrawScript, VFrame30::PropertyNames::preDrawScriptDefault);
	auto returnPressedEdit =
		createScriptControls(VFrame30::PropertyNames::returnPressed, VFrame30::PropertyNames::lineEditDefaultEventScript);

	// --
	//
	QDialogButtonBox* buttonBox = new QDialogButtonBox{QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &d};

	// Set main layout
	//
	layout->addWidget(splitter, 2, 0, 1, 3);
	layout->addWidget(buttonBox, 7, 0, 1, 3);

	d.setLayout(layout);

	// Restore splitter state.
	//
	QByteArray splitterState = QSettings().value("f2KeyForLineEdit\\splitterState").toByteArray();
	if (splitterState.isEmpty() == false)
	{
		splitter->restoreState(splitterState);
	}

	connect(buttonBox, &QDialogButtonBox::accepted, &d, &QDialog::accept);
	connect(buttonBox, &QDialogButtonBox::rejected, &d, &QDialog::reject);

	// --
	//
	int result = d.exec();

	if (result == QDialog::Accepted)
	{
		QString newObjectName = objectNameEdit->text();
		QString newText = textEdit->text();
		QString newAfterCreate = afterCreateEdit->toPlainText();
		QString newPreDrawScript = preDrawEdit->toPlainText();
		QString newReturnPressed = returnPressedEdit->toPlainText();

		if (newObjectName != objectName || newText != text || newAfterCreate != scriptAfterCreate ||
			newPreDrawScript != scriptPreDrawCreate || newReturnPressed != scriptReturnPressed)
		{
			if (bool ok = m_editEngine->startBatch(); ok == true)
			{
				m_editEngine->runSetProperty(VFrame30::PropertyNames::objectName, QVariant(newObjectName), item);
				m_editEngine->runSetProperty(VFrame30::PropertyNames::text, QVariant(newText), item);
				m_editEngine->runSetProperty(VFrame30::PropertyNames::afterCreate, QVariant(newAfterCreate), item);
				m_editEngine->runSetProperty(VFrame30::PropertyNames::preDrawScript, QVariant(newPreDrawScript), item);
				m_editEngine->runSetProperty(VFrame30::PropertyNames::returnPressed, QVariant(newReturnPressed), item);

				m_editEngine->endBatch();
			}
		}
	}

	// Save splitter state for next time.
	//
	QSettings().setValue("f2KeyForLineEdit\\splitterState", splitter->saveState());

	return;
}
