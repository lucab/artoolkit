all:
	(cd calib_distortion; make -f Makefile)
	(cd calib_cparam;     make -f Makefile)
	(cd mk_patt;          make -f Makefile)
	(cd calib_camera2;    make -f Makefile)

clean:
	(cd calib_distortion; make -f Makefile clean)
	(cd calib_cparam;     make -f Makefile clean)
	(cd mk_patt;          make -f Makefile clean)
	(cd calib_camera2;    make -f Makefile clean)

allclean:
	(cd calib_distortion; make -f Makefile allclean)
	(cd calib_cparam;     make -f Makefile allclean)
	(cd mk_patt;          make -f Makefile allclean)
	(cd calib_camera2;    make -f Makefile allclean)
	rm -f Makefile
