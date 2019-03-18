all:
	cd main
	nmake -f Makefile-windows-vc10
rebuild: clean
	cd /d D:\snt\qualnet\6.1\main
	nmake -f Makefile-windows-vc10
clean:
	cd main
	nmake -f Makefile-windows-vc10 clean