#include <BuildCompLib/BuildComp.h>
#include <BuildCompLib/BuildCompDialog.h>

#include <QApplication>
#include <QDialogButtonBox>
#include <QFile>
#include <QFileDialog>
#include <QFontDatabase>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPainter>
#include <QPixmap>
#include <QPushButton>
#include <QScrollBar>
#include <QSvgRenderer>
#include <QTextEdit>


namespace
{
	const QString ButtonSwapSvg =
		R"_(<?xml version="1.0" encoding="utf-8"?><!-- Uploaded to: SVG Repo, www.svgrepo.com, Generator: SVG Repo Mixer Tools -->
<svg width="800px" height="800px" viewBox="0 0 512 512" xmlns="http://www.w3.org/2000/svg">
  <polygon fill="var(--ci-primary-color, #000000)" points="364.118 67.313 433.373 136.568 160 136.568 160 168.568 433.373 168.568 364.118 237.823 386.745 260.45 494.628 152.568 386.745 44.687 364.118 67.313" class="ci-primary"/>
  <polygon fill="var(--ci-primary-color, #000000)" points="147.882 267.882 125.255 245.255 17.373 353.137 125.255 461.02 147.882 438.393 78.627 369.137 352 369.137 352 337.137 78.627 337.137 147.882 267.882" class="ci-primary"/>
</svg>)_";

	QPixmap svgToPixmap(const QString& svgData, int width, int height)
	{
		QSvgRenderer renderer{svgData.toUtf8()};

		QPixmap pixmap{width, height};
		pixmap.fill(Qt::transparent);

		QPainter painter{&pixmap};
		renderer.render(&painter);

		return pixmap;
	}

	QString centerJustified(int columnWidth, const QString& text)
	{
		int textLength = text.length();

		if (textLength >= columnWidth)
		{
			return text;                               // No padding if text is already longer
		}

		int totalPadding = columnWidth - textLength;
		int leftPadding = totalPadding / 2;
		int rightPadding = totalPadding - leftPadding; // Ensure exact width

		// Convert spaces to &nbsp; for QTextEdit
		//
		QString leftPad = QString(leftPadding, ' ').replace(" ", "&nbsp;");
		QString rightPad = QString(rightPadding, ' ').replace(" ", "&nbsp;");

		return leftPad + text + rightPad;
	}


} // namespace

namespace BuildCompLib
{
	struct BuildCompDialogPrivate
	{
		QLineEdit* editFile1 = nullptr;
		QLineEdit* editFile2 = nullptr;
		QTextEdit* compareResult = nullptr;

		QString defaultFolder;
	};

	BuildCompDialog::BuildCompDialog(QWidget* parent, Qt::WindowFlags f) :
		QDialog{parent, f},
		m_d{std::make_unique<BuildCompDialogPrivate>()}
	{
		setWindowFlags(windowFlags() | Qt::WindowMaximizeButtonHint);

		setWindowTitle(tr("Build Compare"));

		auto layout = new QGridLayout(this);

		m_d->editFile1 = new QLineEdit();
		m_d->editFile1->setReadOnly(true);

		QPushButton* browseFile1 = new QPushButton{tr("Browse...")};
		connect(browseFile1, &QPushButton::clicked, this, &BuildCompDialog::browseFile1);

		QPushButton* swapButton = new QPushButton{};
		swapButton->setIcon(svgToPixmap(ButtonSwapSvg, 128, 128));
		connect(swapButton, &QPushButton::clicked, this, &BuildCompDialog::swapFiles);

		m_d->editFile2 = new QLineEdit{};
		m_d->editFile2->setReadOnly(true);

		QPushButton* browseFile2 = new QPushButton{tr("Browse...")};
		connect(browseFile2, &QPushButton::clicked, this, &BuildCompDialog::browseFile2);

		// Line 0 - | file name 1 | browse 1 | swap | file name 2 | browse 2 |
		//
		layout->addWidget(m_d->editFile1, 0, 0);
		layout->addWidget(browseFile1, 0, 1);
		layout->addWidget(swapButton, 0, 2);
		layout->addWidget(m_d->editFile2, 0, 3);
		layout->addWidget(browseFile2, 0, 4);

		// Line 1, 2 - compare result text document
		//
		m_d->compareResult = new QTextEdit{};
		m_d->compareResult->setReadOnly(true);
		QFont font = QFontDatabase::systemFont(QFontDatabase::FixedFont);
		m_d->compareResult->setFont(font);
		m_d->compareResult->zoomIn(1);

		layout->addWidget(new QLabel{tr("Compare result:")}, 1, 0, 1, 5);
		layout->addWidget(m_d->compareResult, 2, 0, 1, 5);

		// Line 3 - | close button |
		//
		QDialogButtonBox* buttonBox = new QDialogButtonBox{QDialogButtonBox::Close};
		connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

		layout->addWidget(buttonBox, 3, 0, 1, 5);

		setLayout(layout);

		// Make dialog bigger at start
		//
		resize(sizeHint() * 1.5);

		return;
	}

