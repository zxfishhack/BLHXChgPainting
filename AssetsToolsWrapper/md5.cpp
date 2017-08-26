#include "md5.h"
#include <string>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <time.h>
#include <windows.h>
using namespace std;

#define BUFF_SIZE (6400)

#define S11 7
#define S12 12
#define S13 17
#define S14 22
#define S21 5
#define S22 9
#define S23 14
#define S24 20
#define S31 4
#define S32 11
#define S33 16
#define S34 23
#define S41 6
#define S42 10
#define S43 15
#define S44 21

const unsigned char tran[17] = "0123456789ABCDEF";

/* F, G, H and I are basic MD5 functions.
 */
#define F(x, y, z) (((x) & (y)) | ((~x) & (z)))
#define G(x, y, z) (((x) & (z)) | ((y) & (~z)))
#define H(x, y, z) ((x) ^ (y) ^ (z))
#define I(x, y, z) ((y) ^ ((x) | (~z)))

/* ROTATE_LEFT rotates x left n bits.
 */
#define ROTATE_LEFT(x, n) (((x) << (n)) | ((x) >> (32-(n))))

/* FF, GG, HH, and II transformations for rounds 1, 2, 3, and 4.
Rotation is separate from addition to prevent recomputation.
 */
#define FF(a, b, c, d, x, s, ac) { \
 (a) += F ((b), (c), (d)) + (x) + (UINT4)(ac); \
 (a) = ROTATE_LEFT ((a), (s)); \
 (a) += (b); \
  }
#define GG(a, b, c, d, x, s, ac) { \
 (a) += G ((b), (c), (d)) + (x) + (UINT4)(ac); \
 (a) = ROTATE_LEFT ((a), (s)); \
 (a) += (b); \
  }
#define HH(a, b, c, d, x, s, ac) { \
 (a) += H ((b), (c), (d)) + (x) + (UINT4)(ac); \
 (a) = ROTATE_LEFT ((a), (s)); \
 (a) += (b); \
  }
#define II(a, b, c, d, x, s, ac) { \
 (a) += I ((b), (c), (d)) + (x) + (UINT4)(ac); \
 (a) = ROTATE_LEFT ((a), (s)); \
 (a) += (b); \
  }

const unsigned char MD5::PADDING[64]={0x80};

MD5::MD5(){
	init();
}

MD5::~MD5(){

}

bool MD5::setMessage(const char * lhs){
	string tem(lhs);
	data = tem;
	init();
	return true;
}

bool MD5::setMessage(const char *lhs, size_t length) {
	data.assign(lhs, length);
	return true;
}

bool MD5::setMessage(const std::string &lhs){
	data = lhs;
	init();
	return true;
}

MD5::MD5(const char *lhs){
	string tem(lhs);
	data = tem;
	init();
}

MD5::MD5(const std::string &lhs){
	data = lhs;
	init();
}

string MD5::Padding(){
	//padding
	string tem;
	tem = data;
	//return tem;
	unsigned int PaddingLen;
	union {
		struct {UINT4 MessageLen,_;};
		unsigned char messageLen[9];
	}message;
	message._ = 0;
	message.MessageLen = tem.length() + 8;
	PaddingLen = message.MessageLen & 0x3f;
	PaddingLen = 64 - PaddingLen;
	message.MessageLen -= 8;
	message.MessageLen *= 8;
	tem.append((const char *)PADDING,PaddingLen);
	tem.append((const char *)message.messageLen,8);
	return tem;
}

