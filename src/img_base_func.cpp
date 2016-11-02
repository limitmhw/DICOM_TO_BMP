#include"img_base.h"
/*函数功能：位图文件头显示
输入参数：BITMAPFILEHEADER* pBmpHead 指向位图文件头结构的指针
（即内存地址，实现主调函数实参和被调函数形参的内存共享）
输出值：没有
*/

void showBmpHead(BITMAPFILEHEADER* pBmpHead)
{
	printf("位图文件头:\n");
	printf("文件类型:%x\n", pBmpHead->bfType);
	printf("文件大小:%d\n", pBmpHead->bfSize);
	printf("保留字:%d\n", pBmpHead->bfReserved1);
	printf("保留字:%d\n", pBmpHead->bfReserved2);
	printf("实际位图数据的偏移字节数:%d\n", pBmpHead->bfOffBits);
}

/*函数功能：位图信息头显示
输入参数：BITMAPINFOHEADER* pBmpInforHead 指向位图文件信息头结构的指针
（即内存地址，实现主调函数实参和被调函数形参的内存共享）
输出值：没有
*/
void showBmpInforHead(BITMAPINFOHEADER* pBmpInforHead)
{
	printf("位图信息头:\n");
	printf("结构体的长度:%d\n", pBmpInforHead->biSize);
	printf("位图宽:%d\n", pBmpInforHead->biWidth);
	printf("位图高:%d\n", pBmpInforHead->biHeight);
	printf("biPlanes平面数:%d\n", pBmpInforHead->biPlanes);
	printf("biBitCount采用颜色位数:%d\n", pBmpInforHead->biBitCount);
	printf("压缩方式:%d\n", pBmpInforHead->biCompression);
	printf("biSizeImage实际位图数据占用的字节数:%d\n", pBmpInforHead->biSizeImage);
	printf("X方向分辨率:%d\n", pBmpInforHead->biXPelsPerMeter);
	printf("Y方向分辨率:%d\n", pBmpInforHead->biYPelsPerMeter);
	printf("使用的颜色数:%d\n", pBmpInforHead->biClrUsed);
	printf("重要颜色数:%d\n", pBmpInforHead->biClrImportant);
}

/*函数功能：颜色表（索引模式）或位图数据区各像素RGB值（真彩模式）显示
输入参数：RGBQUAD* pRGB --- RGBAlpha 四元素结构体指针变量
DWORD num --- 要显示的RGBAlpha 四元素结构体个数，
即颜色表长度或要显示的像素个数
输出值：  没有
*/
void showRgbQuan(RGBQUAD* pRGB, DWORD num)
{
	for (DWORD i = 0; i < num; i++)
	{
		if (i % 5 == 0)
		{
			printf("\n");
		}
		printf("(%-3d,%-3d,%-3d)   ", (pRGB + i)->rgbRed, (pRGB + i)->rgbGreen, (pRGB + i)->rgbBlue);
	}
	printf("\n");

}

/*函数功能：图像反色
输入参数：RGBQUAD* dataOfBmp_src --- 原图像所有像素（以行为序）对应的
RGBAlpha 四元素结构体指针；
RGBQUAD* dataOfBmp_dst --- 反色后的图像所有像素（以行为序）对应的
RGBAlpha 四元素结构体指针；
DWORD width, DWORD height --- 原图像和输出图像的宽度和高度
（以像素为单位）
输出值：  没有
*/
void reverseColor(RGBQUAD** dataOfBmp_src, RGBQUAD** dataOfBmp_dst, DWORD width, DWORD height)
{
	for (DWORD i = 0; i < height; i++)
	for (DWORD j = 0; j < width; j++)
	{
		dataOfBmp_dst[i][j].rgbRed = 255 - dataOfBmp_src[i][j].rgbRed;
		dataOfBmp_dst[i][j].rgbGreen = 255 - dataOfBmp_src[i][j].rgbGreen;
		dataOfBmp_dst[i][j].rgbBlue = 255 - dataOfBmp_src[i][j].rgbBlue;
	}
}

/*函数功能：彩色图像转化为灰度图象
输入参数：RGBQUAD** dataOfBmp_src --- 原图像所有像素（以行为序）对应的
RGBAlpha 四元素结构体指针；
RGBQUAD** dataOfBmp_dst --- 转化为灰度图象后的图像所有像素（以行为序）对应的RGBAlpha 四元素结构体指针；
BYTE** dataOfBmp_gray --- 转化为灰度图象后的图像所有像素（以行为序）对应的灰度值；
DWORD width, DWORD height --- 原图像和输出图像的宽度和高度
（以像素为单位）
输出值：  没有
*/
void RGB2Gray(RGBQUAD** dataOfBmp_src, RGBQUAD** dataOfBmp_dst, BYTE** dataOfBmp_gray, DWORD width, DWORD height)
{
	double gray;

	for (DWORD i = 0; i < height; i++)
	{
		for (DWORD j = 0; j < width; j++)
		{
			gray = 0.299*dataOfBmp_src[i][j].rgbRed + 0.587*dataOfBmp_src[i][j].rgbGreen + 0.114*dataOfBmp_src[i][j].rgbBlue;
			dataOfBmp_gray[i][j] = (BYTE)gray;
			dataOfBmp_dst[i][j].rgbRed = (BYTE)gray;
			dataOfBmp_dst[i][j].rgbGreen = (BYTE)gray;
			dataOfBmp_dst[i][j].rgbBlue = (BYTE)gray;
		}
	}
}

