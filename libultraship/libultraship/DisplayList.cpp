#include "DisplayList.h"
#include "PR/ultra64/gbi.h"

namespace Ship
{
	void DisplayListV0::ParseFileBinary(BinaryReader* reader, Resource* res)
	{
		DisplayList* dl = (DisplayList*)res;

		ResourceFile::ParseFileBinary(reader, res);

		while (reader->GetBaseAddress() % 8 != 0)
			reader->ReadByte();

		while (true)
		{
			uint32_t word0 = reader->ReadUInt32();
			uint32_t word1 = reader->ReadUInt32();

			dl->instructions.push_back(((uint64_t) word0 << 32) | word1);
				
			uint8_t opcode = word0 >> 24;

			// These are 128-bit commands, so read an extra 64 bits...
			if (opcode == G_SETTIMG_OTR || opcode == G_DL_OTR || opcode == G_VTX_OTR || opcode == G_BRANCH_Z_OTR || opcode == G_MARKER || opcode == G_MTX_OTR) {
				word0 = reader->ReadUInt32();
				word1 = reader->ReadUInt32();

				dl->instructions.push_back(((uint64_t) word0 << 32) | word1);
			}

			if (opcode == G_ENDDL)
				break;
		}
	}
}