void MD5::_getResult(const char * tem,int len){
	unsigned int p;
	p = 0;
	UINT4 a,b,c,d;
	while(p < len){
		a=state[0];
		b=state[1];
		c=state[2];
		d=state[3];
		//第一轮
		memcpy(BLOCK.block,tem + p,64);
		FF (a, b, c, d, BLOCK.M[ 0], S11, 0xd76aa478); /* 1 */
		FF (d, a, b, c, BLOCK.M[ 1], S12, 0xe8c7b756); /* 2 */
		FF (c, d, a, b, BLOCK.M[ 2], S13, 0x242070db); /* 3 */
		FF (b, c, d, a, BLOCK.M[ 3], S14, 0xc1bdceee); /* 4 */
		FF (a, b, c, d, BLOCK.M[ 4], S11, 0xf57c0faf); /* 5 */
		FF (d, a, b, c, BLOCK.M[ 5], S12, 0x4787c62a); /* 6 */
		FF (c, d, a, b, BLOCK.M[ 6], S13, 0xa8304613); /* 7 */
		FF (b, c, d, a, BLOCK.M[ 7], S14, 0xfd469501); /* 8 */
		FF (a, b, c, d, BLOCK.M[ 8], S11, 0x698098d8); /* 9 */
		FF (d, a, b, c, BLOCK.M[ 9], S12, 0x8b44f7af); /* 10 */
		FF (c, d, a, b, BLOCK.M[10], S13, 0xffff5bb1); /* 11 */
		FF (b, c, d, a, BLOCK.M[11], S14, 0x895cd7be); /* 12 */
		FF (a, b, c, d, BLOCK.M[12], S11, 0x6b901122); /* 13 */
		FF (d, a, b, c, BLOCK.M[13], S12, 0xfd987193); /* 14 */
		FF (c, d, a, b, BLOCK.M[14], S13, 0xa679438e); /* 15 */
		FF (b, c, d, a, BLOCK.M[15], S14, 0x49b40821); /* 16 */
		//第二轮
		GG (a, b, c, d, BLOCK.M[ 1], S21, 0xf61e2562); /* 17 */
		GG (d, a, b, c, BLOCK.M[ 6], S22, 0xc040b340); /* 18 */
		GG (c, d, a, b, BLOCK.M[11], S23, 0x265e5a51); /* 19 */
		GG (b, c, d, a, BLOCK.M[ 0], S24, 0xe9b6c7aa); /* 20 */
		GG (a, b, c, d, BLOCK.M[ 5], S21, 0xd62f105d); /* 21 */
		GG (d, a, b, c, BLOCK.M[10], S22, 0x02441453); /* 22 */
		GG (c, d, a, b, BLOCK.M[15], S23, 0xd8a1e681); /* 23 */
		GG (b, c, d, a, BLOCK.M[ 4], S24, 0xe7d3fbc8); /* 24 */
		GG (a, b, c, d, BLOCK.M[ 9], S21, 0x21e1cde6); /* 25 */
		GG (d, a, b, c, BLOCK.M[14], S22, 0xc33707d6); /* 26 */
		GG (c, d, a, b, BLOCK.M[ 3], S23, 0xf4d50d87); /* 27 */
		GG (b, c, d, a, BLOCK.M[ 8], S24, 0x455a14ed); /* 28 */
		GG (a, b, c, d, BLOCK.M[13], S21, 0xa9e3e905); /* 29 */
		GG (d, a, b, c, BLOCK.M[ 2], S22, 0xfcefa3f8); /* 30 */
		GG (c, d, a, b, BLOCK.M[ 7], S23, 0x676f02d9); /* 31 */
		GG (b, c, d, a, BLOCK.M[12], S24, 0x8d2a4c8a); /* 32 */
		//第三轮
		HH (a, b, c, d, BLOCK.M[ 5], S31, 0xfffa3942); /* 33 */
		HH (d, a, b, c, BLOCK.M[ 8], S32, 0x8771f681); /* 34 */
		HH (c, d, a, b, BLOCK.M[11], S33, 0x6d9d6122); /* 35 */
		HH (b, c, d, a, BLOCK.M[14], S34, 0xfde5380c); /* 36 */
		HH (a, b, c, d, BLOCK.M[ 1], S31, 0xa4beea44); /* 37 */
		HH (d, a, b, c, BLOCK.M[ 4], S32, 0x4bdecfa9); /* 38 */
		HH (c, d, a, b, BLOCK.M[ 7], S33, 0xf6bb4b60); /* 39 */
		HH (b, c, d, a, BLOCK.M[10], S34, 0xbebfbc70); /* 40 */
		HH (a, b, c, d, BLOCK.M[13], S31, 0x289b7ec6); /* 41 */
		HH (d, a, b, c, BLOCK.M[ 0], S32, 0xeaa127fa); /* 42 */
		HH (c, d, a, b, BLOCK.M[ 3], S33, 0xd4ef3085); /* 43 */
		HH (b, c, d, a, BLOCK.M[ 6], S34,  0x4881d05); /* 44 */
		HH (a, b, c, d, BLOCK.M[ 9], S31, 0xd9d4d039); /* 45 */
		HH (d, a, b, c, BLOCK.M[12], S32, 0xe6db99e5); /* 46 */
		HH (c, d, a, b, BLOCK.M[15], S33, 0x1fa27cf8); /* 47 */
		HH (b, c, d, a, BLOCK.M[ 2], S34, 0xc4ac5665); /* 48 */
		//第四轮
		II (a, b, c, d, BLOCK.M[ 0], S41, 0xf4292244); /* 49 */
		II (d, a, b, c, BLOCK.M[ 7], S42, 0x432aff97); /* 50 */
		II (c, d, a, b, BLOCK.M[14], S43, 0xab9423a7); /* 51 */
		II (b, c, d, a, BLOCK.M[ 5], S44, 0xfc93a039); /* 52 */
		II (a, b, c, d, BLOCK.M[12], S41, 0x655b59c3); /* 53 */
		II (d, a, b, c, BLOCK.M[ 3], S42, 0x8f0ccc92); /* 54 */
		II (c, d, a, b, BLOCK.M[10], S43, 0xffeff47d); /* 55 */
		II (b, c, d, a, BLOCK.M[ 1], S44, 0x85845dd1); /* 56 */
		II (a, b, c, d, BLOCK.M[ 8], S41, 0x6fa87e4f); /* 57 */
		II (d, a, b, c, BLOCK.M[15], S42, 0xfe2ce6e0); /* 58 */
		II (c, d, a, b, BLOCK.M[ 6], S43, 0xa3014314); /* 59 */
		II (b, c, d, a, BLOCK.M[13], S44, 0x4e0811a1); /* 60 */
		II (a, b, c, d, BLOCK.M[ 4], S41, 0xf7537e82); /* 61 */
		II (d, a, b, c, BLOCK.M[11], S42, 0xbd3af235); /* 62 */
		II (c, d, a, b, BLOCK.M[ 2], S43, 0x2ad7d2bb); /* 63 */
		II (b, c, d, a, BLOCK.M[ 9], S44, 0xeb86d391); /* 64 */
		state[0] += a;
		state[1] += b;
		state[2] += c;
		state[3] += d;
		p+=64;
	}
	//sprintf(md5hash,"%x%x%x%x",state[0],state[1],state[2],state[3]);

	//MD5HASH = md5hash;
	unsigned char * result;
	result = (unsigned char *)state;
	MD5HASH="";
	for(int i = 0;i<16;i++,result++){
		MD5HASH.append(1,tran[*result >> 4]);
		MD5HASH.append(1,tran[*result % 16]);
	}
}

