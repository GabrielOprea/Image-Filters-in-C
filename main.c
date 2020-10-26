#include <stdio.h>
#include <stdlib.h>
#include "bmp_header.h"
#include <string.h>

typedef unsigned char byte;

byte** AllocPixels(int height, int width)
{
	byte **pixel = calloc(height, sizeof(byte*));
	for(int i = 0; i < height; i++)
	{
		*(pixel + i) = calloc(3*width, sizeof(byte));
	}

	return pixel;
}

//int** AllocPixels_int(int height; int width)

//RGB**

void CopyPixels(byte** pixel1, byte** pixel2, int height, int width)
{
	for(int i = 0 ; i <  height; i++)
		for(int j = 0; j < 3 * width; j++)
			pixel1[i][j] = pixel2[i][j];

}
int det_padding(int width)
{
	int padding = 4 - (width * 3) % 4;
	if(padding == 4)
		padding = 0;
	return padding;
}

int** read_input(char* operation, int* pooling, int* clustering, char* name, int* size)
{
	
	FILE* input = fopen("input.txt", "rt");
	char *filter_file, *pooling_file, *cluster_file;
	fgets(name, 10, input);

	filter_file = calloc(30, sizeof(char));
	pooling_file = calloc(30, sizeof(char));
	cluster_file = calloc(30, sizeof(char));
	
	fscanf(input, "%s", filter_file);
	fscanf(input, "%s", pooling_file);
	fscanf(input, "%s", cluster_file);

	FILE* in_filter = fopen(filter_file, "rt");

	fscanf(in_filter, "%d", size);

	int** filter = calloc(*size, sizeof(int*));
	for(int i = 0; i < *size; i++)
	{
		*(filter + i) = calloc(*size, sizeof(int));
	}

	for(int i = 0; i < *size; i++)
		for(int j = 0; j < *size ; j++)
			fscanf(in_filter,"%d ",&filter[i][j]);
	
	FILE* in_pooling = fopen(pooling_file, "rt");
	fscanf(in_pooling, "%c %d", operation, pooling);

	FILE* in_clustering = fopen(cluster_file, "rt");
	fscanf(in_clustering, "%d", clustering);

	fclose(input);
	fclose(in_pooling);
	fclose(in_filter);
	fclose(in_clustering);
	free(filter_file);
	free(pooling_file);
	free(cluster_file);
	return filter;
}

byte** read_image(FILE* fisier,bmp_fileheader* header, bmp_infoheader* info)
{
	fread(&header->fileMarker1, sizeof(char), 1, fisier);
	fread(&header->fileMarker2, sizeof(char), 1, fisier);
	fread(&header->bfSize, sizeof(int), 1, fisier);
	fread(&header->unused1, sizeof(short), 1, fisier);
	fread(&header->unused2, sizeof(short), 1, fisier);
	fread(&header->imageDataOffset, sizeof(int), 1, fisier);

	fread(&info->biSize, sizeof(int), 1, fisier);
	fread(&info->width, sizeof(int), 1, fisier);
	fread(&info->height, sizeof(int), 1, fisier);
	fread(&info->planes, sizeof(short), 1, fisier);
	fread(&info->bitPix, sizeof(short), 1, fisier);
	fread(&info->biCompression, sizeof(int), 1, fisier);
	fread(&info->biSizeImage, sizeof(int), 1, fisier);
	fread(&info->biXPelsPerMeter, sizeof(int), 1, fisier);
	fread(&info->biYPelsPerMeter, sizeof(int), 1, fisier);
	fread(&info->biClrUsed, sizeof(int), 1, fisier);
	fread(&info->biClrImportant, sizeof(int), 1, fisier);

	byte **pixel = AllocPixels(info->height, info->width);

	fseek(fisier, header->imageDataOffset, 0);

	int padding = det_padding(info->width);

	for(int i = 0; i < info->height; i++)
	{
		fread(pixel[i], sizeof(byte), 3 * info->width, fisier);
		fseek(fisier, padding * sizeof(byte), 1);
	}

return pixel;

}

