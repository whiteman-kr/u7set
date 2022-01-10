"use strict";

module Log {
	export interface LogController {
		writeAlert(text: string): boolean;
		writeError(text: string): boolean;
		writeWarning(text: string): boolean;
		writeMessage(text: string): boolean;
		writeText(text: string): boolean;
	}
}