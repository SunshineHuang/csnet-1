#ifndef __MY_BUFFER_H__
#define __MY_BUFFER_H__


#define BUF_LEN_1 2048
#define BUF_LEN_2 4096
#define BUF_LEN_3 8192
#define BUF_LEN_4 16384
#define BUF_LEN_5 32768
#define BUF_LEN_6 65536
#define BUF_LEN_7 131072
#define BUF_LEN_8 262144	
#define BUF_LEN_9 524288
#define BUF_LEN_10 10485760

class CMyBuffer
{
public:
	CMyBuffer(int size = BUF_LEN_3);
	~CMyBuffer();

	int append(const char* data, int len);
	int clear();
	int rebuild(int len);
	char* data();
	int  length();

private:
	int reAllocLength(int len);

private:
	char*	m_buf;
	int 	m_size;
	int 	m_len;
};

#endif

