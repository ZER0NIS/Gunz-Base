// MMD5.h
#ifndef _MMD5_H_
#define _MMD5_H_

typedef struct md5
{
	unsigned long total[2];     /*!< number of bytes processed  */
	unsigned long state[4];     /*!< intermediate digest state  */
	unsigned char buffer[64];   /*!< data block being processed */
} md5_context;

class MMD5
{
public:
	MMD5();
	virtual ~MMD5();
	void md5_string(unsigned char *input, int ilen, unsigned char output[16]);
	int md5_file(char *filePath, unsigned char output[16]);

	// wrapper
	void md5_string(unsigned char *input, int ilen, std::string &strOutput);
	int md5_file(char *filePath, std::string &strOutput);
	static std::string ToString(unsigned char *input);

//private:
	void md5_starts(md5_context *ctx);
	void md5_update(md5_context *ctx, unsigned char *input, int ilen);
	void md5_finish(md5_context *ctx, unsigned char output[16]);

	// wrapper
	std::string convToString(unsigned char *bytes);
};

#endif