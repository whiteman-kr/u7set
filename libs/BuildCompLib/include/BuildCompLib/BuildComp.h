#pragma once

#include <CommonLib/expected.hpp>

#include "CompareResult.h"

#include <memory>


namespace BuildCompLib
{
	class BuildCompImpl;


	class BuildComp final
	{
	public:
		BuildComp();
		BuildComp(const BuildComp&) = delete;
		BuildComp& operator=(const BuildComp&) = delete;
		~BuildComp();

	public:
		tl::expected<bool, QString> setLeftFolder(QString directory);
		tl::expected<bool, QString> setRightFolder(QString directory);

		tl::expected<bool, QString> setLeftFile(QString fileName);
		tl::expected<bool, QString> setRightFile(QString fileName);

		[[nodiscard]] BuildCompLib::CompareResult compare() const;

	private:
		std::unique_ptr<BuildCompImpl> m_impl;
	};
} // namespace BuildCompLib