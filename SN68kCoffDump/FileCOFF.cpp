
#include <iostream>

#include "FileCOFF.h"
#include "timeutils.h"

FileCOFF::FileCOFF()
{

}

FileCOFF::~FileCOFF()
{

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