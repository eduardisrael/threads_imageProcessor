procesador: threads_imageProcessor.c
	gcc -Wall threads_imageProcessor.c -o procesador -lpng -lpthread

.PHONY: clean
clean:
	rm procesador
