#include <BuildCompLib/BuildComp.h>

#include "BuildCompImpl.h"

#include <QDir>

namespace BuildCompLib
{
	BuildComp::BuildComp() :
		m_impl{std::make_unique<BuildCompImpl>()}
	{
	}

	BuildComp::~BuildComp() = default;

	tl::expected<bool, QString> BuildComp::setLeftFolder(QString directory)
	{
		// Look for a single *.bts file in the folder
		//
		QDir dir{directory};
		QStringList files = dir.entryList(QStringList("*.bts"), QDir::Files | QDir::NoSymLinks);

		if (files.size() != 1)
		{
			return tl::make_unexpected(QString("Error: %1").arg("No or more than one *.bts file found in the folder"));
		}

		return setLeftFile(directory + QDir::separator() + files[0]);
	}

	tl::expected<bool, QString> BuildComp::setRightFolder(QString directory)
	{
		// Look for a single *.bts file in the folder
		//
		QDir dir{directory};
		QStringList files = dir.entryList(QStringList("*.bts"), QDir::Files | QDir::NoSymLinks);

		if (files.size() != 1)
		{
			return tl::make_unexpected(QString("Error: %1").arg("No or more than one *.bts file found in the folder"));
		}

		return setRightFile(directory + QDir::separator() + files[0]);
	}

	tl::expected<bool, QString> BuildComp::setLeftFile(QString fileName)
	{
		return m_impl->setLeftFile(fileName);
	}

	tl::expected<bool, QString> BuildComp::setRightFile(QString fileName)
	{
		return m_impl->setRightFile(fileName);
	}

	BuildCompLib::CompareResult BuildComp::compare() const
	{
		return m_impl->compare();
	}
} // namespace BuildCompLib