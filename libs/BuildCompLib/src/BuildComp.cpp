#include <BuildCompLib/BuildComp.h>

#include "BuildCompImpl.h"


namespace BuildCompLib
{
	BuildComp::BuildComp() :
		m_impl{std::make_unique<BuildCompImpl>()}
	{
	}

	BuildComp::~BuildComp() = default;

	tl::expected<bool, QString> BuildComp::setFileLeft(QString fileName)
	{
		return m_impl->setFileLeft(fileName);
	}

	tl::expected<bool, QString> BuildComp::setFileRight(QString fileName)
	{
		return m_impl->setFileRight(fileName);
	}

	BuildCompLib::CompareResult BuildComp::compare() const
	{
		return m_impl->compare();
	}
} // namespace BuildCompLib