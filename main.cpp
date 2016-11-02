
#include"mh.h"
#include"img_base.h"
#include<string>

void RemoveTailingSpace(char *pszStr)//删除字符串最后的空格和tab
{
	char *cc = pszStr + strlen(pszStr) - 1;

	while (((*cc == ' ') || (*cc == '\t')) && (cc != pszStr))
	{
		*cc-- = '\0';
	}
}
int	WriteBMPFile(char *pData, int nFrameSize, short nCols, short nRows, int nBytesP, char *pszPhotometric, int nFrameNum)
{
	DWORD width = nCols;
	DWORD height = nRows;
	int nRowBytes = nCols*nBytesP;//每行的字节数
	RGBQUAD** dataOfBmp_dst = NULL;

	dataOfBmp_dst = new RGBQUAD*[height]; //反色后的图象中每个像素的RGBAlpha
	for (int i = 0; i < height; i++){
		dataOfBmp_dst[i] = new RGBQUAD[width];
	}
	int js = 0; //pData + (nRows - 1)*nRowBytes;
	for (DWORD i = 0; i < height; i++)
	{
		for (DWORD j = 0; j < width; j++)
		{
			BYTE gray;
			dataOfBmp_dst[i][j].rgbRed = (BYTE)*(pData + js);
			dataOfBmp_dst[i][j].rgbGreen = (BYTE)*(pData + js);
			dataOfBmp_dst[i][j].rgbBlue = (BYTE)*(pData + js++);
		}
	}
	saveBmp(dataOfBmp_dst, width, height);

	return 0;
}



void SwapDWord(char *pArray, int nDWords)//交换相邻四bytes
{
	char *cc = pArray, c0;
	int i;

	for (i = 0; i < nDWords; i++)
	{
		c0 = *cc;
		*cc = *(cc + 3);
		*(cc + 3) = c0;

		c0 = *(cc + 2);
		*(cc + 2) = *(cc + 1);
		*(cc + 1) = c0;

		cc += 4;
	}
}

float ReadDS(FILE *fp, bool bImplicitVR, DATA_ENDIAN nDataEndian) //读入float字符串
{
	char szTemp[64] = "";
	float fVal = 0;

	if (ReadString(fp, szTemp, bImplicitVR, nDataEndian) == 0)
		sscanf(szTemp, "%f", &fVal);

	return fVal;
}

int ReadIS(FILE *fp, bool bImplicitVR, DATA_ENDIAN nDataEndian) //读入Integer String字符串
{
	char szTemp[64] = "";
	int nVal = 0;

	if (ReadString(fp, szTemp, bImplicitVR, nDataEndian) == 0)
		sscanf(szTemp, "%d", &nVal);

	return nVal;
}

void SwapWord(char *pArray, int nWords)//交换相邻两bytes
{
	char *cc = pArray, c0;
	int i;

	for (i = 0; i < nWords; i++)
	{
		c0 = *cc;
		*cc = *(cc + 1);
		*(cc + 1) = c0;

		cc += 2;
	}
}

int ReadString(FILE *fp, char *pszStr, bool bImplicitVR, DATA_ENDIAN nDataEndian) //读入字符串
{
	long int nValLength = 0;

	nValLength = ReadLength(fp, bImplicitVR, nDataEndian);

	if ((nValLength > 64) || (nValLength < 0))
		return -1;

	fread(pszStr, 1, nValLength, fp);
	pszStr[nValLength] = '\0';
	RemoveTailingSpace(pszStr);

	return 0;
}


long int ReadLength(FILE *fp, bool bImplicitVR, DATA_ENDIAN nDataEndian)//读入Data Element长度
{
	long int nValLength = 0;//四个字节
	short int nsLength;//两个字节

	if (bImplicitVR)//值长度是一个16或32位（取决于显式VR或隐式VR)无符号整数
	{
		fread(&nValLength, sizeof(long), 1, fp);
        if (nDataEndian == BIG_ENDIAN_CC)
			SwapDWord((char *)&nValLength, 1);
	}
	else
	{
		fseek(fp, 2, SEEK_CUR); // 跳过4字节，SEEK_CUR指文件指针当前位置

		fread(&nsLength, sizeof(short), 1, fp);
		if (nDataEndian == BIG_ENDIAN_CC)
			SwapWord((char *)&nsLength, 1);

		nValLength = nsLength;
	}

	return nValLength;
}