void build_image(FILE* fisier,bmp_fileheader* header, bmp_infoheader* info, byte** pixel)
{
	fwrite(&header->fileMarker1, sizeof(char), 1, fisier);
	fwrite(&header->fileMarker2, sizeof(char), 1, fisier);
	fwrite(&header->bfSize, sizeof(int), 1, fisier);
	fwrite(&header->unused1, sizeof(short), 1, fisier);
	fwrite(&header->unused2, sizeof(short), 1, fisier);
	fwrite(&header->imageDataOffset, sizeof(int), 1, fisier);

	fwrite(&info->biSize, sizeof(int), 1, fisier);
	fwrite(&info->width, sizeof(int), 1, fisier);
	fwrite(&info->height, sizeof(int), 1, fisier);
	fwrite(&info->planes, sizeof(short), 1, fisier);
	fwrite(&info->bitPix, sizeof(short), 1, fisier);
	fwrite(&info->biCompression, sizeof(int), 1, fisier);
	fwrite(&info->biSizeImage, sizeof(int), 1, fisier);
	fwrite(&info->biXPelsPerMeter, sizeof(int), 1, fisier);
	fwrite(&info->biYPelsPerMeter, sizeof(int), 1, fisier);
	fwrite(&info->biClrUsed, sizeof(int), 1, fisier);
	fwrite(&info->biClrImportant, sizeof(int), 1, fisier);

	int dif = ftell(fisier) - header->imageDataOffset;

	if(dif !=0)
	{
		byte* zero = calloc(dif, sizeof(byte));
		fwrite(zero, sizeof(byte), dif, fisier);
		free(zero);
	}

	int padding = det_padding(info->width);
	
	byte* zero = calloc(padding, sizeof(byte));

	for(int i = 0 ;i < info->height; i++)
	{
		fwrite(pixel[i], sizeof(byte), 3 * info->width, fisier);

		fwrite(zero, sizeof(byte), padding, fisier);
	}

	free(zero);
}

void Free_All(int** filter, int size, bmp_fileheader* header, bmp_infoheader* info, char* name)
{
	for(int i = 0; i < size; i++)
	free(filter[i]);
	free(filter);
	free(header);
	free(info);
	free(name);
}
char* buildname(char* name, char* task)
{
	char* output_name = calloc(strlen(name) + strlen(task) + 1, sizeof(char));
	strcpy(output_name, name);

	output_name[strlen(name) - strlen(".bmp")] = '\0';
	strcat(output_name, task);
	strcat(output_name, ".bmp");
	return output_name;
}
void Reverse_Pixels(byte** pixel, bmp_infoheader* info)
{

	for(int i = 0;i < info->height/2; i++)
		for(int  j = 0; j < 3 * info->width ;j++)
		{
			int aux = pixel[i][j];
			pixel[i][j] = pixel[info->height - i - 1][j];
			pixel[info->height - i - 1][j] = aux;
		}
}

void Free_Pixel(byte** pixel, int height, int width)
{
	for(int i = 0 ; i < height; i++)
		free(pixel[i]);
	free(pixel);
}

void Black_White(byte** pixel, bmp_infoheader* info, bmp_fileheader* header, char* name)
{
	
	char* output_name = buildname(name, "_black_white");
	FILE* output = fopen(output_name, "wb");
	free(output_name);

	for(int i = 0; i < info->height; i++)
	{
		for(int j = 0; j < 3* info->width; j+=3)
		{
			byte X = (pixel[i][j] + pixel[i][j+1] + pixel[i][j+2]) / 3;
			pixel[i][j] = X;
			pixel[i][j+1] = X;
			pixel[i][j+2] = X;
		}
	}

	build_image(output, header, info, pixel);
	fclose(output);
}

void No_Crop(byte** pixel, bmp_infoheader* info, bmp_fileheader* header, char* name)
{

	char* output_name = buildname(name, "_nocrop");
	FILE* output = fopen(output_name, "wb");
	free(output_name);

	int height = info->height;
	int width = info->width;
	int diff = height - width;

	if(diff > 0)
	{
		for(int i = 0; i < height; i++)
		{
			byte* aux= realloc(pixel[i], 3 * height * sizeof(byte));
			if(aux != NULL)
				pixel[i] = aux;
		}
		int lspace = diff/2;
		if(diff % 2 == 1)
			lspace++;

		for(int i = 0; i < height; i++)
			for(int j = 3 * (height - lspace) -1 ; j >= 3 * lspace; j--)
				pixel[i][j] = pixel[i][j - 3 * lspace];
		
		for(int i = 0 ; i < height ; i++)
			for(int j = 0 ; j < 3 * lspace; j++)
				pixel[i][j] = 255;

		for(int i = 0 ; i < height ; i++)
			for(int j = 3 * (height - lspace); j < 3* height; j++)
				pixel[i][j] = 255;

	info->width = info->height;

	}

	else
		if(diff < 0)
		{
			diff *= -1;

			byte** aux = realloc(pixel, width * sizeof(byte*));
			if(aux != NULL)
				pixel = aux;
			for(int i = height; i < width; i++)
				pixel[i] = calloc(3 * width, sizeof(byte));

			int uspace = diff/2;
			if(diff % 2 == 1)
				uspace ++;


			for(int i = height + uspace - 1; i >= uspace; i--)
				for(int j = 0; j < 3 * width; j++)
					pixel[i][j]=pixel[i - uspace][j];

			for(int i = 0; i < uspace; i++)
				for(int j = 0; j < 3 * width; j++)
					pixel[i][j] = 255;
			
			for(int i = height + uspace; i < width; i++)
				for (int j = 0; j < 3 * width; j++)
					pixel[i][j] = 255;

		info->height = width;

		}

build_image(output, header, info, pixel);

	for(int i = 0 ;i < info->height; i++)
		free(pixel[i]);
	free(pixel);
	fclose(output);

}