	BuildCompDialog::~BuildCompDialog()
	{
		qDebug() << "BuildCompDialog::~BuildCompDialog()";
	}

	void BuildCompDialog::setDefaultFolder(const QString& directory)
	{
		m_d->defaultFolder = directory;
	}

	void BuildCompDialog::setLeftFolder(const QString& directory, bool compare)
	{
		// Find a single *.bts file in the folder
		//
		QDir dir{directory};
		QStringList files = dir.entryList(QStringList("*.bts"), QDir::Files | QDir::NoSymLinks);

		if (files.size() != 1)
		{
			return;
		}

		return setFileLeft(directory + QDir::separator() + files[0], compare);
	}

	void BuildCompDialog::setRightFolder(const QString& directory, bool compare)
	{
		// Find a single *.bts file in the folder
		//
		QDir dir{directory};
		QStringList files = dir.entryList(QStringList("*.bts"), QDir::Files | QDir::NoSymLinks);

		if (files.size() != 1)
		{
			return;
		}

		return setFileRight(files[0], compare);
	}

	void BuildCompDialog::setFileLeft(const QString& fileName, bool compare)
	{
		QString nativeFileName = QDir::toNativeSeparators(fileName);
		m_d->editFile1->setText(nativeFileName);

		if (compare == true)
		{
			compareFiles();
		}

		return;
	}

	void BuildCompDialog::setFileRight(const QString& fileName, bool compare)
	{
		QString nativeFileName = QDir::toNativeSeparators(fileName);
		m_d->editFile2->setText(nativeFileName);

		if (compare == true)
		{
			compareFiles();
		}

		return;
	}

