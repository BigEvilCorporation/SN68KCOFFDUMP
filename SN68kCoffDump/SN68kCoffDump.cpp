// SN68kCoffDump.cpp : Defines the entry point for the console application.
//

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <algorithm>

#include "stdafx.h"
#include "FileCOFF.h"

void PrintUsage(std::stringstream& stream)
{
	stream << "Usage:" << std::endl;
	stream << "\tsn68kcoffdump filename.cof [options]" << std::endl;
	stream << "Options:" << std::endl;
	stream << "\t-summary\t\tPrints COFF summary" << std::endl;
	stream << "\t-symbols\t\tPrints symbol table" << std::endl;
	stream << "\t-extractrom [filename]\tExtracts ROM file" << std::endl;
	stream << "\t-addr2line [hex address]\tPrints file/line and symbol from physical address" << std::endl;
}

int _tmain(int argc, _TCHAR* argv[])
{
	std::stringstream textStream;

	textStream << "-------------------------------------" << std::endl;
	textStream << "SNASM2 68000 COFF File Info Dump Tool" << std::endl;
	textStream << "-------------------------------------" << std::endl;
	textStream << "Release 0.1a, 23/Jan/2016" << std::endl;
	textStream << "Matt Phillips, Big Evil Corporation" << std::endl;
	textStream << "http://www.bigevilcorporation.co.uk" << std::endl;
	textStream << "-------------------------------------" << std::endl << std::endl;

	//Args
	std::string filename;
	bool argDumpSummary = false;
	bool argDumpSymbols = false;
	bool argExtractROM = false;
	std::string argROMFilename;
	bool argAddressToLine = false;
	u32  argAddress = 0;
	bool argError = false;

	//Need at least 3 args (exe + input filename + operation)
	if(argc > 2)
	{
		//Input filename
		filename = argv[1];

		//Tokenise remaining arguments
		for(int i = 2; i < argc; i++)
		{
			if(_stricmp(argv[i], "-summary") == 0)
				argDumpSummary = true;
			else if(_stricmp(argv[i], "-symbols") == 0)
				argDumpSymbols = true;
			else if(_stricmp(argv[i], "-extractrom") == 0)
			{
				//Need filename arg
				if(i < (argc - 1))
				{
					i++;
					argExtractROM = true;
					argROMFilename = argv[i];
				}
			}
			else if(_stricmp(argv[i], "-addr2line") == 0)
			{
				//Need address arg
				if(i < (argc-1))
				{
					i++;
					argAddressToLine = true;
					argAddress = strtol(argv[i], NULL, 16);
				}
			}
			else
			{
				argError = true;
			}
		}

		if(argError || (!argDumpSummary && !argDumpSymbols && !argAddressToLine && !argExtractROM))
		{
			//No operation specified, or arg error, print usage
			PrintUsage(textStream);
		}
	}
	else
	{
		//No filename arg, print usage
		PrintUsage(textStream);
	}

	if(filename.size() > 0 && !argError)
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

			//Sanity checks
			if(coffFile.m_fileHeader.machineType != COFF_MACHINE_68000)
			{
				//Unsupported machine/processor type
				textStream << "Unknown COFF machine/processor type, not a SNASM68K COFF";
			}
			else if(coffFile.m_sectionHeaders.size() != COFF_SECTION_COUNT)
			{
				//SNASM2 COFF has a fixed number of sections
				textStream << "Unsupported section count, not a SNASM68K COFF";
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

					for(int i = 0; i < coffFile.m_sortedSymbols.size(); i++)
					{
						textStream << "0x" << std::hex << coffFile.m_sortedSymbols[i].value << std::dec << "\t" << coffFile.m_sortedSymbols[i].name.c_str() << std::endl;
					}
				}

				if(argExtractROM)
				{
					//Extract ROM
					std::ofstream outFile(argROMFilename, std::ios::out | std::ios::binary);
					if(outFile.is_open())
					{
						u8* romData = coffFile.m_sectionHeaders[COFF_SECTION_ROM_DATA].data;
						u32 romSize = coffFile.m_sectionHeaders[COFF_SECTION_ROM_DATA].size;
						
						outFile.write((const char*)romData, romSize);
						outFile.close();

						textStream << "ROM extracted" << std::endl;
						textStream << "Filename: " << argROMFilename.c_str() << std::endl;
						textStream << "Size: " << romSize << " bytes" << std::endl;
					}
					else
					{
						textStream << "Error: Could not create file " << argROMFilename.c_str() << std::endl;
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
						//Line found, get nearest symbol
						std::string nearestSymbol = "Not found";
						FileCOFF::Symbol findSymbol;
						findSymbol.value = argAddress;
						auto symbolIt = std::lower_bound(coffFile.m_sortedSymbols.begin(), coffFile.m_sortedSymbols.end(), findSymbol);

						textStream << "Address 0x" << std::hex << argAddress << std::dec << std::endl;
						textStream << "Filename: " << it->second.filename->c_str() << std::endl;
						textStream << "Line: " << it->second.lineNumber << std::endl;

						if(symbolIt != coffFile.m_sortedSymbols.end())
						{
							//Get last symbol
							symbolIt--;

							textStream << "Nearest symbol name: " << symbolIt->name.c_str() << std::endl;
							textStream << "Nearest symbol address: " << std::hex << symbolIt->value << std::dec << std::endl;
						}
						else
						{
							textStream << "Nearest symbol: Not found" << std::endl;
						}
					}
				}
			}
		}
	}

	//Dump to TTY
	std::cout << textStream.str() << std::endl;

	return 0;
}