int minimum(int a, int b)
{
	return a < b? a : b;
}

int maximum(int a, int b)
{
	return a > b? a : b;
}

int abs(int a)
{
	return (unsigned) a;
}
int to_byte(int a)
{
	if(a > 255)
		a = 255;
	if(a < 0)
		a = 0;
	return a;
}

void Filters(int** filter, int size, byte** pixel, bmp_infoheader* info,
bmp_fileheader* header, char* name)
{


	char* output_name = buildname(name, "_filter");
	FILE* output = fopen(output_name, "wb");
	free(output_name);
	

	int height = info->height;
	int width = info->width;

	Reverse_Pixels(pixel, info);

	byte** new_pixel = AllocPixels(height, width);

	for(int i = 0; i < height; i++)
		for(int j = 0; j < 3 * width; j++)
		{

			int sum = 0;

			for(int k = 0; k < size; k++)
				for(int l = 0; l < size; l++)
				{
					int line = i - size/2 + k;
					int col = j + 3 * (l - size/2);
					if(line >= 0 && line < height && col >=0 && col < 3*width)

						sum += pixel[line][col] * filter[k][l];
				}

			sum = to_byte(sum);

			new_pixel[i][j] = sum;

		}

	CopyPixels(pixel, new_pixel, height, width);

	Reverse_Pixels(pixel, info);

	Free_Pixel(new_pixel, height, width);

	build_image(output, header, info, pixel);

}

void Pooling(int size, byte** pixel, bmp_infoheader* info,
bmp_fileheader* header, char* name, char operation)
{

	char* output_name = buildname(name, "_pooling");
	FILE* output = fopen(output_name, "wb");
	free(output_name);

	int height = info->height;
	int width = info->width;

	byte** new_pixel = AllocPixels(height, width);

	for(int i = 0; i < height; i++)
		for(int j = 0; j < 3 * width; j++)
		{
			int max = 0;
			int min = 255;

			for(int k = 0; k < size; k++)
				for(int l = 0; l < size; l++)
				{
					int line = i - size/2 + k;
					int col = j + 3 * (l - size/2);
					if(line >= 0 && line < height && col >=0 && col < 3*width)
					{
						max = maximum(max, pixel[line][col]);
						min = minimum(min, pixel[line][col]);
					}
					else
						min = 0;
				}

			if(operation == 'M')
				new_pixel[i][j] = max;
			else
				if(operation == 'm')
					new_pixel[i][j] = min;
		}

	CopyPixels(pixel, new_pixel, height, width);

	Free_Pixel(new_pixel, height, width);

	build_image(output, header, info, pixel);
}

RGB** alloc_RGB_array(int height, int width)
{
	RGB** arr = calloc(width * height + 1, sizeof(RGB*));
	for(int i = 0; i <= width * height; i++)
		arr[i] = calloc(1, sizeof(RGB));
	if(arr == NULL)
		printf("Nu s-a alocat memorie!\n");
	return arr;
}

int** AllocPixels_int(int height, int width, byte** pixel)
{
	int** pixel_int = calloc(height, sizeof(int*));
	for(int i = 0; i < height; i++)
		pixel_int[i] = calloc(3*width, sizeof(int));

	for(int i = 0; i < height; i++)
		for(int j = 0; j < 3 * width; j++)
			pixel_int[i][j] = pixel[i][j];

	return pixel_int;
}

void Free_Pixel_int(int height, int width, int** pixel)
{
	for(int i = 0; i < height; i++)
		free(pixel[i]);
	
	free(pixel);
}

void Free_RGB_arr(int height, int width, RGB** arr)
{
	for(int i = 0 ; i < width * height + 1; i++)
		free(arr[i]);
	
	free(arr);
}

void ReplaceZones(int height, int width, int** pixel_new, byte** pixel,
RGB** zone_arr)
{
	for(int i = 0; i < height; i++)
		for(int j = 0; j < 3 * width; j += 3)
		{
			pixel[i][j] = zone_arr[abs(pixel_new[i][j])]->Red;
			pixel[i][j + 1] = zone_arr[abs(pixel_new[i][j])]->Green;
			pixel[i][j + 2] = zone_arr[abs(pixel_new[i][j])]->Blue;
		}
}

