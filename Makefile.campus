#
#  Makefile for fltk applications
#

INCLUDE = -I/lusr/opt/fltk1.3-1.3.0/include
LIBS = -L/lusr/opt/fltk1.3-1.3.0/lib -lfltk -lfltk_gl -lfltk_images -lGL -lGLU -ljpeg -lpng -lz

CFLAGS = -g -std=c++11 $(INCLUDE) $(LIBS) 
#CFLAGS = -O1 -std=c++11 $(INCLUDE) $(LIBS) 

CC = g++

.SUFFIXES: .o .cpp .cxx

.cpp.o: 
	$(CC) $(CFLAGS) -c -o $*.o $<

.cxx.o: 
	$(CC) $(CFLAGS) -c -o $*.o $<

ALL.O = bitmap.o camera.o color.o curve.o curveevaluator.o \
        graphwidget.o indicatorwindow.o linearcurveevaluator.o \
        modelerapp.o modelerdraw.o modelerui.o animatoruiwindows.o \
        modelerview.o particleSystem.o point.o \
        rect.o robotarm.o rulerwindow.o

animator: $(ALL.O)
	$(CC) $(CFLAGS) -o $@ $(ALL.O) $(INCLUDE) $(LIBDIR) $(LIBS)

clean:  
	rm -f $(ALL.O)

clean_all:
	rm -f $(ALL.O) animator
