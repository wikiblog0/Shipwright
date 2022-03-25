#include "DisplayList.h"
#include "PR/ultra64/gbi.h"

namespace Ship
{
	uint64_t DisplayListV0::ReadCommand(BinaryReader* reader)
	{
		uint32_t word0 = reader->ReadUInt32();
		uint32_t word1 = reader->ReadUInt32();
		return word1 | ((uint64_t) word0 << 32);
	}

	void DisplayListV0::ParseFileBinary(BinaryReader* reader, Resource* res)
	{
		DisplayList* dl = (DisplayList*)res;

		ResourceFile::ParseFileBinary(reader, res);

		while (reader->GetBaseAddress() % 8 != 0)
			reader->ReadByte();

		while (true)
		{
			uint64_t data = ReadCommand(reader);

			dl->instructions.push_back(data);
				
			uint8_t opcode = data >> 24;

			// These are 128-bit commands, so read an extra 64 bits...
			if (opcode == G_SETTIMG_OTR || opcode == G_DL_OTR || opcode == G_VTX_OTR || opcode == G_BRANCH_Z_OTR || opcode == G_MARKER || opcode == G_MTX_OTR)
				dl->instructions.push_back(ReadCommand(reader));

			if (opcode == G_ENDDL)
				break;
		}
	}
}