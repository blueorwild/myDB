#include "seq_num.h"

SequenceNumber* SequenceNumber::p = new SequenceNumber;

SequenceNumber::SequenceNumber()
{
	read_SN = 0;
	write_SN = 1;
}

uint64_t SequenceNumber::GetWriteSN() {
	return write_SN++;
}

uint64_t SequenceNumber::GetReadSN() {
	return read_SN;
}

void SequenceNumber::SetReadSN(uint64_t s) {
	read_SN = s;
}