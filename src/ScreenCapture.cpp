#include "stdafx.h"
#include <afxwin.h>
#include <string>
#include <sstream>
#include "ScreenCapture.h"
#include <iostream>
#include <zmq.h>
#include <zhelpers.h>

#define pptPort "5558"
const int pptSize =	612000;
void *picpubisher;
using namespace std;
using namespace cv;
int picnum = 0;


void init_SendPpt(){
	void *context = zmq_init(1);
	picpubisher = zmq_socket(context, ZMQ_PUB);
	cv::string port(pptPort);
	int rc = zmq_bind(picpubisher, ("tcp://*:" + port).c_str());
}
void Screen(string filename)
{
	CDC *pDC;//屏幕DC
	pDC = CDC::FromHandle(GetDC(NULL));//获取当前整个屏幕DC
	int BitPerPixel = pDC->GetDeviceCaps(BITSPIXEL);//获得颜色模式
	int Width = pDC->GetDeviceCaps(HORZRES);
	int Height = pDC->GetDeviceCaps(VERTRES);

	cout << BitPerPixel << ":" << Width << ":" << Height << endl;
	CDC memDC;//内存DC
	memDC.CreateCompatibleDC(pDC);

	CBitmap memBitmap, *oldmemBitmap;//建立和屏幕兼容的bitmap
	memBitmap.CreateCompatibleBitmap(pDC, Width, Height);

	oldmemBitmap = memDC.SelectObject(&memBitmap);//将memBitmap选入内存DC
	memDC.BitBlt(0, 0, Width, Height, pDC, 0, 0, SRCCOPY);//复制屏幕图像到内存DC

	//以下代码保存memDC中的位图到文件
	BITMAP bmp;
	memBitmap.GetBitmap(&bmp);//获得位图信息
	stringstream picindex;
	picindex << picnum;
	filename = filename+"/"+picindex.str()+".jpg";
	FILE *fp = fopen(filename.c_str(), "w+b");

	BITMAPINFOHEADER bih = { 0 };//位图信息头
	bih.biBitCount = bmp.bmBitsPixel;//每个像素字节大小
	bih.biCompression = BI_RGB;
	bih.biHeight = bmp.bmHeight;//高度
	bih.biPlanes = 1;
	bih.biSize = sizeof(BITMAPINFOHEADER);
	bih.biSizeImage = bmp.bmWidthBytes * bmp.bmHeight;//图像数据大小
	bih.biWidth = bmp.bmWidth;//宽度

	BITMAPFILEHEADER bfh = { 0 };//位图文件头
	bfh.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);//到位图数据的偏移量
	bfh.bfSize = bfh.bfOffBits + bmp.bmWidthBytes * bmp.bmHeight;//文件总的大小
	bfh.bfType = (WORD)0x4d42;

	fwrite(&bfh, 1, sizeof(BITMAPFILEHEADER), fp);//写入位图文件头

	fwrite(&bih, 1, sizeof(BITMAPINFOHEADER), fp);//写入位图信息头

	byte * p = new byte[bmp.bmWidthBytes * bmp.bmHeight];//申请内存保存位图数据

	GetDIBits(memDC.m_hDC, (HBITMAP)memBitmap.m_hObject, 0, Height, p,
		(LPBITMAPINFO)&bih, DIB_RGB_COLORS);//获取位图数据

	fwrite(p, 1, bmp.bmWidthBytes * bmp.bmHeight, fp);//写入位图数据

	delete[] p;

	fclose(fp);

	memDC.SelectObject(oldmemBitmap);
	string path = filename;
	Mat img = imread(path);
	Mat Pic;
	resize(img, Pic, Size(600, 340));
	char *buffer = new char[pptSize];

	for (int c = 0; c < Pic.channels(); c++){
		for (int i = 0; i < Pic.rows; i++){
			for (int j = 0; j < Pic.cols; j++){
				buffer[(Pic.cols * i + j) * 3 + c] = Pic.at<Vec3b>(i, j)[c];
			}
		}
	}
	int len = zmq_send(picpubisher,buffer,pptSize,0);
	cout << len << endl;
	delete[] buffer;
	remove(path.c_str());
	picnum++;
}