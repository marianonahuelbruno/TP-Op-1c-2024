clean:
	make clean -C ./utils
	make clean -C ./memoria
	make clean -C ./cpu
	make clean -C ./kernel
	make clean -C ./entradasalida

all:
	make -C ./utils
	make -C ./memoria
	make -C ./cpu
	make -C ./kernel
	make -C ./entradasalida