/*函数功能：灰度图象四近邻（flag=0）或八近邻（flag=1）对比度
输入参数：BYTE* dataOfBmp_gray --- 灰度图像所有像素（以行为序）对应的灰度值；
DWORD width, DWORD height --- 原图像和输出图像的宽度和高度
（以像素为单位）
bool flag --- 四近邻或八近邻标志， flag=0为四近邻，flag=1为八近邻
输出值：  四近邻（flag=0）或八近邻（flag=1）对比度
*/
double contrast(BYTE** dataOfBmp_gray, DWORD width, DWORD height, bool flag)
{
	DWORD i, j;
	double contrast_sum = 0;
	int tmp0 = 2, tmp1 = 3, tmp2 = 4;
	int num = 0;

	if (flag)
	{
		tmp0 = 3;
		tmp1 = 5;
		tmp2 = 8;
	}
	num = 4 * tmp0 + ((width - 2) + (height - 2))*tmp1 + ((width - 2)*(height - 2))*tmp2;

	for (i = 0; i<height; i++)
	{
		for (j = 0; j<width; j++)
		{
			if (i>0)
			{
				contrast_sum += pow((dataOfBmp_gray[i - 1][j] - dataOfBmp_gray[i][j]), 2.0);
				if (flag)
				{
					if (j>0)
					{
						contrast_sum += pow((dataOfBmp_gray[i - 1][j - 1] - dataOfBmp_gray[i][j]), 2.0);
					}
					if (j < width - 1)
					{
						contrast_sum += pow((dataOfBmp_gray[i - 1][j + 1] - dataOfBmp_gray[i][j]), 2.0);
					}
				}
			}
			if (i<height - 1)
			{
				contrast_sum += pow((dataOfBmp_gray[i + 1][j] - dataOfBmp_gray[i][j]), 2.0);
				if (flag)
				{
					if (j>0)
					{
						contrast_sum += pow((dataOfBmp_gray[i + 1][j - 1] - dataOfBmp_gray[i][j]), 2.0);
					}
					if (j<width - 1)
					{
						contrast_sum += pow((dataOfBmp_gray[i + 1][j + 1] - dataOfBmp_gray[i][j]), 2.0);
					}
				}
			}

			if (j>0)
			{
				contrast_sum += pow((dataOfBmp_gray[i][j - 1] - dataOfBmp_gray[i][j]), 2.0);
			}

			if (j < width - 1)
			{
				contrast_sum += pow((dataOfBmp_gray[i][j + 1] - dataOfBmp_gray[i][j]), 2.0);
			}

		}
	}

	return contrast_sum / num;
}


/*函数功能：写（32位）位图（即保存）
输入参数：RGBQUAD* dataOfBmp --- 待存储图像所有像素（以行为序）对应的
RGBAlpha 四元素结构体指针；
DWORD width, DWORD height --- 待存储图像的宽度和高度
（以像素为单位）
输出值：  没有
*/
void saveBmp(RGBQUAD** dataOfBmp, DWORD width, DWORD height)
{
	DWORD i, j;
	BITMAPFILEHEADER   bitHead;
	BITMAPINFOHEADER bitInfoHead;
	WORD biBitCount = 32;
	FILE* pfile;
	char strFile[50];
	printf("please input the .bmp destination file name:\n");
	scanf("%s", strFile);
	pfile = fopen(strFile, "wb");//打开文件
	if (pfile != NULL)
	{
		printf("file %s open success.\n", strFile);
		//写位图文件头信息
		bitHead.bfType = 0x4d42;
		bitHead.bfSize = 0;
		bitHead.bfReserved1 = 0;
		bitHead.bfReserved2 = 0;
		bitHead.bfOffBits = 54;
		if (biBitCount <= 8)
			bitHead.bfOffBits += (DWORD)pow(2.0, biBitCount) * 4;
		fwrite(&bitHead, 1, sizeof(tagBITMAPFILEHEADER), pfile);

		bitInfoHead.biSize = 40;
		bitInfoHead.biWidth = width;
		bitInfoHead.biHeight = height;
		bitInfoHead.biPlanes = 1;
		bitInfoHead.biBitCount = biBitCount;
		bitInfoHead.biCompression = 0;
		bitInfoHead.biSizeImage = 0;
		bitInfoHead.biXPelsPerMeter = 0;
		bitInfoHead.biYPelsPerMeter = 0;
		bitInfoHead.biClrImportant = 0;
		bitInfoHead.biClrUsed = 0;
		fwrite(&bitInfoHead, 1, sizeof(tagBITMAPINFOHEADER), pfile);

		if (biBitCount <= 8)
		{
			BYTE tmp = 0;

			for (i = 0; i < (DWORD)pow(2.0, biBitCount); i++)
			{
				tmp = (BYTE)i;
				fwrite(&tmp, 1, 4, pfile);
			}
		}

		int l_width = WIDTHBYTES(width * biBitCount) - width * 4;//计算为确保位图数据区的实际宽度为32字节的整数倍需添加的0字节个数
		for (i = 0; i < height; i++)
		{
			for (j = 0; j < width; j += 1)
			{
				fwrite(&dataOfBmp[height - i - 1][j], 1, 4, pfile);

			}

			BYTE tmp = 0;
			for (j = 0; j < l_width; j++)
				fwrite(&tmp, 1, 1, pfile);

		}
	}

	fclose(pfile);

}
