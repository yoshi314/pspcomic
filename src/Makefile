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
librar:
	make -C rar -f Makefile.generic lib
	mv -f rar/libunrar.a .
libxml:
	make -C tinyxml -f Makefile.generic 
	mv -f tinyxml/libtinyxml.a .
windows:
	WINDOWS=1 make -f Makefile.generic
clean:
	rm -fv *.o zip/*.o tinyxml/*.o rar/*.o PARAM.SFO EBOOT.PBP pspcomic.prx
	rm -rfv 3.xx 1.50 
distclean:
	make clean
	rm -v *.a
unix_app:
	make -f Makefile.generic

unix: librar libxml unix_app

docs:
	doxygen pspcomic.doxy
