
SHELL = /bin/sh

SRCDIR = .
XPDFSRCDIR = ../xpdf
XPDFLIBDIR = ../xpdf
GOOSRCDIR = ../goo
GOOLIBDIR = ../goo
FOFISRCDIR = ../fofi
FOFILIBDIR = ../fofi
SPLASHSRCDIR = ../splash
SPLASHLIBDIR = ../splash

CXXFLAGS = -I/usr/local/include -g -O2 -DHAVE_CONFIG_H -DHAVE_DIRENT_H=1  -I.. -DHAVE_REWINDDIR=1 -DHAVE_POPEN=1 -I.. -I$(GOOSRCDIR) -I$(XPDFSRCDIR) -I$(FOFISRCDIR) -I$(SPLASHSRCDIR) -I$(srcdir)           -I/usr/X11R6/include

LDFLAGS = 
FTLIBS = 

OTHERLIBS = 

CXX ?= c++

LIBPREFIX = lib
EXE = 

#------------------------------------------------------------------------

.SUFFIXES: .cc

.cc.o:
	$(CXX) $(CXXFLAGS) -c $<

#------------------------------------------------------------------------

CXX_SRC = \
	$(SRCDIR)/pdf2json.cc \
	$(SRCDIR)/ImgOutputDev.cc \
	$(SRCDIR)/XmlFonts.cc \
	$(SRCDIR)/XmlLinks.cc 

#------------------------------------------------------------------------

all: pdf2json$(EXE)

#-------------------------------------------------------------------------

PDF2JSON_OBJS = ImgOutputDev.o XmlFonts.o XmlLinks.o \
    pdf2json.o
PDF2JSON_LIBS = -L$(GOOLIBDIR) -L$(FOFILIBDIR) -L$(SPLASHLIBDIR) $(FTLIBS) -L$(XPDFLIBDIR) $(OTHERLIBS) -lXpdf -lGoo -lfofi -lsplash -lm

pdf2json$(EXE): $(PDF2JSON_OBJS) $(GOOLIBDIR)/$(LIBPREFIX)Goo.a
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o pdf2json$(EXE) $(PDF2JSON_OBJS) \
		$(PDF2JSON_LIBS)

#-------------------------------------------------------------------------
PDF2JSON_WINOBJS = pdf2json.exe ImgOutPutDev.obj  pdf2json.obj  XmlFonts.obj  XmlLinks.obj

clean:
	rm -f $(PDF2JSON_OBJS) pdf2json$(EXE)
	rm -f $(PDF2JSON_WINOBJS)

#------------------------------------------------------------------------

distdepend:
	cp Makefile.in Makefile.in.bak
	sed '/^#----- dependences -----/q' Makefile.in.bak >Makefile.in
	$(CXX) $(CXXFLAGS) -MM $(CXX_SRC) >>Makefile.in

