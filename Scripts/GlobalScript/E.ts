"use strict";

module E {
	export const enum ArchiveTimeType {
		TIME_PLANT = 0,
		TIME_SYSTEM = 1,
		TIME_LOCAL = 2
	}

	export enum Channel {
		A = 0, /**< Channel A = 0*/
		B = 1, /**< Channel B = 1*/
		C = 2, /**< Channel C = 2 */
		D = 3  /**< Channel D = 3 */
	}

	export enum AnalogFormat {
		e_9e = 'e',			/**< e_9e = 'e' (0x65/101) Format as [-]9.9e[+|-]999*/
		E_9E = 'E',			/**< E_9E = 'E' (0x45/69) Format as [-]9.9E[+|-]999*/
		f_9 = 'f',			/**< f_9 = 'f' (0x66/102) Format as [-]9.9*/
		g_9_or_9e = 'g',	/**< g_9_or_9e = 'g' (0x67/103) Use 'e' or 'f' format, whichever is the most concise*/
		G_9_or_9E = 'G'		/**< G_9_or_9E = 'G' (0x47/71) Use E or f format, whichever is the most concise*/
	}

	export enum ColumnData {
		AppSignalID = 0,			/**< AppSignalID = 0*/
		CustomSignalID = 1,			/**< CustomSignalID = 1*/
		Caption = 2,				/**< Caption = 2*/
		State = 3,					/**< State = 3*/
		ImpactAppSignalID = 32,		/**< ImpactAppSignalID = 32*/
		ImpactCustomSignalID = 33,	/**< ImpactCustomSignalID = 33*/
		ImpactCaption = 34,			/**< ImpactCaption = 34*/
		ImpactState = 35,			/**< ImpactState = 35*/
		CustomText = 64				/**< CustomText = 64*/
	}

	export enum LineStyle {
		NoPen = 0,				/**< NoPen = 0. No line at all, for example rect fills but does not draw any boundary line.*/
		SolidLine = 1,			/**< SolidLine = 1. A plain line.*/
		DashLine = 2,			/**< DashLine = 2. Dashes separated by a few pixels.*/
		DotLine = 3,			/**< DotLine = 3. Dots separated by a few pixels.*/
		DashDotLine = 4,		/**< DashDotLine = 4. Alternate dots and dashes.*/
		DashDotDotLine = 5		/**< DashDotDotLine = 5. One dash, two dots, one dash, two dots.*/
	}

	export enum TextFormat {
		PlainText = 0,			/**< PlainText = 0. Manual formatting.*/
		Markdown = 1,			/**< Markdown = 1. Markdown formatting, supports GitHub-style Markdown.*/
		HtmlSubset = 2			/**< HtmlSubset = 2. HTML-formatted text in the html string. Support of the limited HTML Subset.*/
	}

	export enum HorzAlign {
		AlignLeft = 0x01,		/**< AlignLeft = 0x01*/
		AlignRight = 0x02,		/**< AlignRight = 0x02*/
		AlignHCenter = 0x04,	/**< AlignHCenter = 0x04*/
		AlignJustify = 0x08		/**< AlignJustify = 0x08*/
	}

	export enum VertAlign {
		AlignTop = 0x20,		/**< AlignTop = 0x20*/
		AlignBottom = 0x40,		/**< AlignBottom = 0x40*/
		AlignVCenter = 0x80,	/**< AlignVCenter = 0x80*/
		AlignBaseline = 0x100	/**< AlignBaseline = 0x100*/
	}

	export enum Alignment {
		AlignLeft = 0x01,		/**< AlignLeft = 0x01*/
		AlignRight = 0x02,		/**< AlignRight = 0x02*/
		AlignHCenter = 0x04,	/**< AlignHCenter = 0x04*/
		AlignJustify = 0x08,	/**< AlignJustify = 0x08*/
		AlignAbsolute = 0x10,	/**< AlignAbsolute = 0x10*/

		AlignTop = 0x20,		/**< AlignTop = 0x20*/
		AlignBottom = 0x40,		/**< AlignBottom = 0x40*/
		AlignVCenter = 0x80,	/**< AlignVCenter = 0x80*/
		AlignBaseline = 0x100	/**< AlignBaseline = 0x100*/
	}

	export enum SoftwareType
	{
		Unknown = 8000,
		BaseService = 8999,
		Monitor = 9000,
		ConfigurationService = 9001,
		AppDataService = 9002,
		ArchiveService = 9003,
		TuningService = 9004,
		DiagDataService = 9005,
		TuningClient = 9006,
		Metrology = 9007,
		ServiceControlManager = 9008,
		TestClient = 9009,
		TestSuite = 9010,
		GatewayService = 9011,
		Diagnostics = 9012
	};
}