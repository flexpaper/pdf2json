set CC=cl
set FPFLAGS=/I..\freetype\include\freetype.win32 /I..\freetype.win32\include\ /I..\freetype.win32\lib
set CFLAGS=/DWIN32 %FPFLAGS% /I.. /I..\splash /I..\goo /I..\fofi /I..\xpdf /O2 /nologo

set CXX=cl

set CXXFLAGS=%CFLAGS% /TP

set LIBPROG=lib
set FTLIBS=-lfreetype


copy aconf-win32.h aconf.h



cd goo

%CXX% %CXXFLAGS% /c GHash.cc

%CXX% %CXXFLAGS% /c GList.cc

%CXX% %CXXFLAGS% /c GString.cc

%CXX% %CXXFLAGS% /c gmempp.cc

%CXX% %CXXFLAGS% /c gfile.cc

%CC% %CFLAGS% /c gmem.cc

%CC% %CFLAGS% /c parseargs.c

%LIBPROG% /nologo /out:libGoo.lib GHash.obj GList.obj GString.obj gmempp.obj gfile.obj gmem.obj parseargs.obj



cd ..\splash

%CXX% %CXXFLAGS% /c Splash.cc 

%CXX% %CXXFLAGS% /c SplashBitmap.cc 

%CXX% %CXXFLAGS% /c SplashClip.cc 

%CXX% %CXXFLAGS% /c SplashFTFont.cc 

%CXX% %CXXFLAGS% /c SplashFTFontEngine.cc 

%CXX% %CXXFLAGS% /c SplashFTFontFile.cc 

%CXX% %CXXFLAGS% /c SplashFont.cc 

%CXX% %CXXFLAGS% /c SplashFontEngine.cc 

%CXX% %CXXFLAGS% /c SplashFontFile.cc 

%CXX% %CXXFLAGS% /c SplashFontFileID.cc 

%CXX% %CXXFLAGS% /c SplashPath.cc 

%CXX% %CXXFLAGS% /c SplashPattern.cc 

%CXX% %CXXFLAGS% /c SplashScreen.cc 

%CXX% %CXXFLAGS% /c SplashState.cc 

%CXX% %CXXFLAGS% /c SplashT1Font.cc 

%CXX% %CXXFLAGS% /c SplashT1FontEngine.cc 

%CXX% %CXXFLAGS% /c SplashT1FontFile.cc 

%CXX% %CXXFLAGS% /c SplashXPath.cc 

%CXX% %CXXFLAGS% /c SplashXPathScanner.cc

%LIBPROG% /nologo /out:libsplash.lib Splash.obj SplashBitmap.obj SplashClip.obj SplashFTFont.obj SplashFTFontEngine.obj SplashFTFontFile.obj SplashFont.obj SplashFontEngine.obj SplashFontFile.obj SplashFontFileID.obj SplashPath.obj SplashPattern.obj SplashScreen.obj SplashState.obj SplashT1Font.obj SplashT1FontEngine.obj SplashT1FontFile.obj SplashXPath.obj SplashXPathScanner.obj



cd ..\fofi

%CXX% %CXXFLAGS% /c FoFiBase.cc

%CXX% %CXXFLAGS% /c FoFiEncodings.cc

%CXX% %CXXFLAGS% /c FoFiTrueType.cc 

%CXX% %CXXFLAGS% /c FoFiType1.cc

%CXX% %CXXFLAGS% /c FoFiType1C.cc

%LIBPROG% /nologo /out:libfofi.lib FoFiBase.obj FoFiEncodings.obj FoFiTrueType.obj FoFiType1.obj FoFiType1C.obj



cd ..\xpdf

%CXX% %CXXFLAGS% /c Annot.cc

%CXX% %CXXFLAGS% /c Array.cc

%CXX% %CXXFLAGS% /c BuiltinFont.cc

%CXX% %CXXFLAGS% /c BuiltinFontTables.cc

