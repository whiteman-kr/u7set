#include "Metaparser.h"

const QString issueCode = "IssueCode";
const QString issueType = "IssueType";
const QString title = "Title";
const QString parameters = "Parameters";
const QString description = "Description";

Metaparser::Metaparser()
{

}

Metaparser::~Metaparser()
{

}

void Metaparser::setInputFile(const QString fileName)
{
	m_inFileName = fileName;
}

void Metaparser::setOutputFile(const QString fileName)
{
	m_outFileName = fileName;
}

int Metaparser::searchBlocks()
{
	// Try open selected file. If file opens with error -
	// terminate program and show error message
	//
	QFile inputFile(m_inFileName);

	if (inputFile.open(QIODevice::ReadOnly) == false)
	{
		qDebug() << "Error opening file: " << inputFile.errorString();
		return 2;
	}

	// Read every line from file
	//
	QTextStream dataFromInputFile(&inputFile);
	QString oneLineFromInputFile = dataFromInputFile.readLine();

	// Create dynamic array of type "parse". It will contain
	// foubded blocks
	//
	while (dataFromInputFile.atEnd() == false)
	{
		// Try to find beginning of the block
		//
		if (oneLineFromInputFile.indexOf("///") != -1 &&
				(oneLineFromInputFile.indexOf(issueCode) != -1 ||
				 oneLineFromInputFile.indexOf(issueType) != -1 ||
				 oneLineFromInputFile.indexOf(title) != -1 ||
				 oneLineFromInputFile.indexOf(parameters) != -1 ||
				 oneLineFromInputFile.indexOf(description) != -1) &&
				oneLineFromInputFile[oneLineFromInputFile.indexOf("///") + 3] != '/')
		{
			// Here we must check block beginning: first three symbols must be "/"
			//
			QString stringForCheckingBlockBeginning = oneLineFromInputFile;

			if (stringForCheckingBlockBeginning.replace(" ", "").indexOf("///") == 0)
			{
				// Create list of strings, where block will be placed
				//
				QStringList buff;

				buff << oneLineFromInputFile;

				QString stringForCheckingLineBeginning = oneLineFromInputFile;

				while (dataFromInputFile.atEnd() == false &&
					   oneLineFromInputFile.indexOf("///") != -1 &&
					   oneLineFromInputFile[oneLineFromInputFile.indexOf("///") + 3] != '/' &&
					   stringForCheckingLineBeginning.replace(" ", "").indexOf("///") == 0)
				{
					oneLineFromInputFile = dataFromInputFile.readLine();
					stringForCheckingLineBeginning = oneLineFromInputFile;
					buff << oneLineFromInputFile;
				}

				// When block was read, start processBlock function, which will
				// make structure from our list
				//
				BlockFromFile block = processBlock(buff);
				buff.clear();

				// At last, push this block to array
				//
				m_commentBlocks.push_back(block);
			}

		}

		oneLineFromInputFile = dataFromInputFile.readLine();
	}

	// Call writing function to generate html file
	//
	return 0;
}

