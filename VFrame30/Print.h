#pragma once

#include "Scheme.h"

namespace VFrame30
{
	class VFRAME30LIBSHARED_EXPORT CPrint
	{
		std::list<std::pair<std::shared_ptr<Scheme>, bool>> FrameList;

	public:
		CPrint(void);
		~CPrint(void);

		void Add(std::pair<std::shared_ptr<VFrame30::Scheme>, bool> VFrameItem);

		//void Print(HWND hWnd) {  Print(hWnd, ""); }
		//void Print(HWND hWnd, const std::string FileName);
	};


}
