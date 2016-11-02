#include<iostream>

using namespace std;
typedef unsigned long       DWORD;
#define DICOM_HEADER_MAKER   ((DWORD) ('M' << 24) |('C' << 16) |('I' <<8) | 'D')
enum COMPRESSION_MODE
{
	COMPRESS_NONE = 0,
	COMPRESS_RLE,
	COMPRESS_JPEGLOSSY,
	COMPRESS_JPEGLOSSY12BIT,
	COMPRESS_JPEGLOSSLESS,
	COMPRESS_JPEGLOSSLESS2
};
enum CONVERSION_MODE
{
	MODE_DICOM2BMP = 0,
	MODE_BMP2DICOM
};

enum DATA_ENDIAN
{
	LITTLE_ENDIAN_CC,
    BIG_ENDIAN_CC
};

void SwapWord(char *pArray, int nWords);
void SwapDWord(char *pArray, int nDWords);
int ReadUS(FILE *fp, DATA_ENDIAN nDataEndian);
int ReadIS(FILE *fp, bool bImplicitVR, DATA_ENDIAN nDataEndian);
float ReadDS(FILE *fp, bool bImplicitVR, DATA_ENDIAN nDataEndian);
char * ConvertTo8Bit(char *pData, long int nNumPixels, bool bIsSigned, short nHighBit, float fRescaleSlope, float fRescaleIntercept, float fWindowCenter, float fWindowWidth);
void RemoveTailingSpace(char *pszStr);
int ReadString(FILE *fp, char *pszStr, bool bImplicitVR, DATA_ENDIAN nDataEndian);
long int ReadLength(FILE *fp, bool bImplicitVR, DATA_ENDIAN nDataEndian);
void ConvertDicomToBMP(const char*pszFileName);
int	WriteBMPFile(char *pData, int nFrameSize, short nCols, short nRows, int nBytesP, char *pszPhotometric, int nFrameNum);
