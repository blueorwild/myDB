#include <string.h>
extern int flag[524288] = { 0 };

// SAX hash
unsigned int SaxHash(const char *key)
{
	unsigned int h = 0;
	while (*key) {
		h ^= (h << 5) + (h >> 2) + (unsigned char)*key;
		++key;
	}
	flag[h & 524287] += 1;
	return h;
}

// SDBM hash
unsigned int SdbmHash(const char *key)
{
	unsigned int h = 0;
	while (*key) {
		h = (unsigned char)*key + (h << 6) + (h << 16) - h;
		++key;
	}
	flag[h & 524287] += 1;
	return h;
}

// Murmur2 hash
unsigned int MurmurHash(const char *key)
{
	if (!key) {
		return 0;
	}
	unsigned int m = 0x5bd1e995;
	unsigned int r = 24;
	unsigned int seed = 0xdeadbeef;
	size_t len = strlen(key);
	unsigned h = seed ^ len;
	while (len >= 4) {
		unsigned k = *(unsigned*)key;
		k *= m;
		k ^= k >> r;
		k *= m;
		h *= m;
		h ^= k;
		key += 4;
		len -= 4;
	}
	switch (len) {
	case 3:
		h ^= key[2] << 16;
	case 2:
		h ^= key[1] << 8;
	case 1:
		h ^= key[0];
		h *= m;
	};
	h ^= h >> 13;
	h *= m;
	h ^= h >> 15;
	flag[h & 524287] += 1;
	return h;
}

// Jenkins
unsigned int JenkinsHash(const char *key)
{
	if (!key) {
		return 0;
	}
	unsigned int h, i;
	size_t len = strlen(key);
	for (h = i = 0; i < len; ++i) {
		h += key[i];
		h += (h << 10);
		h ^= (h >> 6);
	}
	h += (h << 3);
	h ^= (h >> 11);
	h += (h << 15);
	flag[h & 524287] += 1;
	return h;
}

// ElfHash(Î´ÓÃ)
unsigned int ElfHash(const char *key)
{
	unsigned int h = 0;
	unsigned int x = 0;
	while (*key)
	{
		h = (h << 4) + *key;     
		if ((x = h & 0xf0000000) != 0)
		{
			h ^= (x >> 24);
			h &= ~x;
		}
		key++;
	}
	return (h & 0x7fffffff); 
}
