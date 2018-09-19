#include "Stream.h"

using namespace Gaia;

Stream::Stream():bufferPointer_(NULL), position_(0), size_(0){}


Stream::~Stream(void)
{
	if (bufferPointer_)
		delete [] bufferPointer_;
}

inline void Stream::checkBuffer(int size)
{
	if (bufferPointer_)
	{
		int oLength = bufferLength_;
		while (size_ + size > bufferLength_)
			bufferLength_ += STREAM_BLOCK_SIZE;
		if (bufferLength_ > oLength)
		{
			char *temp = new char[bufferLength_];
			memcpy(temp, bufferPointer_, oLength);
			delete [] bufferPointer_;
			bufferPointer_ = temp;
		}
	}
	else 
	{
		bufferPointer_ = new char[STREAM_BLOCK_SIZE];
		bufferLength_ = STREAM_BLOCK_SIZE;
		checkBuffer(size);
	}
}

int Stream::readInt()
{
	int result;
	memcpy(&result, bufferPointer_ + position_, sizeof(int));
	position_ += sizeof(int);
	return result;
}

void Stream::writeInt(int data)
{
	checkBuffer(sizeof(int));
	memcpy(bufferPointer_ + position_, &data, sizeof(int));
	position_ += sizeof(int);
	if (position_ > size_)
		size_ = position_;
}

unsigned int Stream::readUInt()
{
	unsigned int result;
	memcpy(&result, bufferPointer_ + position_, sizeof(unsigned int));
	position_ += sizeof(unsigned int);
	return result;
}

void Stream::writeUInt(unsigned int data)
{
	checkBuffer(sizeof(unsigned int));
	memcpy(bufferPointer_ + position_, &data, sizeof(unsigned int));
	position_ += sizeof(unsigned int);
	if (position_ > size_)
		size_ = position_;
}

short Stream::readShort()
{
	short result;
	memcpy(&result, bufferPointer_ + position_, sizeof(short));
	position_ += sizeof(short);
	return result;
}

void Stream::writeShort(short data)
{
	checkBuffer(sizeof(short));
	memcpy(bufferPointer_ + position_, &data, sizeof(short));
	position_ += sizeof(short);
	if (position_ > size_)
		size_ = position_;
}

unsigned short Stream::readUShort()
{
	unsigned short result;
	memcpy(&result, bufferPointer_ + position_, sizeof(unsigned short));
	position_ += sizeof(unsigned short);
	return result;
}

void Stream::writeUShort(unsigned short data)
{
	checkBuffer(sizeof(unsigned short));
	memcpy(bufferPointer_ + position_, &data, sizeof(unsigned short));
	position_ += sizeof(unsigned short);
	if (position_ > size_)
		size_ = position_;
}

char Stream::readByte()
{
	char c;
	memcpy(&c, bufferPointer_ + position_, sizeof(char));
	position_ += sizeof(char);
	return c;
}

void Stream::writeByte(char c)
{
	checkBuffer(sizeof(char));
	memcpy(bufferPointer_ + position_, &c, sizeof(char));
	position_ += sizeof(char);
	if (position_ > size_)
		size_ = position_;
}

void Stream::readBytes(char *pData, int size)
{
	memcpy(pData, bufferPointer_ + position_, size);
	position_ += size;
}

void Stream::writeBytes(char *pData, int size)
{
	checkBuffer(size);
	memcpy(bufferPointer_ + position_, pData, size);
	position_ += size;
	if (position_ > size_)
		size_ = position_;
}


void Stream::writeString(char *pData)
{
	int size = strlen(pData) + 1;
	checkBuffer(size);
	memcpy(bufferPointer_ + position_, pData, size);
	position_ += size;
	if (position_ > size_)
		size_ = position_;
}

float Stream::readFloat()
{
	float result;
	memcpy(&result, bufferPointer_ + position_, sizeof(float));
	position_ += sizeof(float);
	return result;
}

void Stream::writeFloat(float data)
{
	checkBuffer(sizeof(float));
	memcpy(bufferPointer_ + position_, &data, sizeof(float));
	position_ += sizeof(float);
	if (position_ > size_)
		size_ = position_;

}

double Stream::readDouble()
{
	double result;
	memcpy(&result, bufferPointer_ + position_, sizeof(double));
	position_ += sizeof(double);
	return result;
}

void Stream::writeDouble(double data)
{
	checkBuffer(sizeof(double));
	memcpy(bufferPointer_ + position_, &data, sizeof(double));
	position_ += sizeof(double);
	if (position_ > size_)
		size_ = position_;
}

void Stream::setPosition(unsigned int val)
{
	if (val > (unsigned int)size_)
		return;
	position_ = val;
}

unsigned int Stream::getPosition()
{
	return position_;
}

unsigned int Stream::getLength()
{
	return size_;
}