%CXX% %CXXFLAGS% /c CMap.cc

%CXX% %CXXFLAGS% /c Catalog.cc

%CXX% %CXXFLAGS% /c CharCodeToUnicode.cc

%CXX% %CXXFLAGS% /c Decrypt.cc

%CXX% %CXXFLAGS% /c Dict.cc

%CXX% %CXXFLAGS% /c Error.cc

%CXX% %CXXFLAGS% /c FontEncodingTables.cc

%CXX% %CXXFLAGS% /c Function.cc

%CXX% %CXXFLAGS% /c Gfx.cc

%CXX% %CXXFLAGS% /c GfxFont.cc

%CXX% %CXXFLAGS% /c GfxState.cc

%CXX% %CXXFLAGS% /c GlobalParams.cc

%CXX% %CXXFLAGS% /c JArithmeticDecoder.cc

%CXX% %CXXFLAGS% /c JBIG2Stream.cc

%CXX% %CXXFLAGS% /c JPXStream.cc

%CXX% %CXXFLAGS% /c Lexer.cc

%CXX% %CXXFLAGS% /c Link.cc

%CXX% %CXXFLAGS% /c PSTokenizer.cc

%CXX% %CXXFLAGS% /c NameToCharCode.cc

%CXX% %CXXFLAGS% /c Object.cc

%CXX% %CXXFLAGS% /c Outline.cc

%CXX% %CXXFLAGS% /c OutputDev.cc

%CXX% %CXXFLAGS% /c PDFDoc.cc

%CXX% %CXXFLAGS% /c PDFDocEncoding.cc

%CXX% %CXXFLAGS% /c PSOutputDev.cc

%CXX% %CXXFLAGS% /c PSTokenizer.cc

%CXX% %CXXFLAGS% /c Page.cc

%CXX% %CXXFLAGS% /c Parser.cc

%CXX% %CXXFLAGS% /c PreScanOutputDev.cc

%CXX% %CXXFLAGS% /c Stream.cc

%CXX% %CXXFLAGS% /c SplashOutputDev.cc

%CXX% %CXXFLAGS% /c UnicodeMap.cc

%CXX% %CXXFLAGS% /c SecurityHandler.cc

%CXX% %CXXFLAGS% /c UnicodeTypeTable.cc

%CXX% %CXXFLAGS% /c XRef.cc



%LIBPROG% /nologo /out:libxpdf.lib Annot.obj Array.obj BuiltinFont.obj BuiltinFontTables.obj Catalog.obj CharCodeToUnicode.obj CMap.obj Decrypt.obj Dict.obj Error.obj FontEncodingTables.obj Function.obj Gfx.obj GfxFont.obj GfxState.obj GlobalParams.obj JArithmeticDecoder.obj JBIG2Stream.obj JPXStream.obj Lexer.obj Link.obj NameToCharCode.obj Object.obj Outline.obj OutputDev.obj Page.obj Parser.obj PDFDoc.obj PDFDocEncoding.obj PreScanOutputDev.obj PSOutputDev.obj PSTokenizer.obj Stream.obj SplashOutputDev.obj UnicodeMap.obj SecurityHandler.obj UnicodeTypeTable.obj XRef.obj 



cd ..\src

%CXX% %CXXFLAGS% /c XmlFonts.cc

%CXX% %CXXFLAGS% /c XmlLinks.cc

%CXX% %CXXFLAGS% /c ImgOutPutDev.cc

%CXX% %CXXFLAGS% /c pdf2json.cc



%CXX% %FPFLAGS% /nologo /Fepdf2json.exe XmlFonts.obj XmlLinks.obj ImgOutPutDev.obj pdf2json.obj ..\goo\libGoo.lib ..\xpdf\libxpdf.lib ..\fofi\libfofi.lib ..\splash\libsplash.lib advapi32.lib Gdi32.lib User32.lib ..\freetype.win32\lib\freetype_a.lib



cd ..