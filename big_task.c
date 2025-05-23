#include <stdio.h>
#include <stdlib.h> 
#include <strings.h>
#include <math.h> 
#include "lodepng/lodepng.h"

//загрузка рисунка
unsigned char* load_png(const char* filename, unsigned int* width, unsigned int* height) 
{
  unsigned char* image = NULL; 
  int error = lodepng_decode32_file(&image, width, height, filename);
  if(error != 0) {
    printf("error %u: %s\n", error, lodepng_error_text(error)); 
  }
  return (image);
}

//запись рисунка
void write_png(const char* filename, const unsigned char* image, unsigned width, unsigned height)
{
  unsigned char* png;
  long unsigned int pngsize;
  int error = lodepng_encode32(&png, &pngsize, image, width, height);
  if(error == 0) {
      lodepng_save_file(png, pngsize, filename);
  } else { 
    printf("error %u: %s\n", error, lodepng_error_text(error));
  }
  free(png);
}


// вариант огрубления серого цвета в ЧБ  по порогам low и high
void contrast(unsigned char *col, int bw_size, int low, int high)
{ 
    int i; 
    for(i=0; i < bw_size; i++)
    {
        if(col[i] < low)
        col[i] = 0; 
        if(col[i] > high)
        col[i] = 255;
    } 
    return; 
} 


// Гауссово размыттие
void Gauss_blur(unsigned char *col, unsigned char *blr_pic, int width, int height)
{ 
    int i, j; 
    for(i=1; i < height-1; i++) 
        for(j=1; j < width-1; j++)
        { 
            blr_pic[width*i+j] = 0.084*col[width*i+j] + 0.084*col[width*(i+1)+j] + 0.084*col[width*(i-1)+j]; 
            blr_pic[width*i+j] = blr_pic[width*i+j] + 0.084*col[width*i+(j+1)] + 0.084*col[width*i+(j-1)]; 
            blr_pic[width*i+j] = blr_pic[width*i+j] + 0.063*col[width*(i+1)+(j+1)] + 0.063*col[width*(i+1)+(j-1)]; 
            blr_pic[width*i+j] = blr_pic[width*i+j] + 0.063*col[width*(i-1)+(j+1)] + 0.063*col[width*(i-1)+(j-1)]; 
        } 
   return; 
} 

//  Место для экспериментов
void color(unsigned char *blr_pic, unsigned char *res, int size)
{ 
  int i;
    for(i=1;i<size;i++) 
    { 
        res[i*4]=40+blr_pic[i]+0.35*blr_pic[i-1]; 
        res[i*4+1]=65+blr_pic[i]; 
        res[i*4+2]=170+blr_pic[i]; 
        res[i*4+3]=255; 
		/* unsigned char C1, C2, C3, alpha=255;
		int grey = blr_pic[i];
		if (grey >= 0 && grey < 85) 
                C1 = C2 = C3 = grey * 3; 
            else if (grey >= 85 && grey < 170){
				C2 = (grey - 85) * 3;
				C1 = 255; 
			}                
            else {
                C3 = (grey - 170) * 3;
				C2 = 255;
				C1 = 255; 
			};
		res[i*4]   = C1; //красный
		res[i*4+1] = C2; //зеленый
		res[i*4+2] = C3; //синий
		res[i*4+3] = alpha; //альфа */
    } 
    return; 
} 

void to_grey(unsigned char *pic, unsigned char *bw_pic, int size_pic){
	
	for(int i=0; i<size_pic-4; i+=4){
		int r = pic[i+1];
		int g = pic[i+2];
		int b = pic[i+3];
		int alpha = pic[i];
		int grey = r*.3 + g*0.5 + b*0.11;// + alpha*0.5;
		bw_pic[i/4]=grey;
	}
} 
void deep_copy(unsigned char *in, unsigned char *out, int size){
	for(int i=0;i<size;i++){
		out[i] = in[i];
	}
}
void bernsen_binarization(unsigned char *pic, int width, int height, char contrast_thresold, int r){
	unsigned char* bw_pic = (unsigned char*)calloc(width*height,sizeof(unsigned char));
	deep_copy(pic, bw_pic, width*height);
	int Y; //яркость
	for(int i = height-r; i>r; i--)
		for(int j = width-r; j>r; j--){
			int Imin = 255, Imax = 0;
			for(int k = i-r; k <i+r; k++){
				for(int l=j-r;l<j+r; l++){
					Imax = pic[k*width+l]>Imax ? pic[k*width +l] : Imax;
					Imin = pic[k*width+l]<Imin ? pic[k*width +l] : Imin;				
				}
			}
			if((Imax-Imin) > contrast_thresold) 
				Y = 255;
			else
				Y = (pic[i*width +j] < (Imax-Imin)/2) ? 255 : 0;
			bw_pic[i*width +j] = Y;
		}
	deep_copy(bw_pic, pic, width*height); 
	free(bw_pic);
}


void bw_to_pic(unsigned char *pic, unsigned char *bw_pic, int size_pic){
	for(int i=0; i<size_pic-4; i+=4){
		int Y = bw_pic[i/4];
		pic[i]=Y;
		pic[i+1]=Y;
		pic[i+2]=Y;
		pic[i+3]=255;		
	}
}

