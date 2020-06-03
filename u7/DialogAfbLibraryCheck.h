#ifndef DIALOGAFBLIBRARYREPORT_H
#define DIALOGAFBLIBRARYREPORT_H

#include <QDialog>
#include "../lib/DbController.h"
#include "../VFrame30/Afb.h"

enum class AfbComponentColumns
{
	Caption,
	OpCode_OpIndex,
	HasRam,
	ImpVersion,
	VersionOpIndex,
	PinVersionOpIndex,
	MaxInstCount,
	SimulationFunc,
	Count
};


enum class AfbElementColumns
{
	Caption = 0,
	StrID,
	OpCode_OpIndex,
	Version_Type,
	HasRam_DataFormat,
	InternalUse_SignalType,
	Size,
	ByteOrder,
	Count
};

class AfbElementTreeWidgetItem : public QTreeWidgetItem
{
public:
	AfbElementTreeWidgetItem(QTreeWidget* parent):QTreeWidgetItem(parent){}
	AfbElementTreeWidgetItem(QTreeWidgetItem* parent):QTreeWidgetItem(parent){}
private:
	bool operator<(const QTreeWidgetItem &other)const
	{
		int column = (treeWidget()->sortColumn());

		if (column == static_cast<int>(AfbElementColumns::OpCode_OpIndex))
		{
			return text(column).toInt() < other.text(column).toInt();
		}
		else
		{
			return text(column) < other.text(column);
		}
	}
};

class AfbComponentTreeWidgetItem : public QTreeWidgetItem
{
public:
	AfbComponentTreeWidgetItem(QTreeWidget* parent):QTreeWidgetItem(parent){}
	AfbComponentTreeWidgetItem(QTreeWidgetItem* parent):QTreeWidgetItem(parent){}
private:
	bool operator<(const QTreeWidgetItem &other)const
	{
		int column = (treeWidget()->sortColumn());

		if (column == static_cast<int>(AfbComponentColumns::OpCode_OpIndex) ||
				column == static_cast<int>(AfbComponentColumns::ImpVersion) ||
				column == static_cast<int>(AfbComponentColumns::VersionOpIndex) ||
				column == static_cast<int>(AfbComponentColumns::MaxInstCount))
		{
			return text(column).toInt() < other.text(column).toInt();
		}
		else
		{
			return text(column) < other.text(column);
		}
	}
};

class DialogAfbCheckResult : public QDialog
{
public:
	DialogAfbCheckResult(QString message, QWidget* parent);

};

class DialogAfbLibraryCheck : public QDialog
{
	Q_OBJECT
public:
	DialogAfbLibraryCheck(DbController* db, QWidget* parent);
	virtual ~DialogAfbLibraryCheck();

private slots:
	void libraryFileChanged(const QString& fileName);
	//void onCheckPins();

private:
	void fillAfbSignalItem(AfbElementTreeWidgetItem* pinItem,
						   const QString& caption,
						   const QString& opName,
						   int operandIndex,
						   const QString& type,
						   E::DataFormat dataFormat,
						   E::SignalType signalType,
						   int size,
						   E::ByteOrder byteOrder);

private:
	DbController* m_db = nullptr;

	QTreeWidget* m_afbElementTreeWidget = nullptr;
	QTreeWidget* m_afbComponentTreeWidget = nullptr;

	std::map<int, std::shared_ptr<Afb::AfbComponent>> m_afbComponents;
	std::vector<std::shared_ptr<Afb::AfbElement>> m_afbElements;

	QSplitter* m_splitter = nullptr;
};

extern DialogAfbLibraryCheck* theDialogAfbLibraryCheck;

#endif // DIALOGAFBLIBRARYREPORT_H