	void BuildCompDialog::compareFiles()
	{
		// Set wait cursor
		//
		QApplication::setOverrideCursor(Qt::WaitCursor);

		m_d->compareResult->clear();

		BuildComp comparer;
		auto f1 = comparer.setLeftFile(m_d->editFile1->text());
		auto f2 = comparer.setRightFile(m_d->editFile2->text());

		if (!f1)
		{
			QString text = "<font color = 'red'>Left File</font><br>"
						   "<font color = 'red'>" +
						   f1.error() + "</font >";

			m_d->compareResult->setText(text);
		}

		if (!f2)
		{
			QString text = "<font color = 'red'>Right File</font><br>"
						   "<font color = 'red'>" +
						   f2.error() + "</font >";

			m_d->compareResult->setText(text);
		}

		if (!f1 || !f2)
		{
			// Restore cursor
			//
			QApplication::restoreOverrideCursor();
			return;
		}

		auto result = comparer.compare();

		// Print compare result to QStringList
		//
		QStringList text;

		QString redStart = "<font color = 'red'>";
		QString greenStart = "<font color = 'green'>";
		QString orangeStart = "<font color = 'orange'>";
		QString textEnd = "</font>";
		QString endline = "<br>";


		if (result.projectName == false)
		{
			text << redStart;
			text << "Project names are different: " << result.projectNameLeft << " vs " << result.projectNameRight << endline;
			text << textEnd;
		}
		else
		{
			text << "Project names are the same: " << result.projectNameLeft << endline;
		}

		if (result.userName == false)
		{
			text << "User names are different: " << result.userNameLeft << " vs " << result.userNameRight << endline;
		}
		else
		{
			text << "User names are the same: " << result.userNameLeft << endline;
		}

		if (result.buildNumber == false)
		{
			text << "Build numbers are different: " << QString::number(result.buildNumberLeft) << " vs "
				 << QString::number(result.buildNumberRight) << endline;
		}
		else
		{
			text << orangeStart;
			text << "<b>Build numbers are the same: " << QString::number(result.buildNumberLeft) << "</b>" << endline;
			text << textEnd;
		}

		auto subsystemSideResultToStr = [](BuildCompLib::CompareResult::Subsystem::SideResult v)
		{
			switch (v)
			{
			case BuildCompLib::CompareResult::Subsystem::SideResult::NotModified:
				return "Not modified";
			case BuildCompLib::CompareResult::Subsystem::SideResult::Modified:
				return "Modified";
			case BuildCompLib::CompareResult::Subsystem::SideResult::NotExists:
				return "Not exists";
			default:
				return "Unknown";
			}
		};

		for (const auto& subsystem : result.subsystems)
		{
			if (subsystem.left != BuildCompLib::CompareResult::Subsystem::NotModified ||
				subsystem.right != BuildCompLib::CompareResult::Subsystem::NotModified)
			{
				text << redStart;
			}

			text << "Subsystem: " << subsystem.subsystemId << endline;

			// Print columns in one line, each column is separated by spaces, has at least 32 symbols width, and centered.
			// column separator is "|"
			//
			QString column1 = subsystemSideResultToStr(subsystem.left);
			QString column2 = subsystemSideResultToStr(subsystem.right);

			text << centerJustified(20, column1) << "  |  " << centerJustified(20, column2) << endline;

			if (subsystem.left != CompareResult::Subsystem::NotModified || subsystem.right != CompareResult::Subsystem::NotModified)
			{
				text << textEnd;
			}
		}

		text << endline;
		text << "<b>Summary</b>" << endline;

		if (result.isSame == true)
		{
			text << greenStart;
			text << "<b>FC: The payloads of the compared files are identical.</b>" << endline;
			text << textEnd;
		}
		else
		{
			text << redStart;

			text << "<b>The following subsystem(s) differ:</b>" << endline;

			for (const auto& subsystem : result.subsystems)
			{
				if (subsystem.left != BuildCompLib::CompareResult::Subsystem::NotModified ||
					subsystem.right != BuildCompLib::CompareResult::Subsystem::NotModified)
				{
					text << "<b>&nbsp;&nbsp;&nbsp;&nbsp;" << subsystem.subsystemId << "</b>" << endline;

					auto modules = subsystem.leftModules + subsystem.rightModules;
					modules.sort();
					modules.removeDuplicates();

					for (const auto& rightModule : modules)
					{
						text << "<b>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;" << rightModule << "</b>" << endline;
					}
				}
			}

			text << "<b>FC: The files are different.</b>" << endline;
			text << textEnd;
		}

		// Print compare result to QTextEdit
		//
		m_d->compareResult->setText(text.join(""));

		// Scroll down.
		//
		auto scrollValue = m_d->compareResult->verticalScrollBar()->maximum();
		m_d->compareResult->verticalScrollBar()->setValue(scrollValue);

		// Restore cursor
		//
		QApplication::restoreOverrideCursor();
		return;
	}

	void BuildCompDialog::browseFile1()
	{
		QFileDialog fd{this};

		fd.setAcceptMode(QFileDialog::AcceptOpen);
		fd.setFileMode(QFileDialog::ExistingFile);

		if (m_d->defaultFolder.isEmpty() == false)
		{
			fd.setDirectory(m_d->defaultFolder);
			m_d->defaultFolder.clear();
		}

		QStringList filters;
		filters << "Bitstream files (*.bts)"
				<< "All files (*.*)";

		fd.setNameFilters(filters);

		if (fd.exec() == QDialog::Rejected)
		{
			return;
		}

		QStringList fileList = fd.selectedFiles();
		if (fileList.size() != 1)
		{
			return;
		}

		setFileLeft(fileList[0], true);
		return;
	}

	void BuildCompDialog::browseFile2()
	{
		QFileDialog fd{this};

		fd.setAcceptMode(QFileDialog::AcceptOpen);
		fd.setFileMode(QFileDialog::ExistingFile);

		if (m_d->defaultFolder.isEmpty() == false)
		{
			fd.setDirectory(m_d->defaultFolder);
			m_d->defaultFolder.clear();
		}

		QStringList filters;
		filters << "Bitstream files (*.bts)"
				<< "All files (*.*)";

		fd.setNameFilters(filters);

		if (fd.exec() == QDialog::Rejected)
		{
			return;
		}

		QStringList fileList = fd.selectedFiles();
		if (fileList.size() != 1)
		{
			return;
		}

		setFileRight(fileList[0], true);
		return;
	}

	void BuildCompDialog::swapFiles()
	{
		auto fileNameLeft = m_d->editFile1->text();
		auto fileNameRight = m_d->editFile2->text();

		m_d->editFile1->setText(fileNameRight);
		m_d->editFile2->setText(fileNameLeft);

		compareFiles();
		return;
	}

} // namespace BuildCompLib