const string & MD5::getResult(){
	string tem;
	tem = Padding();
	_getResult(tem.c_str(),tem.length());
	return MD5HASH;
}

const string & File_MD5::getResult(){
	ifstream in;
	string tem;
	unsigned long filesize;
	clock_t s,e;
	int count=0;
	char buff[BUFF_SIZE]={0};
	char * _buff;
	float percent=0,bpercent=0;
	HANDLE hFile = CreateFile(filename.c_str(),
		GENERIC_READ,
		FILE_SHARE_READ, 
		NULL,
		OPEN_EXISTING, 
		FILE_FLAG_RANDOM_ACCESS,
		NULL);
	DWORD dwFileSizeHigh;
	__int64 qwFileSize = GetFileSize(hFile, &dwFileSizeHigh);
	qwFileSize |= (((__int64)dwFileSizeHigh) << 32);
	if(_showinfo){
		cout<<setprecision(2)<<fixed;
		cout<<"The file size is:"<<qwFileSize / 1024.0<<"KB"<<endl;
	}
	if(1){
		//in.close();
		//对所有文件使用内存映射文件进行输入输出处理
		HANDLE hFileMapping = CreateFileMapping(hFile,NULL,PAGE_READONLY, 0, 0, NULL);
		CloseHandle(hFile);
		SYSTEM_INFO SysInfo;
		GetSystemInfo(&SysInfo);
		// 设定大小、偏移量等参数
		DWORD dwGran = SysInfo.dwAllocationGranularity;
		__int64 qwFileOffset = 0;
		__int64 T_newmap = 900 * dwGran;
		DWORD dwBlockBytes = 1000 * dwGran;
		static int GRAN_PER_BLOCK = 100;
		_buff = new char[GRAN_PER_BLOCK * dwGran];
		if (qwFileSize - qwFileOffset < dwBlockBytes)
			dwBlockBytes = (DWORD)qwFileSize;
		char * pbFile2;
		char * pbFile = (char *)MapViewOfFile(hFileMapping,FILE_MAP_READ,(DWORD)(qwFileOffset>>32), (DWORD)(qwFileOffset&0xFFFFFFFF), dwBlockBytes);
		pbFile2 = pbFile;
		s = clock(); 
		while(qwFileOffset < qwFileSize){
			if(_showinfo){
				percent = float(qwFileOffset);
				if(percent > qwFileSize) percent = 100;
				else percent = percent / qwFileSize * 100;
				if( percent - bpercent > 0.05){
					bpercent = percent;
					cout<<qwFileOffset / 1024.0<<"KB Readed,"<<setw(6)<<percent<<"% completed.\r";
				}
			}
			count ++;

			if (qwFileOffset > T_newmap){
				UnmapViewOfFile(pbFile);
				if (qwFileSize - T_newmap < dwBlockBytes)
					dwBlockBytes = (DWORD)(qwFileSize - T_newmap);
				pbFile = (char *)MapViewOfFile(hFileMapping,FILE_MAP_READ,
				(DWORD)(T_newmap >> 32), (DWORD)(T_newmap & 0xFFFFFFFF),dwBlockBytes);
				// 修正参数
				pbFile2 = pbFile + qwFileOffset - T_newmap;
				T_newmap =T_newmap + 900 * dwGran;
			}
			if(qwFileSize - qwFileOffset > GRAN_PER_BLOCK * dwGran){
				//data="";
				//data.append(pbFile, GRAN_PER_BLOCK * dwGran);
				memcpy(_buff,pbFile2,GRAN_PER_BLOCK * dwGran);
				pbFile2 += GRAN_PER_BLOCK * dwGran;
				qwFileOffset += GRAN_PER_BLOCK * dwGran;
				//_getResult(data.c_str(),GRAN_PER_BLOCK * dwGran);
				_getResult(_buff,GRAN_PER_BLOCK * dwGran);
			}
			else{
				data="";
				data.append(pbFile2,int(qwFileSize - qwFileOffset));
				memcpy(_buff,pbFile2,int(qwFileSize - qwFileOffset));
				//_Padding(_buff,int(qwFileSize - qwFileOffset),qwFileSize);
				data = Padding(qwFileSize);
				_getResult(data.c_str(),data.length());
				//_getResult(_buff,(int(qwFileSize - qwFileOffset) / 64 + 1) * 64);
				qwFileOffset += qwFileSize - qwFileOffset;
			}
		}
		e = clock();
		UnmapViewOfFile(pbFile);
		CloseHandle(hFileMapping); 
		if(_showinfo) cout<<qwFileOffset / 1024.0<<"KB Readed,100.00% completed."<<endl;
		cout<< (double)(e - s) / CLOCKS_PER_SEC<<"s elapsed."<<endl;
		delete _buff;
		return MD5HASH;
	}
	/*
	s = clock();
	while(!in.eof()){
		if(_showinfo){
			percent = float(count) * BUFF_SIZE;
			if(percent >filesize) percent = 100;
			else percent = percent / filesize * 100;
			if( percent - bpercent > 0.05){
				bpercent = percent;
				cout<<in.tellg() / 1024.0<<"KB Readed,"<<setw(6)<<percent<<"% completed.\r";
			}
		}
		count ++;
		in.read(buff,BUFF_SIZE);
		data="";
		data.append(buff,in.gcount());
		if(in.eof())
			data = Padding((count - 1) * BUFF_SIZE + in.gcount());
		_getResult(data.c_str(),data.length());
	}
	e = clock();
	if(_showinfo) cout<<filesize / 1024.0<<"KB Readed,"<<"100.00% completed."<<endl;
	cout<< (double)(e - s) / CLOCKS_PER_SEC<<"s elapsed."<<endl;
	return MD5HASH;
	*/
}

