obj-m +=findoffsets.o
obj-m +=vmitest.o

all:
	#make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules -lcsys
	make -C /lib/modules/2.6.27.7/build M=$(PWD) modules -lcsys

clean:
	#make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	make -C /lib/modules/2.6.27.7/build M=$(PWD) clean
