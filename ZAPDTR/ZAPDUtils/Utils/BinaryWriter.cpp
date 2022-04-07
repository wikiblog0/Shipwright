#include "BinaryWriter.h"

#ifdef _MSC_VER
#define __builtin_bswap16 _byteswap_ushort
#define __builtin_bswap32 _byteswap_ulong
#define __builtin_bswap64 _byteswap_uint64
#endif

BinaryWriter::BinaryWriter(Stream* nStream, Endianess endianess)
{
	this->endianess = endianess;
	stream.reset(nStream);
}

BinaryWriter::BinaryWriter(std::shared_ptr<Stream> nStream, Endianess endianess)
{
	this->endianess = endianess;
	stream = nStream;
}

void BinaryWriter::Close()
{
	stream->Close();
}

std::shared_ptr<Stream> BinaryWriter::GetStream()
{
	return stream;
}

uint64_t BinaryWriter::GetBaseAddress()
{
	return stream->GetBaseAddress();
}

uint64_t BinaryWriter::GetLength()
{
	return stream->GetLength();
}

void BinaryWriter::Seek(int32_t offset, SeekOffsetType seekType)
{
	stream->Seek(offset, seekType);
}

void BinaryWriter::Write(int8_t value)
{
	stream->Write((char*)&value, sizeof(int8_t));
}

void BinaryWriter::Write(uint8_t value)
{
	stream->Write((char*)&value, sizeof(uint8_t));
}

void BinaryWriter::Write(int16_t value)
{
	if (endianess != Endianess::Little)
		value = __builtin_bswap16(value);

	stream->Write((char*)&value, sizeof(int16_t));
}

void BinaryWriter::Write(uint16_t value)
{
	if (endianess != Endianess::Little)
		value = __builtin_bswap16(value);

	stream->Write((char*)&value, sizeof(uint16_t));
}

void BinaryWriter::Write(int32_t value)
{
	if (endianess != Endianess::Little)
		value = __builtin_bswap32(value);

	stream->Write((char*)&value, sizeof(int32_t));
}

void BinaryWriter::Write(int32_t valueA, int32_t valueB)
{
	Write(valueA);
	Write(valueB);
}

void BinaryWriter::Write(uint32_t value)
{
	if (endianess != Endianess::Little)
		value = __builtin_bswap32(value);

	stream->Write((char*)&value, sizeof(uint32_t));
}

void BinaryWriter::Write(int64_t value)
{
	if (endianess != Endianess::Little)
		value = __builtin_bswap64(value);

	stream->Write((char*)&value, sizeof(int64_t));
}

void BinaryWriter::Write(uint64_t value)
{
	if (endianess != Endianess::Little)
		value = __builtin_bswap64(value);

	stream->Write((char*)&value, sizeof(uint64_t));
}

void BinaryWriter::Write(float value)
{
	if (endianess != Endianess::Little)
	{
		uint32_t tmp = __builtin_bswap32(*(uint32_t*)&value);
		value = *(float*)&tmp;
	}

	stream->Write((char*)&value, sizeof(float));
}

void BinaryWriter::Write(double value)
{
	if (endianess != Endianess::Little)
	{
		uint64_t tmp = __builtin_bswap64(*(uint64_t*)&value);
		value = *(double*)&tmp;
	}

	stream->Write((char*)&value, sizeof(double));
}

void BinaryWriter::Write(const std::string& str)
{
	int strLen = str.size();
	Write(strLen);

	for (char c : str)
		stream->WriteByte(c);
}

void BinaryWriter::Write(char* srcBuffer, size_t length)
{
	stream->Write(srcBuffer, length);
}