void simple_colorization(unsigned char *pic, int size){
	int C1,C2,C3;
	for(int i=0; i<size-4; i+=4){
		int r = pic[i];
		int g = pic[i+1];
		int b = pic[i+2];
		int alpha = pic[i+3];
		
		int grey = r*.3 + g*0.59 + b*0.11;// + alpha*0.5;
		if (grey >= 0 && grey < 85) 
                C1 = C2 = C3 = grey * 3; 
            else if (grey >= 85 && grey < 170){
				C2 = (grey - 85) * 3;
				C1 = 255; 
			}                
            else {
                C3 = (grey - 170) * 3;
				C2 = 255;
				C1 = 255; 
			};
		pic[i]   = C1; //красный
		pic[i+1] = C2; //зеленый
		pic[i+2] = C3; //синий
		pic[i+3] = alpha; //альфа
	}
}  
void erode(unsigned char *pic, int width, int height) {
	unsigned char* bw_pic = (unsigned char*)calloc(width*height,sizeof(unsigned char));
	deep_copy(pic, bw_pic, width*height);
	int r=1; //радиус эрозии
	for(int i = height-r; i>r; i--)
		for(int j = width-r; j>r; j--){
			int res = 255;
			for(int k = i-r; k <i+r; k++){
				for(int l=j-r;l<j+r; l++){
					res *= pic[k*width+l];				
				}
			}
			bw_pic[i*width +j] = res>0?255:0;
		}
	deep_copy(bw_pic, pic, width*height); 
	free(bw_pic);
}

void dilate(unsigned char *pic, int width, int height) {
	unsigned char* bw_pic = (unsigned char*)calloc(width*height,sizeof(unsigned char));
	deep_copy(pic, bw_pic, width*height);
	int r=1; //радиус эрозии
	for(int i = height-r; i>r; i--)
		for(int j = width-r; j>r; j--){
			int res = 0;
			for(int k = i-r; k <i+r; k++){
				for(int l=j-r;l<j+r; l++){
					res += pic[k*width+l];				
				}
			}
			bw_pic[i*width +j] = res>0?255:0;
		}
	deep_copy(bw_pic, pic, width*height); 
	free(bw_pic);
}

int main() 
{ 
    const char* filename = "imagine.png"; 
    unsigned int width, height;
    int size;
    int bw_size;
    
    // Прочитали картинку (RGBA) -> Y
    unsigned char* picture = load_png(filename, &width, &height); 
    if (picture == NULL)
    { 
        printf("Problem reading picture from the file %s. Error.\n", filename); 
        return -1; 
    }
	printf("Файл %s загружен. Размер изображения %d x %d\n", filename, width, height);

    size = width * height * 4;
    bw_size = width * height;
    
    //цвет.изобр -> чб.изобр.
    unsigned char* bw_pic = (unsigned char*)malloc(bw_size*sizeof(unsigned char)); 
    unsigned char* blr_pic = (unsigned char*)malloc(bw_size*sizeof(unsigned char)); 
    unsigned char* finish = (unsigned char*)malloc(size*sizeof(unsigned char)); 
	
	
	
	
	//шкала серого
	printf("Простое преобразование из шкалы серого в цвет ...  ");
	to_grey(picture, bw_pic, size); //массив яркостей
	printf("Выполнено. \nПромежуточный результат сохранен в файл contrast.png\n");
	
	//шкалу серого в цвет (простое преобразование)
	printf("Простое преобразование из шкалы серого в цвет ...  ");
	contrast(bw_pic, bw_size,25, 155); 
	Gauss_blur(bw_pic, blr_pic, width, height); 
	bw_to_pic(finish, blr_pic, size);
	simple_colorization(finish,size);
	contrast(finish, size,45, 175); 
	write_png("simple.png", finish, width, height);
	printf("Выполнено. \nПромежуточный результат сохранен в файл simple.png\n");
	
	//контраст
	printf("Увеличение контраста...  ");
	contrast(bw_pic, bw_size,15, 125); 
	bw_to_pic(finish, bw_pic, size);
	write_png("contrast.png", finish, width, height);
	printf("Выполнено. \nПромежуточный результат сохранен в файл contrast.png\n");
	
	//бинаризация (черно-белый)
	printf("Преобразование в черно-белый цвет с бинаризацией по Бернсену...  ");
	bernsen_binarization(bw_pic, width, height, 47, 3);
	bw_to_pic(finish, bw_pic, size);
	write_png("wb.png", finish, width, height);
	printf("Выполнено. \nПромежуточный результат сохранен в файл wb.png\n");
	
    // размытие Гауссом
	printf("Гауссово размытие ...  ");
    Gauss_blur(bw_pic, blr_pic, width, height); 
	bw_to_pic(finish, blr_pic, size);
    // посмотрим на промежуточные картинки
    write_png("gauss.png", finish, width, height);
    printf("Выполнено. \nПромежуточный результат сохранен в файл gauss.png\n");
	
	
	
	//эрозия изображения
	printf("Эрозия изображения ...  ");
    erode(blr_pic, width, height);
    bw_to_pic(finish, blr_pic, size);
    write_png("erode.png", finish, width, height);
	printf("Выполнено. \nПромежуточный результат сохранен в файл erode.png\n");
	
	//Раскраска
	printf("Раскраска изображения ...  ");
    color(blr_pic, finish, bw_size); 
    write_png("picture_out.png", finish, width, height);
	printf("Выполнено. \nПромежуточный результат сохранен в файл picture_out.png\n");
	
	//дилатация изображения
	printf("Дилатация изображения ...  ");
	dilate(blr_pic, width, height);
	bw_to_pic(finish, blr_pic, size);
    write_png("dilate.png", finish, width, height);
	printf("Выполнено. \nПромежуточный результат сохранен в файл dilate.png\n");
	
	
    
    
    // не забыли почистить память!
    free(bw_pic); 
    free(blr_pic); 
    free(finish); 
    free(picture); //*/
    
    return 0; 
}