Metaparser::BlockFromFile Metaparser::processBlock(const QStringList &block)
{
	// Create one value to be returned by function, one list of strings with headers from the block,
	// and one value, which will store information under the header
	//
	BlockFromFile Result;

	QStringList blockHeaders;
	blockHeaders << issueCode;
	blockHeaders << issueType;
	blockHeaders << title;
	blockHeaders << parameters;
	blockHeaders << description;

	QString bufferPart;

	// This value is specifying on current header
	//
	int currentPartParsing = 0;

	for (QString header : blockHeaders)
	{
		// Remove header from list, which going to be processed
		//
		blockHeaders.removeAt(0);

		// Create value, which will store line from block
		//
		QString bufferLine;

		// Cycle, which will read all rows from block
		//
		for (int currentString = 0; currentString < block.size(); currentString++)
		{
			bufferLine = block[currentString];

			// Check: is this row have the header, with we are working now?
			//
			if (bufferLine.indexOf(header) != -1)
			{
				// If it is true - write it to value, which stores information under header
				//
				bufferPart += bufferLine.remove(0, bufferLine.indexOf("///")+3);
				if (currentString + 1 < block.size())
					currentString ++;
				bufferLine = block[currentString];

				// Now, read and store all rows, which not includes other headers name,
				// and includes '///'
				while (currentString < block.size() &&
					   bufferLine.indexOf(issueCode) == -1 &&
					   bufferLine.indexOf(issueType) == -1 &&
					   bufferLine.indexOf(title) == -1 &&
					   bufferLine.indexOf(parameters) == -1 &&
					   bufferLine.indexOf(description) == -1 &&
					   bufferLine[bufferLine.indexOf("///") + 3] != '/' &&
					   bufferLine.indexOf("///") != -1)
				{
					bufferPart += bufferLine.remove(0, bufferLine.indexOf("///")+3);

					// Check: if we reached end of block - stop this cycle
					//

					if (currentString + 1 < block.size())
					{
						currentString++;
					}
					else
					{
						break;
					}

					bufferLine = block[currentString];
				}

				// Now, using value, that help us to detect our header, write info to the
				// returning value
				//
				switch (currentPartParsing)
				{
				case 0: Result.issueCode = bufferPart; break;
				case 1: Result.issueType = bufferPart; break;
				case 2: Result.title = bufferPart; break;
				case 3: Result.parameters = bufferPart; break;
				case 4: Result.description = bufferPart; break;
				}

				bufferLine.clear();
				bufferPart.clear();
			}
		}

		// Get to the next header
		//
		currentPartParsing++;
	}

	// Check gathered information on errors
	//
	if (Result.issueCode.isNull())
	{
		Result.error = true;
		Result.errorString = "Error: IssueCode was not found. Check the code.";
		Result.issueCode = "Error";
	}

	if (Result.issueType.isNull())
	{
		Result.error = true;
		Result.errorString = "Error: IssueType was not found. Check the code.";
		Result.issueType = "Type error";
	}

	if (Result.title.isNull())
	{
		Result.error = true;
		Result.errorString = "Error: Title was not found. Check the code.";
		Result.title = "Error: Title was not found. Check the code.";
	}

	if (Result.parameters.isNull())
	{
		Result.error = true;
		Result.errorString = "Error: Parameters was not found. Check the code.";
		Result.parameters = "Error: Parameters was not found. Check the code.";
	}

	if (Result.description.isNull())
	{
		Result.error = true;
		Result.errorString = "Error: Description was not found. Check the code.";
		Result.description = "Error: Description was not found. Check the code.";
	}

	return Result;
}

