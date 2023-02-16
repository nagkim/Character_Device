obj-m += char_dev.o

all:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) modules
	$(CC) main.c -o main
	
clean:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) clean
	rm main rf *.o *.out *.ko *~ core .depend *.mod.c .*.cmd .tmp_versions .*.o.d *.mod modules.order 		*.symvers