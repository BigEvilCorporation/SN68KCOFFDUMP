
#include <iostream>

#include "FileCOFF.h"
#include "timeutils.h"

FileCOFF::FileCOFF()
{
	m_stringTableRaw = NULL;
}

FileCOFF::~FileCOFF()
{
	if(m_stringTableRaw)
	{
		delete m_stringTableRaw;
	}
}

void FileCOFF::Serialise(Stream& stream)
{
	//Serialise file header
	stream.Serialise(m_fileHeader);

	//Serialise executable header
	if(m_fileHeader.exHeaderSize > 0)
	{
		stream.Serialise(m_executableHeader);
	}

	//Allocate section headers
	if(stream.GetDirection() == Stream::STREAM_IN)
	{
		m_sectionHeaders.resize(m_fileHeader.numSections);
	}

	//Serialise section headers
	for(int i = 0; i < m_fileHeader.numSections; i++)
	{
		stream.Serialise(m_sectionHeaders[i]);
	}

	if(stream.GetDirection() == Stream::STREAM_IN)
	{
		//Allocate symbol table
		m_symbols.resize(m_fileHeader.numSymbols);

		//Seek to symbol table
		stream.Seek(m_fileHeader.symbolTableOffset, Stream::SEEK_START);
	}

	//Serialise symbol table
	for(int i = 0; i < m_fileHeader.numSymbols; i++)
	{
		stream.Serialise(m_symbols[i]);
	}

	//Serialise string table size
	u32 stringTableSizeBytes;
	stream.Serialise(stringTableSizeBytes);

	//Allocate string table
	if(stream.GetDirection() == Stream::STREAM_IN)
	{
		m_stringTableRaw = new u8[stringTableSizeBytes];
	}

	//Serialise string table
	stream.Serialise(m_stringTableRaw, stringTableSizeBytes);

	//Resolve symbol strings
	for(int i = 0; i < m_fileHeader.numSymbols; i++)
	{
		if(m_symbols[i].stringTableOffset != -1)
		{
			m_symbols[i].name = (char*)m_stringTableRaw + (m_symbols[i].stringTableOffset - sizeof(u32));
		}
	}

	//Serialise line number sections
	for(int i = 0; i < m_fileHeader.numSections; i++)
	{
		if(m_sectionHeaders[i].numLineNumberTableEntries > 0)
		{
			if(stream.GetDirection() == Stream::STREAM_IN)
			{
				//Seek to line number section start
				stream.Seek(m_sectionHeaders[i].lineNumberTableOffset, Stream::SEEK_START);

				//Read all entries
				for(int j = 0; j < m_sectionHeaders[i].numLineNumberTableEntries; j++)
				{
					//Serialise entry
					LineNumberEntry lineNumberEntry;
					stream.Serialise(lineNumberEntry);

					//If a line number section header
					if(lineNumberEntry.sectionMarker <= 0)
					{
						//Add new section
						m_lineNumberSectionHeaders.push_back(lineNumberEntry);
					}
					else
					{
						//Set line number section
						lineNumberEntry.lineNumberSectionIdx = m_lineNumberSectionHeaders.size() - 1;

						LineNumberEntry& lineNumberSectionHeader = m_lineNumberSectionHeaders[lineNumberEntry.lineNumberSectionIdx];
						lineNumberEntry.symbol = &m_symbols[lineNumberSectionHeader.symbolTableIndex];

						//Insert into address map
						m_lineNumberAddressMap[lineNumberEntry.physicalAddress] = lineNumberEntry;
					}
				}
			}
			else
			{
				//TODO: Stream out
			}
		}
	}
}

void FileCOFF::Dump(std::stringstream& stream)
{
	m_fileHeader.Dump(stream);

	if(m_fileHeader.exHeaderSize > 0)
	{
		m_executableHeader.Dump(stream);
	}

	for(int i = 0; i < m_fileHeader.numSections; i++)
	{
		m_sectionHeaders[i].Dump(stream);
		stream << std::endl;
	}
}

void FileCOFF::FileHeader::Serialise(Stream& stream)
{
	stream.Serialise(fileVersion);
	stream.Serialise(numSections);
	stream.Serialise(timeDate);
	stream.Serialise(symbolTableOffset);
	stream.Serialise(numSymbols);
	stream.Serialise(exHeaderSize);
	stream.Serialise(flags);
}

void FileCOFF::FileHeader::Dump(std::stringstream& stream)
{
	SYSTEMTIME timeStamp;
	UnixTimeToSystemTime(timeDate, timeStamp);

	stream << "-------------------------------------" << std::endl;
	stream << "HEADER" << std::endl;
	stream << "-------------------------------------" << std::endl;
	stream << "COFF filetype: 0x" << std::hex << fileVersion << std::dec << std::endl;
	stream << "Num sections: " << numSections << std::endl;
	stream << "Timestamp: " << timeStamp.wHour << ":" << timeStamp.wMinute << ":" << timeStamp.wSecond << " " << timeStamp.wDay << "/" << timeStamp.wMonth << "/" << timeStamp.wYear << std::endl;
	stream << "Symbol table offset: " << symbolTableOffset << std::endl;
	stream << "Num symbols: " << numSymbols << std::endl;
	stream << "Executable header size: " << exHeaderSize << std::endl;
	stream << "Flags: 0x" << std::hex << flags << std::dec << std::endl;

	stream << std::endl;
}

