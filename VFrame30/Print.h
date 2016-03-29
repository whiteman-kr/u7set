#pragma once

#include "Schema.h"

namespace VFrame30
{
	class VFRAME30LIBSHARED_EXPORT CPrint
	{
		std::list<std::pair<std::shared_ptr<Schema>, bool>> FrameList;

	public:
		CPrint(void);
		~CPrint(void);

		void Add(std::pair<std::shared_ptr<VFrame30::Schema>, bool> VFrameItem);

		//void Print(HWND hWnd) {  Print(hWnd, ""); }
		//void Print(HWND hWnd, const std::string FileName);
	};


}
