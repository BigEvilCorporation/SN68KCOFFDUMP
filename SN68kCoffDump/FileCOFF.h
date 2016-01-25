
#pragma once

#include <sstream>
#include <vector>
#include <map>

#include "atoms.h"
#include "archive.h"

#define COFF_MACHINE_68000		0x150
#define COFF_SECTION_NAME_SIZE	8

//SNASM2 hard coded section idxs
#define COFF_SECTION_FILENAMES	0
#define COFF_SECTION_GLOBALS	1
#define COFF_SECTION_ROM_DATA	2
#define COFF_SECTION_COUNT		3

//Section header flags
#define COFF_SECTION_FLAG_DUMMY	0x00000001
#define COFF_SECTION_FLAG_GROUP	0x00000004
#define COFF_SECTION_FLAG_TEXT	0x00000020
#define COFF_SECTION_FLAG_DATA	0x00000040
#define COFF_SECTION_FLAG_BSS	0x00000080
#define COFF_SECTION_FLAG_WRITE	0x80000000

class FileCOFF
{
public:
	FileCOFF();
	~FileCOFF();

	void Serialise(Stream& stream);
	void Dump(std::stringstream& stream);

	struct FileHeader
	{
		void Serialise(Stream& stream);
		void Dump(std::stringstream& stream);

		u16 machineType;
		u16 numSections;
		u32 timeDate;
		u32 symbolTableOffset;
		u32 numSymbols;
		u16 exHeaderSize;
		u16 flags;
	};

	struct ExecutableHeader
	{
		void Serialise(Stream& stream);
		void Dump(std::stringstream& stream);

		u16 exHeaderMagic;
		u16 exHeaderVersion;
		u32 textDataSize;
		u32 initialisedDataSize;
		u32 uninitialisedDataSize;
		u32 entryPointAddr;
		u32 textDataAddr;
		u32 dataAddr;
	};

	struct SectionHeader
	{
		SectionHeader()
		{
			data = NULL;
		}

		void Serialise(Stream& stream);
		void Dump(std::stringstream& stream);

		std::string name;
		u32 physicalAddr;
		u32 virtualAddr;
		u32 size;
		u32 sectiondataOffset;
		u32 relocationTableOffset;
		u32 lineNumberTableOffset;
		u16 numRelocationEntries;
		u16 numLineNumberTableEntries;
		u32 flags;

		u8* data;
	};

	struct Symbol
	{
		void Serialise(Stream& stream);
		bool operator < (const Symbol& rhs) const { return value < rhs.value; }

		std::string name;
		u32 stringTableOffset;
		u32 value;
		s16 sectionIndex;
		u16 symbolType;
		s8 storageClass;
		s8 auxCount;
	};

	union SymbolNameStringDef
	{
		char name[COFF_SECTION_NAME_SIZE + 1];

		struct
		{
			u32 freeStringSpace;
			u32 stringTableOffset;
		};
	};

	struct LineNumberEntry
	{
		LineNumberEntry()
		{
			physicalAddress = 0;
			sectionMarker = 0;
			lineNumberSectionIdx = 0;
			filename = NULL;
		}

		void Serialise(Stream& stream);

		union
		{
			u32 physicalAddress;
			u32 filenameIndex;
		};

		union
		{
			s16 sectionMarker;
			s16 lineNumber;
		};

		u32 lineNumberSectionIdx;
		std::string* filename;
	};

	FileHeader m_fileHeader;
	ExecutableHeader m_executableHeader;
	std::vector<SectionHeader> m_sectionHeaders;
	std::vector<LineNumberEntry> m_lineNumberSectionHeaders;
	std::map<u32, LineNumberEntry> m_lineNumberAddressMap;
	std::vector<Symbol> m_symbols;
	std::vector<Symbol> m_sortedSymbols;
	std::vector<std::string> m_filenameTable;
	u8* m_stringTableRaw;
};