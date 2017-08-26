#include <string>
using namespace std;

#ifndef MD5_CLASS
#define MD5_CLASS

#define UINT4 unsigned int

class MD5{
public:
	MD5();
	MD5(const char *lhs);
	MD5(const string & lhs);
	~MD5();
	virtual const string & getResult();
	virtual bool setMessage(const char *lhs);
	virtual bool setMessage(const char* lhs, size_t length);
	virtual bool setMessage(const string & lhs);
	virtual const string & getData(){ return data;};
protected:
	string data;
	string Padding();
	inline virtual void _getResult(const char * tem,int len);
	const static unsigned char PADDING[64];
	union {
		char block[64];
		UINT4 M[16];
	}BLOCK;
	string MD5HASH;
	void init(){
		state[0]=0x67452301;
		state[1]=0xEFCDAB89;
		state[2]=0x98BADCFE;
		state[3]=0x10325476;
	}
	UINT4 state[4];
	UINT4 P1(UINT4 b,UINT4 c,UINT4 d){return (b & c) | (~b) & d;}
	UINT4 P2(UINT4 b,UINT4 c,UINT4 d){return (b & d) | (c & (~d));}
	UINT4 P3(UINT4 b,UINT4 c,UINT4 d){return (b ^ c ^ d);}
	UINT4 P4(UINT4 b,UINT4 c,UINT4 d){return c ^ (b | (~d));}

};

class File_MD5 : public MD5{
public:
	File_MD5(bool showinfo = false){filename ="";_showinfo = showinfo;};
	File_MD5(const char * lhs,bool showinfo = false){filename = lhs;_showinfo = showinfo;};
	File_MD5(const string & lhs,bool showinfo = false){filename = lhs;_showinfo = showinfo;};
	~File_MD5();
	void setFileName(const char * lhs){filename = lhs;};
	void setFileName(const string & lhs){filename = lhs;};
	bool setMessage(const char * lhs){return false;};
	bool setMessage(const string & lhs){return false;};
	const string & getResult();
	const string & getData(){ return filename;};
protected:
	string filename;
	bool _showinfo;
	void _Padding(char * buff,int sp, __int64 filelen);
	string Padding(__int64 filelen);
};

#endif


/*
//for backup

	const static int s[4][4];
	const static UINT4 t[64];
	const static char MC[4][16];
const int MD5::s[4][4]={ 7,12,17,22,
						 5, 9,14,20,
						 4,11,16,23,
						 6,10,15,21};

const UINT4 MD5::t[64]={0xD76AA478,0xE8C7B756,0x242070DB,0xC1BDCEEE,0xF57C0FAF,0x4787C62A,0xA8304613,0xFD469501,
				     	0x698098D8,0x8B44F7AF,0xFFFF5BB1,0x895CD7BE,0x6B901122,0xFD987193,0xA679438E,0x49B40821,
						0xF61E2562,0xC040B340,0x265E5A51,0xE9B6C7AA,0xD62F105D,0x02441453,0xD8A1E681,0xE7D3FBC8,
						0x21E1CDE6,0xC33707D6,0xF4D50D87,0x455A14ED,0xA9E3E905,0xFCEFA3F8,0x676F02D9,0x8D2A4C8A,
						0xFFFA3942,0x8771F681,0x699D6122,0xFDE5380C,0xA4BEEA44,0x4BDECFA9,0xF6BB4B60,0xBEBFBC70,
						0x289B7EC6,0xEAA127FA,0xD4EF3085,0x04881D05,0xD9D4D039,0xE6DB99E5,0x1FA27CF8,0xC4AC5665,
						0xF4292244,0x432AFF97,0xAB9423A7,0xFC93A039,0x655B59C3,0x8F0CCC92,0xFFEFF47D,0x85845DD1,
						0x6FA87E4F,0xFE2CE6E0,0xA3014314,0x4E0811A1,0xF7537E82,0xBD3AF235,0x2AD7D2BB,0xBE86D391};

const char MD5::MC[4][16]={ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,
						    1, 6,11, 0, 5,10,15, 4, 9,14, 3, 8,13, 2, 7,12,
						    5, 8,11,14, 1, 4, 7,10,13, 0, 3, 6, 9,12,15, 2,
						    0, 7,14, 5,12, 3,10, 1, 8,15, 6,13, 4,11, 2, 9};



*/
