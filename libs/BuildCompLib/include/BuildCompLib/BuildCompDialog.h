#pragma once

#include <QDialog>

#include <memory>

namespace BuildCompLib
{
	struct BuildCompDialogPrivate;


	class BuildCompDialog : public QDialog
	{
		Q_OBJECT

	public:
		explicit BuildCompDialog(QWidget* parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
		~BuildCompDialog();

	public:
		void setDefaultFolder(const QString& directory);

		void setLeftFolder(const QString& directory, bool compare);
		void setRightFolder(const QString& directory, bool compare);

		void setFileLeft(const QString& fileName, bool compare);
		void setFileRight(const QString& fileName, bool compare);

	protected:
		void compareFiles();

	protected slots:
		void browseFile1();
		void browseFile2();
		void swapFiles();

	private:
		std::unique_ptr<BuildCompDialogPrivate> m_d;
	};

} // namespace BuildCompLib