#include <stdio.h>
#include <png.h>
#include <pthread.h>
#include <time.h>
/* Parte del codigo tomado de https://gist.github.com/niw/5963798 */

#include <stdlib.h>
#include <stdio.h>
#include <png.h>
#include <unistd.h>
//Retorna los pixeles de la imagen, y los datos relacionados en los argumentos: ancho, alto, tipo de color (normalmente RGBA) y bits por pixel (usualemente 8 bits)
png_bytep * abrir_archivo_png(char *filename, int *width, int *height, png_byte *color_type, png_byte *bit_depth) {
	FILE *fp = fopen(filename, "rb");

	png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if(!png) abort();

	png_infop info = png_create_info_struct(png);
	if(!info) abort();

	if(setjmp(png_jmpbuf(png))) abort();

	png_init_io(png, fp);

	png_read_info(png, info);

  //resolucion y color de la image. Usaremos espacio de color RGBA
	*width      = png_get_image_width(png, info);
	*height     = png_get_image_height(png, info);
	*color_type = png_get_color_type(png, info);
	*bit_depth  = png_get_bit_depth(png, info);

  // Read any color_type into 8bit depth, RGBA format.
  // See http://www.libpng.org/pub/png/libpng-manual.txt

	if(*bit_depth == 16)
		png_set_strip_16(png);

	if(*color_type == PNG_COLOR_TYPE_PALETTE)
		png_set_palette_to_rgb(png);

  // PNG_COLOR_TYPE_GRAY_ALPHA is always 8 or 16bit depth.
	if(*color_type == PNG_COLOR_TYPE_GRAY && *bit_depth < 8)
		png_set_expand_gray_1_2_4_to_8(png);

	if(png_get_valid(png, info, PNG_INFO_tRNS))
		png_set_tRNS_to_alpha(png);

  // These color_type don't have an alpha channel then fill it with 0xff.
	if(*color_type == PNG_COLOR_TYPE_RGB ||
		*color_type == PNG_COLOR_TYPE_GRAY ||
		*color_type == PNG_COLOR_TYPE_PALETTE)
		png_set_filler(png, 0xFF, PNG_FILLER_AFTER);

	if(*color_type == PNG_COLOR_TYPE_GRAY ||
		*color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
		png_set_gray_to_rgb(png);

	png_bytep *row_pointers;
	png_read_update_info(png, info);
	row_pointers = (png_bytep*)malloc(sizeof(png_bytep) * (*height));
	if(row_pointers == NULL){
		printf("Error al obtener memoria de la imagen\n");
		exit(-1);
	}
	for(int y = 0; y < *height; y++) {
		row_pointers[y] = (png_byte*)malloc(png_get_rowbytes(png,info));
	}

	png_read_image(png, row_pointers);

	fclose(fp);
	return row_pointers;
}

double obtenerTiempoActual(){
	struct timespec tsp;
	clock_gettime(CLOCK_REALTIME, &tsp);
	double secs=(double)tsp.tv_sec;
	double nano = (double) tsp.tv_nsec/1000000000.0;
	return secs + nano;
}

//Usaremos bit depth 8
//Color type PNG_COLOR_TYPE_GRAY_ALPHA
void guardar_imagen_png(char *filename, int width, int height, png_byte color_type, png_byte bit_depth, png_bytep *res) {
  //int y;

	FILE *fp = fopen(filename, "wb");
	if(!fp) abort();

	png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png) abort();

	png_infop info = png_create_info_struct(png);
	if (!info) abort();

	if (setjmp(png_jmpbuf(png))) abort();

	png_init_io(png, fp);

  // Salida es escala de grises
	png_set_IHDR(
		png,
		info,
		width, height,
		bit_depth,
		color_type,
		PNG_INTERLACE_NONE,
		PNG_COMPRESSION_TYPE_DEFAULT,
		PNG_FILTER_TYPE_DEFAULT
		);
	png_write_info(png, info);

  // To remove the alpha channel for PNG_COLOR_TYPE_RGB format,
  // Use png_set_filler().
  //png_set_filler(png, 0, PNG_FILLER_AFTER);
  png_write_image(png, res);	//row_pointers
  png_write_end(png, NULL);
  for(int y = 0; y < height; y++) {
    free(res[y]);		//row_pointers
}
  free(res);			//row_pointers

  fclose(fp);
}

typedef struct{
	int width;
	int height;
	png_bytep *row_pointers;
	int id;
	int n;
	png_bytep *res;

}thread_info;

void *procesar_thread(void *var){
	thread_info *c = (thread_info *)var;
	

	for(int i = c->id;i<c->height;i+=c->n){


			c->res[i] = malloc(sizeof(png_bytep)*c->width*2);			//greyscale con alpha, 2 bytes por pixel

			if(c->res[i] == NULL){
				printf("No se pudo reservar espacio para la imagen procesada");
				exit(-1);
			}

			png_bytep row = c->row_pointers[i];
			for(int x = 0; x < c->width; x++) {
				png_bytep px = &(row[x * 4]);

			  //Convertimos a escala de grises
				float a = .299f * px[0] + .587f * px[1] + .114f * px[2];
				c->res[i][2*x] = a;
			  c->res[i][2*x + 1] = px[3]; //transparencia... dejamos el campo de transparencia igual.

			}
		
		
	}

	return c;
	
}

png_bytep* procesar_archivo_png_thread(int width, int height, int num_threads, png_bytep *row_pointers,png_byte bit_depth) {
	
	
	pthread_t ids[num_threads];
	png_bytep *res = (png_bytep*)malloc(sizeof(png_bytep *) * height);

	for(int i = 0; i<num_threads;i++){
		pthread_t id;

	

		thread_info *ti = (thread_info *) malloc(sizeof(thread_info));
		ti->width = width;
		ti->height = height;
		ti->row_pointers = row_pointers;
		ti->n = num_threads;
		ti->id=i;
		ti->res=res;
		pthread_create(&id,NULL,procesar_thread,ti);
		ids[i]=id;
	}
	
	for(int i=0;i<num_threads;i++){
		void *c;
		pthread_join(ids[i],&c);
		free(c);
	}
	for(int i = 0 ; i<height;i++){
		free(row_pointers[i]);
	}
	free(row_pointers);


	


	
	return res;
}


int main(int argc, char *argv[]) {

	if(argc<4) abort();

	int num_threads=-1;
	char c;
	char *in = argv[1];
	char *out = argv[2];
	while ((c = getopt (argc, argv, "n:")) != -1){
		switch (c){
			case 'n':
			num_threads = atoi(optarg);
			break;
			default:
			break;
		}
	}
	if(num_threads<=0){
		abort();
	}
  //Datos de la imagen original
	int width, height;
	png_byte color_type; 

	png_bytep *pixeles;

	png_byte bit_depth;

	pixeles = abrir_archivo_png(in,&width, &height, &color_type, &bit_depth);
	double init = obtenerTiempoActual();

	png_bytep *res = procesar_archivo_png_thread(width,height,num_threads, pixeles, bit_depth);
	
	double end = obtenerTiempoActual();
	guardar_imagen_png(out, width, height, PNG_COLOR_TYPE_GRAY_ALPHA, bit_depth, res);
	printf("Tiempo: %lf segundos\n",(end-init));


	return 0;
}


