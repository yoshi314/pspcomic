all: psp
.PHONY: docs clean
psp:
	make clean
	make 3xx
	make clean
	make 150
psplibs: psprar psptinyxml
psprar:
	make -C rar -f Makefile.psp
	mv -f rar/libpspunrar.a .
psptinyxml:
	make -C tinyxml -f Makefile.psp
	mv -f tinyxml/libpsptinyxml.a .
3xx:
	mkdir -p 3.xx/pspcomic
	PSP_FW_VERSION=300 make -f Makefile.psp
	cp -f EBOOT.PBP 3.xx/pspcomic
	cp -f 3.xx/pspcomic/pspcomic.* 3.xx
	mv -f 3.xx/pspcomic/pspcomic.* .
150:
	mkdir -p 1.50
	make -f Makefile.psp
rar:
	make -C rar -f Makefile.generic lib
	mv -f rar/libunrar.a .
tinyxml:
	make -C tinyxml -f Makefile.generic lib
	mv -f tinyxml/libtinyxml.a .
windows:
	WINDOWS=1 make -f Makefile.generic
clean:
	rm -f *.o zip/*.o tinyxml/*.o PARAM.SFO
unix:
	make -f Makefile.generic
docs:
	doxygen pspcomic.doxy
