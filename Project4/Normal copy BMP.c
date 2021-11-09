#define _CRT_SECURE_NO_WARNINGS
#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
typedef unsigned char uch;


// 檔案結構
#pragma pack(2) //記憶體位置連續
struct BmpFileHeader {
    uint16_t bfTybe;
    uint32_t bfSize;
    uint16_t bfReserved1;
    uint16_t bfReserved2;
    uint32_t bfOffBits;
};
struct BmpInfoHeader {
    uint32_t biSize;
    uint32_t biWidth;
    uint32_t biHeight;
    uint16_t biPlanes; // 1=defeaul, 0=custom
    uint16_t biBitCount;
    uint32_t biCompression;
    uint32_t biSizeImage;
    uint32_t biXPelsPerMeter; // 72dpi=2835, 96dpi=3780
    uint32_t biYPelsPerMeter; // 120dpi=4724, 300dpi=11811
    uint32_t biClrUsed;
    uint32_t biClrImportant;
};
#pragma pack()



void bmpWrite(const char* name, const uch* raw_img,
    uint32_t width, uint32_t height, uint16_t bits) {

    // 設定file header
    struct BmpFileHeader file_h = {
        .bfTybe = 0x04d42,
        .bfReserved1 = 0,
        .bfReserved2 = 0,
        .bfOffBits = 54,
    };
    file_h.bfSize = file_h.bfOffBits + width * height * bits / 8;
    //設定 info header
    if (bits == 8) { file_h.bfSize += 1024, file_h.bfOffBits += 1024; } // 灰階調色盤
    // info of raw start
    struct BmpInfoHeader info_h = {
        .biSize = 40,
        .biPlanes = 1,
        .biCompression = 0,
        .biXPelsPerMeter = 0,
        .biYPelsPerMeter = 0,
        .biClrUsed = 0,
        .biClrImportant = 0,

    };
    info_h.biWidth = width;
    info_h.biHeight = height;
    info_h.biBitCount = bits;
    info_h.biSizeImage = width * height * bits / 8;
    if (bits == 8) { info_h.biClrUsed = 256; }
    // info of raw end
    FILE* pFile = NULL;                 
    pFile = fopen(name, "wb+");
    fwrite((char*)&file_h, sizeof(char), sizeof(file_h), pFile);
    fwrite((char*)&info_h, sizeof(char), sizeof(info_h), pFile);
    // 寫 調色盤

    size_t alig = ((width * bits / 8) * 3) % 4;  //bmp 4bit 對齊的公式 size_t alig = ((width*bits/8)*3) % 4;
    for (int j = height - 1; j >= 0; --j) {
        for (unsigned i = 0; i < width; ++i) {
            uint32_t idx = j * width + i;
            if (bits == 24) {
                fwrite((char*)&raw_img[idx * 3 + 2],
                    sizeof(char), sizeof(uch), pFile);
                fwrite((char*)&raw_img[idx * 3 + 1],
                    sizeof(char), sizeof(uch), pFile);
                fwrite((char*)&raw_img[idx * 3 + 0],
                    sizeof(char), sizeof(uch), pFile);
            }
           
        }
       
        for (size_t i = 0; i < alig; ++i) {                  //對齊 4byte
            fwrite("", sizeof(char), sizeof(uch), pFile);

        }
    }
    fclose(pFile);
}


void bmpRead(const char* name, uch** raw_img,
	uint32_t* width, uint32_t* height, uint16_t* bits) {
    struct BmpFileHeader file_h;        //file header
    struct BmpInfoHeader info_h;        //info header
    FILE* pFile = NULL;                 //read head
    pFile = fopen(name, "rb+");
    fread((char*)&file_h, sizeof(char), sizeof(file_h), pFile);// 讀取file header
    fread((char*)&info_h, sizeof(char), sizeof(info_h), pFile);// 讀取info header
    /*
    fseek(pFile, 0, SEEK_END);			//caculate size
    file_size = ftell(pFile);
    fseek(pFile, SEEK_SET, 0);
    fread(buffer, file_size, 1, pFile);	//read file
    for (int i = 0; i < file_size; i++) {
        printf("%x", buffer[i]);
    }*/

    //讀取長寬
    *width = info_h.biWidth;                    
    *height = info_h.biHeight;
    *bits = info_h.biBitCount;              //bit數
    size_t Imgsize = ((size_t)*width) * ((size_t)*height) * 3; //計算img的數量，每一行的資料（藍、綠、紅）需要占3位元組外
    *raw_img = (uch*)calloc(Imgsize, sizeof(uch));
    //讀取片轉raw資訊
    fseek(pFile, file_h.bfOffBits, SEEK_SET);                           // 移動到 所有的header之後
    size_t alig = ((info_h.biWidth * info_h.biBitCount / 8) * 3) % 4;   //bmp 4bit 對齊的公式 size_t alig = ((width*bits/8)*3) % 4;
    for (int j = *height - 1; j >= 0; --j) {                            // 將資料 填入raw_img[陣列],由後往前
        for (unsigned i = 0; i < *width; ++i) {
            uint32_t idx = j * (*width) + i;
            if (*bits == 24) {
                fread((char*)&(*raw_img)[idx * 3 + 2],      // B
                    sizeof(char), sizeof(uch), pFile);
                fread((char*)&(*raw_img)[idx * 3 + 1],      //G
                    sizeof(char), sizeof(uch), pFile);
                fread((char*)&(*raw_img)[idx * 3 + 0],      //R
                    sizeof(char), sizeof(uch), pFile);

            }
           
        }
        fseek(pFile, (long)alig, SEEK_CUR);                 //目前游標位置

    }
    fclose(pFile);



}


typedef struct Imgraw {
	uint32_t width, height;
	uint16_t bits;
	uch* data;

}Imgraw;


void Imgraw_Read(const Imgraw* _this, const char* name) {
	const Imgraw* p = _this;
	bmpRead(name, &p->data, &p->width, &p->height, &p->bits);
}

void Imgraw_write(const Imgraw* _this, const char* name) {
    const Imgraw* p = _this;
    bmpWrite(name, p->data, p->width, p->height, p->bits);
    
}

int main() {
    // 建構
    Imgraw img = { 0, 0, 0, NULL };
    // 讀圖
    Imgraw_Read(&img, "test.bmp");
    //寫圖
    Imgraw_write(&img, "copy.bmp");
    system("pause");
    return 0;
}