int Metaparser::writeToHtml()
{
	// If filename not contains .html ending - add it
	//
	int errorCounter = 0;

	QString fileName = m_outFileName;

	if (fileName.indexOf(".html") == -1)
	{
		fileName.append(".html");
	}

	// Try to open output file. In case of error - write error message
	// and stop the program
	//
	QFile outputFile(fileName);
	if (outputFile.open(QIODevice::WriteOnly) == false)
	{
		qDebug() << "Error creating file: " << outputFile.errorString();
		return 3;
	}

	QTextStream dataForOutputFile(&outputFile);

	// Generate basic html document
	//
	dataForOutputFile << "<DOCTYPE html>";
	dataForOutputFile << "\n\n<html>";
	dataForOutputFile << "\n<head>";
	dataForOutputFile << "\n\t<title>Metaparser_Log_" + fileName + "</title>";
	dataForOutputFile << "\n</head>";
	dataForOutputFile << "\n<body>";
	dataForOutputFile << "\n\t<h3>Issues</h3>";
	dataForOutputFile << "\n\t<table>";
	dataForOutputFile << "\n\t\t<tr>\n\t\t\t<td width=\"20%\">Issue Code</td>\n\t\t\t<td width=\"20%\">Issue Type</td>\n\t\t\t<td width=\"60%\">Title</td>\n\t\t</tr>";

	// This loop generates table with headers of the messages.
	//
	for (BlockFromFile block : m_commentBlocks)
	{
		// Write header of every block into table in html document
		//
		QString htmlAnchor = block.issueCode;

		// Creating html anchors. In case, that block has no or wrong IssueCode - we will match it is anchor
		// as error_#
		//
		if (htmlAnchor == "Error")
		{
			htmlAnchor = "error_" + QString::number(errorCounter);
			errorCounter++;
		}
		else
		{
			htmlAnchor.remove(block.issueCode.indexOf("IssueCode: "), 10).replace(" ", "");
		}

		dataForOutputFile << "\n\t\t<tr>\n\t\t\t<td><a href=\"#"
						  << htmlAnchor
						  << "\">";

		// If IssueCode is wrong - just type Error
		//
		if (block.issueCode == "Error")
		{
			dataForOutputFile << block.issueCode;
		}
		else
		{
			dataForOutputFile << block.issueCode.remove(block.issueCode.indexOf("IssueCode: "), 11).replace("\\n", "<br>");
		}

		dataForOutputFile << "</a></td>\n\t\t\t<td>";

		// If our block has some errors - match it is IssueType and Title with red color
		//
		if (block.error == true)
		{
			dataForOutputFile << "<font color=\"red\">";
		}

		// If IssueType is wrong - just print "Type error"
		//
		if (block.issueType == "Type error")
		{
			dataForOutputFile << block.issueType;
		}
		else
		{
			dataForOutputFile << block.issueType.remove(block.issueType.indexOf("IssueType: "), 11).replace("\\n", "<br>");
		}

		if (block.error == true)
		{
			dataForOutputFile << "</font>";
		}

		dataForOutputFile << "</td>\n\t\t\t<td>";

		if (block.error == true)
		{
			dataForOutputFile << "<font color=\"red\">";
		}

		// If title is wrong or not exist - just print error message
		//
		if (block.title == "Error: Title was not found. Check the code.")
		{
			dataForOutputFile << block.title;
		}
		else
		{
			dataForOutputFile << block.title.remove(block.title.indexOf("Title: "), 7).replace("\\n", "<br>");
		}

		if (block.error == true)
		{
			dataForOutputFile << "</font>";
		}

		dataForOutputFile << "</td>";
	}

	// When table was generated - let's start showing full block information
	//
	dataForOutputFile << "\n\t</table>";
	dataForOutputFile << "\n\t<h3>Descriptions</h3>";

	// Notice: if we had block with corrupted "IssueCode" header - we
	// must match it is anchor with error number
	//
	errorCounter = 0;

	for (BlockFromFile block : m_commentBlocks)
	{
		// Show every message. If message contains error(s) - match it with red color
		//
		if (block.error == true)
		{
			dataForOutputFile << "<font color=\"red\">";
		}
		QString htmlAnchor = block.issueCode;

		if (htmlAnchor == "Error")
		{
			htmlAnchor = "error_" + QString::number(errorCounter);
			errorCounter++;
		}
		else
		{
			htmlAnchor.remove(block.issueCode.indexOf("IssueCode: "), 10).replace(" ", "");
		}

		dataForOutputFile << "\n\t<a id=\"" << htmlAnchor << "\"></a>"
						  << "\n\t<br><b>" << issueCode << ": </b>" << block.issueCode.replace("\\n", "<br>").remove(issueCode + ":")
						  << "\n\t<br><b>" << issueType << ": </b>" << block.issueType.replace("\\n", "<br>").remove(issueType + ":")
						  << "\n\t<br><b>" << title << ": </b>" << block.title.replace("\\n", "<br>").remove(title + ":")
						  << "\n\t<br><b>" << parameters << ": </b>" << block.parameters.replace("\\n", "<br>").remove(parameters + ":")
						  << "\n\t<br><b>" << description << ": </b>" << block.description.replace("\\n", "<br>").remove(description + ":");

		if (block.error == true)
		{
			// If block contains error(s) - show error message
			//
			dataForOutputFile << "\n\t<br>" << block.errorString;
			dataForOutputFile << "</font>";
		}

		dataForOutputFile << "\n\t<br><br><br>";
	}

	// End writing document
	//
	dataForOutputFile << "\n</body>";
	dataForOutputFile << "\n</html>";

	return 0;
}
