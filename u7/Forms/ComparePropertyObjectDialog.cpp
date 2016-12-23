#include "ComparePropertyObjectDialog.h"
#include "ui_ComparePropertyObjectDialog.h"

#ifdef Q_OS_WIN
#pragma warning(push)
#pragma warning(disable : 4267)
#endif
	#include "../../lib/diff_match_patch.h"
#ifdef Q_OS_WIN
#pragma warning(pop)
#endif


ComparePropertyObjectDialog::ComparePropertyObjectDialog(QString source, QString target, QWidget* parent) :
	QDialog(parent),
	ui(new Ui::ComparePropertyObjectDialog)
{
	ui->setupUi(this);

	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
	setWindowFlags(windowFlags() | Qt::WindowMaximizeButtonHint);

	ui->source->setReadOnly(true);

	std::wstring sourceStd = source.toStdWString();
	std::wstring targetStd = target.toStdWString();

	diff_match_patch<std::wstring> diff;
	diff_match_patch<std::wstring>::Diffs diffs = diff.diff_main(sourceStd, targetStd, true);

	bool equal = true;
	for (auto d : diffs)
	{
		if (d.operation != diff_match_patch<std::wstring>::Operation::EQUAL)
		{
			equal = false;
			break;
		}
	}

	if (equal == true)
	{
		ui->source->append(tr("Objects are equal"));
	}
	else
	{
		diff_match_patch<std::wstring>::diff_cleanupSemantic(diffs);
		QString result = QString::fromStdWString(diff_match_patch<std::wstring>::diff_prettyHtml(diffs));

		ui->source->zoomIn();
		ui->source->append(result.remove("&para;"));
	}

	return;
}

ComparePropertyObjectDialog::~ComparePropertyObjectDialog()
{
	delete ui;
}

QString ComparePropertyObjectDialog::objedctToCompareString(PropertyObject* object)
{
	if (object == nullptr)
	{
		assert(object);
		return QString();
	}

	std::vector<std::shared_ptr<Property>> props = object->properties();

	std::sort(props.begin(), props.end(),
			[](std::shared_ptr<Property> p1, std::shared_ptr<Property> p2)
			{
				return  QString("%1-%2").arg(p1->category()).arg(p1->caption()) <
						QString("%1-%2").arg(p2->category()).arg(p2->caption());
			});

	QString result;
	for (std::shared_ptr<Property> p : props)
	{
		if (p->visible() == false)
		{
			continue;
		}

		result += QString("%1\\%2: %3\n")
						.arg(p->category())
						.arg(p->caption())
						.arg(p->value().toString());
	}

	return result;
}

void ComparePropertyObjectDialog::showDialog(
		DbChangesetObject object,
		CompareData compareData,
		std::shared_ptr<PropertyObject> source,
		std::shared_ptr<PropertyObject> target,
		QWidget* parent)
{
	if (source == nullptr ||
		target == nullptr)
	{
		assert(source);
		assert(target);
		return;
	}

	QString sourceStr = ComparePropertyObjectDialog::objedctToCompareString(source.get());
	QString targetStr = ComparePropertyObjectDialog::objedctToCompareString(target.get());

	showDialog(object, compareData, sourceStr, targetStr, parent);
	return;
}

void ComparePropertyObjectDialog::showDialog(
		DbChangesetObject object,
		CompareData compareData,
		QString source,
		QString target,
		QWidget* parent)
{
	ComparePropertyObjectDialog* dialog = new ComparePropertyObjectDialog(source, target, parent);

	// Generate windows title
	//
	QString sourceTitle;

	switch (compareData.sourceVersionType)
	{
	case CompareVersionType::Changeset:
		sourceTitle = QString("Source: CS #%1").arg(compareData.sourceChangeset);
		break;
	case CompareVersionType::Date:
		sourceTitle = QString("Source: %1").arg(compareData.sourceDate.toString());
		break;
	case CompareVersionType::LatestVersion:
		sourceTitle = QString("Source: Latest");
		break;
	default:
		break;
	}

	QString targetTitle;

	switch (compareData.targetVersionType)
	{
	case CompareVersionType::Changeset:
		targetTitle = QString("Target: CS #%1").arg(compareData.targetChangeset);
		break;
	case CompareVersionType::Date:
		targetTitle = QString("Target: %1").arg(compareData.targetDate.toString());
		break;
	case CompareVersionType::LatestVersion:
		targetTitle = QString("Target: Latest");
		break;
	default:
		break;
	}

	dialog->setWindowTitle(tr("Compare %1, %2, %3").arg(object.name()).arg(sourceTitle).arg(targetTitle));

	dialog->setAttribute(Qt::WA_DeleteOnClose);
	dialog->show();
}

void ComparePropertyObjectDialog::showEvent(QShowEvent*)
{
	// Resize depends on monitor size, DPI, resolution
	//
	QRect screen = QDesktopWidget().availableGeometry(this);
	resize(screen.width() * 0.30, screen.height() * 0.45);

	move(screen.center() - rect().center());

	return;
}
