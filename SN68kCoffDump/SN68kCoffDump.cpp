// SN68kCoffDump.cpp : Defines the entry point for the console application.
//

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include "stdafx.h"
#include "FileCOFF.h"

void PrintUsage(std::stringstream& stream)
{
	stream << "Usage:" << std::endl;
	stream << "\tsn68kcoffdump filename.cof [options]" << std::endl;
	stream << "Options:" << std::endl;
	stream << "\t-summary\t\tPrints COFF summary" << std::endl;
	stream << "\t-symbols\t\tPrints symbol table" << std::endl;
	stream << "\t-addr2line [address]\tPrints file/line and symbol from physical address" << std::endl;
}

int _tmain(int argc, _TCHAR* argv[])
{
	std::stringstream textStream;

	textStream << "SNASM2 68000 COFF File Info Dump Tool" << std::endl;
	textStream << "23/Jan/2016" << std::endl;
	textStream << "Matt Phillips, Big Evil Corporation" << std::endl << std::endl;

	//Args
	std::wstring filename;
	bool argDumpSummary = false;
	bool argDumpSymbols = false;
	bool argAddressToLine = false;
	u32  argAddress = 0;

	//Need at least 3 args (exe + input filename + operation)
	if(argc > 3)
	{
		//Input filename
		filename = argv[1];

		//Tokenise remaining arguments
		for(int i = 2; i < argc; i++)
		{
			if(_wcsicmp(argv[i], L"-summary") == 0)
				argDumpSummary = true;
			if(_wcsicmp(argv[i], L"-symbols") == 0)
				argDumpSymbols = true;
			if(_wcsicmp(argv[i], L"-addr2line") == 0)
			{
				//Need address arg
				if(i < (argc-1))
				{
					i++;
					argAddressToLine = true;
					argAddress = _wtoi(argv[i]);
				}
			}
		}

		if(!argDumpSummary && !argDumpSymbols && !argAddressToLine)
		{
			//No op, print usage
			PrintUsage(textStream);
		}
	}
	else
	{
		//No filename arg, print usage
		PrintUsage(textStream);
	}

	if(filename.size() > 0)
	{
		//Open file (at end)
		std::ifstream file(filename, std::ios::in | std::ios::binary | std::ios::ate);
		if(file.is_open())
		{
			//Get filesize
			std::streampos fileSize = file.tellg();

			//Alloc buffer
			char* data = new char[fileSize];

			//Seek to start
			file.seekg(0, std::ios::beg);
			
			//Read file
			file.read(data, fileSize);

			//Close file
			file.close();

			//Serialise COFF file
			FileCOFF coffFile;
			Stream stream((char*)&data[0]);
			stream.Serialise(coffFile);

			//Free buffer
			delete data;

			//Sanity check COFF format type
			if(coffFile.m_fileHeader.fileVersion != COFF_VERSION_SNASM2)
			{
				textStream << "Unknown COFF file format";
			}
			else
			{
				if(argDumpSummary)
				{
					//Dump file info
					coffFile.Dump(textStream);
				}

				if(argDumpSymbols)
				{
					//Dump symbols
					textStream << "-------------------------------------" << std::endl;
					textStream << "SYMBOLS" << std::endl;
					textStream << "-------------------------------------" << std::endl;

					for(int i = 0; i < coffFile.m_symbols.size(); i++)
					{
						textStream << coffFile.m_symbols[i].name.c_str() << " = 0x" << std::hex << coffFile.m_symbols[i].value << std::dec << std::endl;
					}
				}

				if(argAddressToLine)
				{
					//Find line
					std::map<u32, FileCOFF::LineNumberEntry>::iterator it = coffFile.m_lineNumberAddressMap.find(argAddress);
					if(it == coffFile.m_lineNumberAddressMap.end())
					{
						//Line/symbol not found
						textStream << "Symbol at address 0x" << std::hex << argAddress << std::dec << " not found" << std::endl;
					}
					else
					{
						//Line/symbol found
						textStream << "Symbol at address 0x" << std::hex << argAddress << std::dec << std::endl;
						textStream << "Name: " << it->second.symbol->name << std::endl;
						textStream << "Line: " << it->second.lineNumber << std::endl;
					}
				}
			}
		}
	}

	//Dump to TTY
	std::cout << textStream.str() << std::endl;

	return 0;
}

