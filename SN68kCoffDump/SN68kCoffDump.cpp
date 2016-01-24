// SN68kCoffDump.cpp : Defines the entry point for the console application.
//

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include "stdafx.h"
#include "FileCOFF.h"

int Error(const std::string& message)
{
	std::cout << "Error: " << message << std::endl;
	return -1;
}

int _tmain(int argc, _TCHAR* argv[])
{
	std::cout << "SNASM2 68000 COFF File Info Dump Tool" << std::endl;
	std::cout << "23/Jan/2016" << std::endl;
	std::cout << "Matt Phillips, Big Evil Corporation" << std::endl << std::endl;

	if(argc > 1)
	{
		std::wstring filename = argv[1];

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
				return Error("Unknown COFF file format");
			}

			//Dump file info
			std::stringstream textStream;
			coffFile.Dump(textStream);
			
			//Dump to TTY
			std::cout << textStream.str() << std::endl;

			if(argc > 2)
			{
				//Dump to file
				std::wstring outFilename = argv[2];

				std::ofstream outFile(outFilename, std::ios::out);
				if(outFile.is_open())
				{
					outFile << textStream.str();
					outFile.close();
				}
			}
		}
	}

	return 0;
}