int Cond_Clustering(int i, int j, int zona, int** pixel_new, int lim,
	RGB** zone_arr)
{
	int Red = abs(pixel_new[i][j] - zone_arr[abs(zona)]->Red);
	int Green = abs(pixel_new[i][j + 1] - zone_arr[abs(zona)]->Green);
	int Blue = abs(pixel_new[i][j + 2] - zone_arr[abs(zona)]->Blue);
	if(Red + Green + Blue <= lim)
		return 1;
	else return 0;
}

void Clustering(byte** pixel, bmp_infoheader* info,
bmp_fileheader* header, char* name, int lim)
{

	char* output_name = buildname(name, "_clustered");
	FILE* output = fopen(output_name, "wb");
	free(output_name);

	int height = info->height, width = info->width;
	RGB **zone_arr = alloc_RGB_array(height, width);

	Reverse_Pixels(pixel, info);

	int** pixel_new = AllocPixels_int(height, width, pixel);

	int Red, Green, Blue, N, zona = -1;

	void clusterise(int i , int j)
	{
		Red += pixel[i][j];
		Green += pixel[i][j + 1];
		Blue += pixel[i][j + 2];
		N++;

		if(i != 0)
			if(pixel_new[i-1][j]>= 0)
			if(Cond_Clustering(i - 1, j, zona, pixel_new, lim, zone_arr))
			{
				pixel_new[i - 1][j] = zona;
				clusterise(i - 1, j);
			}
		if(j >= 3)
			if(pixel_new[i][j - 3]>= 0)
			if(Cond_Clustering(i, j - 3, zona, pixel_new, lim, zone_arr))
			{
				pixel_new[i][j - 3] = zona;
				clusterise(i, j - 3);
			}
		if(j < 3 * width - 3)
			if(pixel_new[i][j + 3]>= 0)
			if(Cond_Clustering(i , j + 3, zona, pixel_new, lim, zone_arr))
			{
				pixel_new[i][j + 3] = zona;
				clusterise(i, j + 3);
			}
		if(i < height - 1)
			if(pixel_new[i+1][j]>= 0)
			if(Cond_Clustering(i + 1 , j, zona, pixel_new, lim, zone_arr))
			{
				pixel_new[i + 1][j] = zona;
				clusterise(i + 1 , j);
			}
	}

	for(int i = 0 ; i < height; i++)
		for(int j = 0; j < 3 * width; j+=3)
		{
			if(pixel_new[i][j] >= 0)
			{
				zone_arr[abs(zona)]->Red = pixel_new[i][j];
				zone_arr[abs(zona)]->Green = pixel_new[i][j + 1];
				zone_arr[abs(zona)]->Blue = pixel_new[i][j + 2];
				pixel_new[i][j] = pixel_new[i][j + 1] = pixel_new[i][j + 2] = zona;
				Red = Green = Blue = N = 0;
				clusterise(i, j);
				zone_arr[abs(zona)]->Red = Red / N;
				zone_arr[abs(zona)]->Green = Green / N;
				zone_arr[abs(zona)]->Blue = Blue / N;
				zona --;
			}
		}

	ReplaceZones(height, width, pixel_new, pixel, zone_arr);
	Reverse_Pixels(pixel, info);
	Free_Pixel_int(height, width, pixel_new);
	Free_RGB_arr(height, width, zone_arr);

	build_image(output, header, info, pixel);
}

int main()
{

	char* name = calloc(10, sizeof(char));
	int size, pooling, clustering, **filter;
	char operation;

	filter = read_input(&operation, &pooling, &clustering, name, &size);

	bmp_fileheader* header = calloc(1,sizeof(bmp_fileheader));
	bmp_infoheader* info = calloc(1, sizeof(bmp_infoheader));

	FILE* image = fopen(name, "rb");

	if(!image)
		printf("Fisierul nu exista sau nu s-a deschis!\n");

	byte** pixel;
	pixel = read_image(image, header, info);

	fclose(image);

	int height = info->height;
	int width = info->width;

	byte** pixel_copy = AllocPixels(height, width);
	
	CopyPixels(pixel_copy, pixel, height, width);
	Clustering(pixel_copy, info, header, name, clustering);

	CopyPixels(pixel_copy, pixel, height, width);
	Black_White(pixel_copy, info, header, name);

	CopyPixels(pixel_copy, pixel, height, width);
	Filters(filter, size, pixel_copy, info, header, name);

	CopyPixels(pixel_copy, pixel, height, width);
	Pooling(pooling, pixel_copy, info, header, name, operation);
	
	CopyPixels(pixel_copy, pixel, height, width);
	No_Crop(pixel_copy, info, header, name);

	Free_Pixel(pixel, height, width);

	Free_All(filter, size, header, info, name);
}