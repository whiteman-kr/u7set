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
		tl::expected<bool, QString> setFileLeft(QString fileName);
		tl::expected<bool, QString> setFileRight(QString fileName);

		[[nodiscard]] BuildCompLib::CompareResult compare() const;

	private:
		std::unique_ptr<BuildCompImpl> m_impl;
	};
} // namespace BuildCompLib