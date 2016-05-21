#ifndef __PACKAGE_H__
#define __PACKAGE_H__

#include <string>

#define MAX_LEN 32 * 1024

using namespace std;

class CPackage
{
public:
	CPackage();
	CPackage(char *buf, int len);
	
	void putchar(char c);
	void putshort(short s);
	void putint(int i);
	void putuint(unsigned int u);
	void putstr(string str);
	void putstr(const char *str);
	void putstr(const char* str, int len);
	void putfloat(float f);
	void putlong(long long l);
	
	//�õ��ַ���
	char *str();

	//�ַ�������
	int 	 length();
private:
	char	m_data[MAX_LEN];
	int		m_len;
	int		m_maxLen;
};


class CUnpackage
{
public:
	CUnpackage(const char *str, int len);
	char GetChar();
	short GetShort();
	int  GetInt();
	unsigned int GetUint();
	long long GetLong();
	string GetStr();
	float GetFloat();
private:
	const char*	m_data;	//���ݿ�
	int			m_pos; 	//��ʼȡ����λ��
	int			m_len;	//��ǰ��ʣ�೤��
};


#endif //