File_MD5::~File_MD5(){
}


string File_MD5::Padding(__int64 filelen){
	//padding
	string tem;
	tem = data;
	unsigned int PaddingLen;
	union {
		struct {__int64 MessageLen;};
		unsigned char messageLen[8];
	}message;
	message.MessageLen = filelen + 8;
	PaddingLen = message.MessageLen & 0x3f;
	PaddingLen = 64 - PaddingLen;
	message.MessageLen -= 8;
	message.MessageLen *= 8;
	tem.append((const char *)PADDING,PaddingLen);
	tem.append((const char *)message.messageLen,8);
	return tem;
}


void File_MD5::_Padding(char * buff,int sp, __int64 filelen){
	//padding
	//string tem;
	//tem = data;
	unsigned int PaddingLen;
	union {
		struct {__int64 MessageLen;};
		unsigned char messageLen[8];
	}message;
	message.MessageLen = filelen + 8;
	PaddingLen = message.MessageLen & 0x3f;
	PaddingLen = 64 - PaddingLen;
	message.MessageLen -= 8;
	message.MessageLen *= 8;
	//tem.append((const char *)PADDING,PaddingLen);
	memcpy(buff + sp,PADDING,PaddingLen);
	//tem.append((const char *)message.messageLen,8);
	memcpy(buff + sp + PaddingLen , message.messageLen,8);
}