int ReadUS(FILE *fp, DATA_ENDIAN nDataEndian)//读入无符号二进制16长整数
{
	unsigned short nVal;

	fseek(fp, 4, SEEK_CUR); // 跳过4字节
	fread(&nVal, 1, sizeof(short), fp);
	if (nDataEndian == BIG_ENDIAN_CC)
		SwapWord((char *)&nVal, 1);

	return (int)nVal;

}

char * ConvertTo8Bit(char *pData, long int nNumPixels, bool bIsSigned, short nHighBit, float fRescaleSlope, float fRescaleIntercept, float fWindowCenter, float fWindowWidth) //将图像转化为8-bit位图
{
	unsigned char *pNewData = 0;
	long int nCount;
	short *pp;

	if (nHighBit < 15) // 1. Clip the high bits.
	{
		short nMask;
		short nSignBit;

		pp = (short *)pData;
		nCount = nNumPixels;

		if (bIsSigned == 0) // Unsigned integer
		{
			nMask = 0xffff << (nHighBit + 1);

			while (nCount-- > 0)
				*(pp++) &= ~nMask;
		}
		else
		{
			// 1's complement representation
			nSignBit = 1 << nHighBit;
			nMask = 0xffff << (nHighBit + 1);
			while (nCount-- > 0)
			{
				if ((*pp & nSignBit) != 0)
					*(pp++) |= nMask;
				else
					*(pp++) &= ~nMask;
			}
		}
	}

	// 2. Rescale if needed (especially for CT)
	if ((fRescaleSlope != 1.0f) || (fRescaleIntercept != 0.0f))
	{
		float fValue;

		pp = (short *)pData;
		nCount = nNumPixels;

		while (nCount-- > 0)
		{
			fValue = (*pp) * fRescaleSlope + fRescaleIntercept;
			*pp++ = (short)fValue;
		}

	}

	// 3. Window-level or rescale to 8-bit
	if ((fWindowCenter != 0) || (fWindowWidth != 0))
	{
		float fSlope;
		float fShift;
		float fValue;
		unsigned char *np = new unsigned char[nNumPixels + 4];

		pNewData = np;

		// Since we have window level info, we will only map what are
		// within the Window.

		fShift = fWindowCenter - fWindowWidth / 2.0f;
		fSlope = 255.0f / fWindowWidth;

		nCount = nNumPixels;
		pp = (short *)pData;

		while (nCount-- > 0)
		{
			fValue = ((*pp++) - fShift) * fSlope;
			if (fValue < 0)
				fValue = 0;
			else if (fValue > 255)
				fValue = 255;

			*np++ = (unsigned char)fValue;
		}

	}
	else
	{
		// We will map the whole dynamic range.
		float fSlope;
		float fValue;
		int nMin, nMax;
		unsigned char *np = new unsigned char[nNumPixels + 4];

		pNewData = np;

		// First compute the min and max.
		nCount = nNumPixels;
		pp = (short *)pData;
		nMin = nMax = *pp;
		while (nCount-- > 0)
		{
			if (*pp < nMin)
				nMin = *pp;

			if (*pp > nMax)
				nMax = *pp;

			pp++;
		}

		// Calculate the scaling factor.
		if (nMax != nMin)
			fSlope = 255.0f / (nMax - nMin);
		else
			fSlope = 1.0f;

		nCount = nNumPixels;
		pp = (short *)pData;
		while (nCount-- > 0)
		{
			fValue = ((*pp++) - nMin) * fSlope;
			if (fValue < 0)
				fValue = 0;
			else if (fValue > 255)
				fValue = 255;

			*np++ = (unsigned char)fValue;
		}
	}
	return (char *)pNewData;
}


