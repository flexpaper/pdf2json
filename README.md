PDF2JSON
=======

PDF2JSON is a conversion library based on XPDF (3.02) which can be used for high performance PDF page by page conversion to JSON and XML format. It also supports compressing data to minimize size. PDF2JSON is available for Windows and Linux.

Usage
-----
```
Usage: pdf2json [options] <PDF-file> [<output-file>]
  -f <int>          : first page to convert
  -l <int>          : last page to convert
  -compress         : Use compressed mode
  -q                : dont print any messages or errors
  -h                : print usage information
  -help             : print usage information
  -i                : ignore images
  -noframes         : use standard output
  -xml              : output for XML post-processing
  -split <int>      : split the output into separate files on every X page. 
                      use '%' as part of output file name to specify where page number should appear (e.g. Paper_%.js)
  -hidden           : output hidden text
  -enc <string>     : output text encoding name
  -v                : print copyright and version info
  -opw <string>     : owner password (for encrypted files)
  -upw <string>     : user password (for encrypted files)
```

Compiling & Installing on Linux
-------------------
./configure<br/>
make<br/>
sudo make install<br/>


Installing on Windows
-------------------
Download the latest installer (.msi file) and run through the installation steps



Compiling on Windows
-------------------
Make sure you have Visual Studio installed and run ms_make.bat



License
-------------------
GNU GPL v2

<br/>

<h3>Maintained and sponsored by █▒▓▒░ <a href="http://flowpaper.com/">The FlowPaper Project</a></h3>

