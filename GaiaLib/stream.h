#pragma once
#include "def.h"
namespace Gaia
{
#define STREAM_BLOCK_SIZE 1024
	class Stream
	{
	public:
		Stream();
		~Stream();
	public:
		int readInt();
		void writeInt(int data);

		unsigned int readUInt();
		void writeUInt(unsigned int data);

		short readShort();
		void writeShort(short data);

		unsigned short readUShort();
		void writeUShort(unsigned short data);

		char readByte();
		void writeByte(char c);

		void readBytes(char *pData, int size);
		void writeBytes(char *pData, int size);

		void writeString(char *pData);

		float readFloat();
		void writeFloat(float Data);

		double readDouble();
		void writeDouble(double data);

		void setPosition(unsigned int val);
		unsigned int getPosition();
		unsigned int getLength();

	private:
		inline void checkBuffer(int size);
	public:
		char *bufferPointer_;
		int bufferLength_;
		int  position_;
		int size_;
	};
}