void ConvertDicomToBMP(const char*pszFileName) //将Dicom格式图像转化为BMP格式
{
	short int nCols = 0, nRows = 0;
	short int nBitsAllocated, nSamplesPerPixel = 1;
	short int nHighBit = 0;
	float fWindowWidth = 0, fWindowCenter = 0, fRescaleSlope = 1, fRescaleIntercept = 0;
	bool bIsSigned = false;
	bool bGroup2Done = false, bGroup28Done = false, bPixelDataDone = false;
	int nBytesP = 0;
	int nFrameSize = 0;
	long int nLength;
	char szPhotometric[32] = "", szTemp[32] = "", szTransferSyntaxUID[80] = "";
	bool bImplicitVR = true;
	COMPRESSION_MODE nCompressionMode = COMPRESS_NONE;
    DATA_ENDIAN nDataEndian = LITTLE_ENDIAN_CC;
	int i;
	int nBytes;
	long name;
	FILE* fp;
	char* pData = 0;
	short int gTag, eTag;
	int nNumFrames = 1;
    cout<<pszFileName<<endl;
	fp = fopen(pszFileName, "rb");//以只读的方式打开文件
	if (!fp){
		cout << "read file eror" << endl;
		return;
	}
	fseek(fp, 128, SEEK_SET);//跳过128个字节
	fread(&name, sizeof(long), 1, fp);
	if (name != DICOM_HEADER_MAKER)
	{
		cout << "读取图像格式出错，不支持该类型文件！" << endl;
		return;
	}
	else
	while (fread(&gTag, sizeof(short), 1, fp) == 1)//fread返回成功读取的元素个数，这个循环即遍历整个文件
	{
		if (nDataEndian == BIG_ENDIAN_CC)
			SwapWord((char *)&gTag, 1);

		switch (gTag)
		{
		case 0x0002: // Meta header. 
			if (bGroup2Done)
				break;
			fread(&eTag, sizeof(short), 1, fp);
			// Meta header is always in Little Endian Explicit VR syntax.
			switch (eTag)
			{
			case 0x0010: // 数据元素的唯一符
                if (ReadString(fp, szTransferSyntaxUID, false, LITTLE_ENDIAN_CC) != 0)
					break;

				// Check data endian.
				if (!strcmp(szTransferSyntaxUID, "1.2.840.10008.1.2.2")) // Explicit VR Big Endian 
					nDataEndian = BIG_ENDIAN_CC; // Big Endian
				else
                    nDataEndian = LITTLE_ENDIAN_CC; // Little Endian

				// Check if it is implicit VR or Explicit VR
				if (!strcmp(szTransferSyntaxUID, "1.2.840.10008.1.2")) // Implicit VR Little Endian 
					bImplicitVR = true; // Implicit VR
				else
					bImplicitVR = false; // Explicit VR

				// Parse the encapsulation/compression
				if (!strcmp(szTransferSyntaxUID, "1.2.840.10008.1.2.4.50")) // JPEG lossy
					nCompressionMode = COMPRESS_JPEGLOSSY;
				else if (!strcmp(szTransferSyntaxUID, "1.2.840.10008.1.2.4.51")) // JPEG lossy 12bit
					nCompressionMode = COMPRESS_JPEGLOSSY12BIT;
				else if (!strcmp(szTransferSyntaxUID, "1.2.840.10008.1.2.4.70")) // JPEG lossless first order prediction
					nCompressionMode = COMPRESS_JPEGLOSSLESS;
				else if (!strcmp(szTransferSyntaxUID, "1.2.840.10008.1.2.4.57")) // JPEG lossless process 14
					nCompressionMode = COMPRESS_JPEGLOSSLESS2;
				else if (!strcmp(szTransferSyntaxUID, "1.2.840.10008.1.2.5")) // RLE
					nCompressionMode = COMPRESS_RLE;

				bGroup2Done = true;

				break;
			}
			break;

		case 0x0008: // First non-Meta group
			fread(&eTag, sizeof(short), 1, fp);
			if (nDataEndian == BIG_ENDIAN_CC)
				SwapWord((char *)&eTag, 1);
			if ((eTag == 0x0005) || (eTag == 0x0008))
				bGroup2Done = true;
			break;
		case 0x0028: // Image pixel data info group
			fread(&eTag, sizeof(short), 1, fp);

			if (bGroup28Done)
				break;

			if (nDataEndian == BIG_ENDIAN_CC)
				SwapWord((char *)&eTag, 1);

			switch (eTag)
			{
			case 0x0002: // Samples per Pixel,样品的每个像素
				nSamplesPerPixel = ReadUS(fp, nDataEndian);
				break;

			case 0x0004:  // Photometric Interpolation,调色板
				ReadString(fp, szPhotometric, bImplicitVR, nDataEndian);
				break;

			case 0x0008:  // Number of frames,图像的帧数
				nNumFrames = ReadIS(fp, bImplicitVR, nDataEndian);
				break;

			case 0x0010: // Rows,图像的行数
				nRows = ReadUS(fp, nDataEndian);
				break;

			case 0x0011: // Columns,图像的列数
				nCols = ReadUS(fp, nDataEndian);
				break;

			case 0x0100: // Bits allocated,存储位数
				nBitsAllocated = ReadUS(fp, nDataEndian);
				break;

			case 0x0102: // High bit,最高位数
				nHighBit = ReadUS(fp, nDataEndian);
				break;

			case 0x0103: // Pixel Representation,像素代表
				bIsSigned = ReadUS(fp, nDataEndian);
				break;

			case 0x1050: // Window Center,窗口中心
				fWindowCenter = ReadDS(fp, bImplicitVR, nDataEndian);
				break;

			case 0x1051: // Window Width,窗宽
				fWindowWidth = ReadDS(fp, bImplicitVR, nDataEndian);
				break;

			case 0x1052: // Rescale intercept,重新缩放载矩
				fRescaleIntercept = ReadDS(fp, bImplicitVR, nDataEndian);
				break;

			case 0x1053: // Rescale slope,重新调整坡度
				fRescaleSlope = ReadDS(fp, bImplicitVR, nDataEndian);
				bGroup28Done = true;
				break;

			default:
				// do nothing
				break;
			}
			break;
		case 0x7fe0:
			fread(&eTag, sizeof(short), 1, fp);
			if (bPixelDataDone)
				break;

			if (nDataEndian == BIG_ENDIAN_CC)
				SwapWord((char *)&eTag, 1);

			if (eTag == 0x0010)
			{
				nBytesP = nSamplesPerPixel*nBitsAllocated / 8;//每一像素占的字节数
				nFrameSize = nCols * nRows * nBytesP;//一桢图片像素数据大小
				nLength = nNumFrames * nFrameSize;//总大小

				// Don't try to parse grup 2 and group 28 any more
				bGroup2Done = true;
				bGroup28Done = true;

				// Parse pixel data
				switch (nCompressionMode)
				{
				case COMPRESS_NONE:
					pData = new char[nLength + 16];
					fseek(fp, 4, SEEK_CUR); // Skip 4 bytes (length bytes)
					nBytes = fread(pData, 1, nLength, fp);

					if (nBytes != nLength)
					{
						cout << "Failed to read all pixel data." << endl;
					}
					bPixelDataDone = true;
					break;

				case COMPRESS_RLE:
					cout << "RLE compression not supported at this moment" << endl;
					break;
				case COMPRESS_JPEGLOSSY:
					cout << "JPEG lossy compression not supported at this moment" << endl;
					break;
				case COMPRESS_JPEGLOSSY12BIT:
					cout << "JPEG lossy 12-bit compression not supported at this moment" << endl;
					break;
				case COMPRESS_JPEGLOSSLESS:
				case COMPRESS_JPEGLOSSLESS2:
					cout << "JPEG lossless compression not supported at this moment" << endl;
					break;
				}

			}
			break;
		}

		if (pData)
			break; // We are done.
	}
	fclose(fp);
	if (pData)
	{
		// Need to do byte swap
		if (nDataEndian == BIG_ENDIAN_CC)
		{
			if (nBitsAllocated > 8)
				SwapWord(pData, nLength / 2);
		}
		if (nBitsAllocated > 8)
		{
			// We need to convert it to 8-bit.
			char *pNewData;

			pNewData = ConvertTo8Bit(pData, nLength / 2, bIsSigned, nHighBit,
				fRescaleSlope, fRescaleIntercept, fWindowCenter, fWindowWidth);

			// Use the new 8-bit data. 
			if (pNewData)
			{
				delete[] pData;
				pData = pNewData;
				nBytesP = 1;
				nFrameSize /= 2;

				nLength /= 2;
			}
		}
		cout << "nNumFrames_" << nNumFrames << endl;

		for (i = 0; i < nNumFrames; i++){
			cout << "1_" << pData + nFrameSize*i << endl;
			cout << "2_" << nFrameSize << endl;
			cout << "3_" << nCols << endl;
			cout << "4_" << nRows << endl;
			cout << "5_" << nBytesP << endl;
			cout << "6_" << szPhotometric << endl;
			cout << "7_" << i + 1 << endl << endl << endl << endl;

			WriteBMPFile(pData + nFrameSize*i, nFrameSize, nCols, nRows, nBytesP, szPhotometric, i + 1);

		}


		delete[] pData;

		cout << "change to bmp success" << endl;
	}
}

int main(){
    cout<<"Input pic path:"<<endl;
    string ifile;
    cin>>ifile;
    cout<<"Start_to_bmp:"<<endl;
    ConvertDicomToBMP(ifile.c_str());
	return 0;
}
