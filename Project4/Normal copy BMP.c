#define _CRT_SECURE_NO_WARNINGS
#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
typedef unsigned char uch;


// �ɮ׵��c
#pragma pack(2) //�O�����m�s��
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

    // �]�wfile header
    struct BmpFileHeader file_h = {
        .bfTybe = 0x04d42,
        .bfReserved1 = 0,
        .bfReserved2 = 0,
        .bfOffBits = 54,
    };
    file_h.bfSize = file_h.bfOffBits + width * height * bits / 8;
    //�]�w info header
    if (bits == 8) { file_h.bfSize += 1024, file_h.bfOffBits += 1024; } // �Ƕ��զ�L
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
    // �g �զ�L

    size_t alig = ((width * bits / 8) * 3) % 4;  //bmp 4bit ��������� size_t alig = ((width*bits/8)*3) % 4;
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
       
        for (size_t i = 0; i < alig; ++i) {                  //��� 4byte
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
    fread((char*)&file_h, sizeof(char), sizeof(file_h), pFile);// Ū��file header
    fread((char*)&info_h, sizeof(char), sizeof(info_h), pFile);// Ū��info header
    /*
    fseek(pFile, 0, SEEK_END);			//caculate size
    file_size = ftell(pFile);
    fseek(pFile, SEEK_SET, 0);
    fread(buffer, file_size, 1, pFile);	//read file
    for (int i = 0; i < file_size; i++) {
        printf("%x", buffer[i]);
    }*/

    //Ū�����e
    *width = info_h.biWidth;                    
    *height = info_h.biHeight;
    *bits = info_h.biBitCount;              //bit��
    size_t Imgsize = ((size_t)*width) * ((size_t)*height) * 3; //�p��img���ƶq�A�C�@�檺��ơ]�šB��B���^�ݭn�e3�줸�ե~
    *raw_img = (uch*)calloc(Imgsize, sizeof(uch));
    //Ū������raw��T
    fseek(pFile, file_h.bfOffBits, SEEK_SET);                           // ���ʨ� �Ҧ���header����
    size_t alig = ((info_h.biWidth * info_h.biBitCount / 8) * 3) % 4;   //bmp 4bit ��������� size_t alig = ((width*bits/8)*3) % 4;
    for (int j = *height - 1; j >= 0; --j) {                            // �N��� ��Jraw_img[�}�C],�ѫ᩹�e
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
        fseek(pFile, (long)alig, SEEK_CUR);                 //�ثe��Ц�m

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
    // �غc
    Imgraw img = { 0, 0, 0, NULL };
    // Ū��
    Imgraw_Read(&img, "test.bmp");
    //�g��
    Imgraw_write(&img, "copy.bmp");
    system("pause");
    return 0;
}