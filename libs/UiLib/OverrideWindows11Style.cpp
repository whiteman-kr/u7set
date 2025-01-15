namespace
{
	bool wasStyleExplicitlyRequested(int argc, char* argv[])
	{
		// Check command line
		//
		for (int i = 1; i < argc; ++i)
		{
			QString arg = QString::fromLatin1(argv[i]).toLower();
			if (arg.startsWith("-style") == true || arg.startsWith("--style") == true)
			{
				return true;
			}
		}

		// Check environment variable
		// (QT_STYLE_OVERRIDE is commonly used to force a particular style)
		//
		if (qEnvironmentVariableIsEmpty("QT_STYLE_OVERRIDE") == false)
		{
			return true;
		}

		return false;
	}
} // namespace

namespace UiLib
{
	// Override Windows11 style, the implementation just ugly.
	//
	void OverrideWindows11Style(QApplication& app, int argc, char* argv[])
	{
		// 1) After QApplication is constructed, Qt has already
		//
		QStyle* currentStyle = app.style();
		QString currentStyleName = currentStyle ? QString::fromLatin1(currentStyle->metaObject()->className()) : QString();

		// 2) Determine if the user explicitly forced a style.
		//
		bool styleForced = wasStyleExplicitlyRequested(argc, argv);

		// 3) Check if the current style is Windows 11
		//    (you may need to adjust the string to match your Qt build).
		//    Possible names: "QWindows11Style", "QWindows11LightStyle"
		//
		bool isWindows11 = currentStyleName.compare("QWindows11Style", Qt::CaseInsensitive) == 0 ||
						   currentStyleName.compare("QWindows11LightStyle", Qt::CaseInsensitive) == 0 ||
						   currentStyleName.compare("windows11", Qt::CaseInsensitive) == 0;

		// 4) If we are on Windows 11 style OR a style was explicitly set,
		//    then override it to "windowsvista".
		if (isWindows11 == true && styleForced == false)
		{
			qDebug() << "Overriding style" << currentStyleName << "with windowsvista.";
			app.setStyle("windowsvista");
		}

		return;
	}
} // namespace UiLib