void FileCOFF::ExecutableHeader::Serialise(Stream& stream)
{
	stream.Serialise(exHeaderMagic);
	stream.Serialise(exHeaderVersion);
	stream.Serialise(textDataSize);
	stream.Serialise(initialisedDataSize);
	stream.Serialise(uninitialisedDataSize);
	stream.Serialise(entryPointAddr);
	stream.Serialise(textDataAddr);
	stream.Serialise(dataAddr);
}

void FileCOFF::ExecutableHeader::Dump(std::stringstream& stream)
{
	stream << "-------------------------------------" << std::endl;
	stream << "EXECUTABLE HEADER" << std::endl;
	stream << "-------------------------------------" << std::endl;
	stream << "Magic: 0x" << std::hex << exHeaderMagic << std::dec << std::endl;
	stream << "Version: 0x" << std::hex << exHeaderVersion << std::dec << std::endl;
	stream << "Text data size: " << textDataSize << std::endl;
	stream << "Initialised data size: " << initialisedDataSize << std::endl;
	stream << "Uninitialised data size: " << uninitialisedDataSize << std::endl;
	stream << "Entry point address: 0x" << std::hex << entryPointAddr << std::dec << std::endl;
	stream << "Text data address: 0x" << std::hex << textDataAddr << std::dec << std::endl;
	stream << "Data address: 0x" << std::hex << dataAddr << std::dec << std::endl;

	stream << std::endl;
}

void FileCOFF::SectionHeader::Serialise(Stream& stream)
{
	stream.Serialise(name, COFF_SECTION_NAME_SIZE);
	stream.Serialise(physicalAddr);
	stream.Serialise(virtualAddr);
	stream.Serialise(size);
	stream.Serialise(sectiondataOffset);
	stream.Serialise(relocationTableOffset);
	stream.Serialise(lineNumberTableOffset);
	stream.Serialise(numRelocationEntries);
	stream.Serialise(numLineNumberTableEntries);
	stream.Serialise(flags);
}

void FileCOFF::SectionHeader::Dump(std::stringstream& stream)
{
	std::string flagsString;

	if(flags & COFF_SECTION_FLAG_DUMMY)
		flagsString += "DUMMY SECTION";
	if(flags & COFF_SECTION_FLAG_GROUP)
		flagsString += "GROUP SECTION";
	if(flags & COFF_SECTION_FLAG_TEXT)
		flagsString += "TEXT SECTION";
	if(flags & COFF_SECTION_FLAG_DATA)
		flagsString += "DATA SECTION";
	if(flags & COFF_SECTION_FLAG_BSS)
		flagsString += "BSS SECTION";
	if(flags & COFF_SECTION_FLAG_WRITE)
		flagsString += " + WRITEABLE";

	stream << "-------------------------------------" << std::endl;
	stream << "SECTION HEADER: " << name.c_str() << std::endl;
	stream << "-------------------------------------" << std::endl;
	stream << "Physical address: 0x" << std::hex << physicalAddr << std::dec << std::endl;
	stream << "Virtual address: 0x" << std::hex << virtualAddr << std::dec << std::endl;
	stream << "Size: " << size << std::endl;
	stream << "Section data offset: " << sectiondataOffset << std::endl;
	stream << "Relocation table offset: " << relocationTableOffset << std::endl;
	stream << "Line number table offset: " << lineNumberTableOffset << std::endl;
	stream << "Num relocation table entries: " << numRelocationEntries << std::endl;
	stream << "Num line number table entries: " << numLineNumberTableEntries << std::endl;
	stream << "Flags: 0x" << std::hex << flags << std::dec << " (" << flagsString.c_str() << ")" << std::endl;

	stream << std::endl;
}

void FileCOFF::LineNumberEntry::Serialise(Stream& stream)
{
	stream.Serialise(physicalAddress);
	stream.Serialise(lineNumber);
}

void FileCOFF::Symbol::Serialise(Stream& stream)
{
	SymbolNameStringDef symbolStringDef;
	stream.Serialise((u8*)symbolStringDef.name, COFF_SECTION_NAME_SIZE);

	//NULL terminate string
	symbolStringDef.name[COFF_SECTION_NAME_SIZE] = 0;

	if(symbolStringDef.freeStringSpace == 0)
	{
		//Name doesn't fit, get string table offset
		stringTableOffset = symbolStringDef.stringTableOffset;
	}
	else
	{
		//Name fits here
		name = symbolStringDef.name;
		stringTableOffset = (u32)-1;
	}

	stream.Serialise(value);
	stream.Serialise(sectionIndex);
	stream.Serialise(symbolType);
	stream.Serialise(storageClass);
	stream.Serialise(auxCount);
}
