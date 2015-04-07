#ifndef _XML_FONTS_H
#define _XML_FONTS_H
#include "GVector.h"
#include "GString.h"
#include "GfxState.h"
#include "CharTypes.h"


class XmlFontColor{
 private:
   unsigned int r;
   unsigned int g;
   unsigned int b;
   GBool Ok(unsigned int xcol){ return ((xcol<=255)&&(xcol>=0));}
   GString *convtoX(unsigned  int xcol) const;
 public:
   XmlFontColor():r(0),g(0),b(0){}
   XmlFontColor(GfxRGB rgb);
   XmlFontColor(const XmlFontColor& x){r=x.r;g=x.g;b=x.b;}
   XmlFontColor& operator=(const XmlFontColor &x){
     r=x.r;g=x.g;b=x.b;
     return *this;
   }
   ~XmlFontColor(){};
   GString* toString() const;
   GBool isEqual(const XmlFontColor& col) const{
     return ((r==col.r)&&(g==col.g)&&(b==col.b));
   }
} ;  


class XmlFont{
 private:
   unsigned int size;
   double charspace;
   int lineSize;
   GBool italic;
   GBool bold;
   GBool oblique;
   int pos; // position of the font name in the fonts array
   static GString *DefaultFont;
   GString *FontName;
   XmlFontColor color;
   static GString* HtmlFilter(Unicode* u, int uLen); //char* s);
public:  

   XmlFont(){FontName=NULL;};
   XmlFont(GString* fontname,int _size,double _charspace, GfxRGB rgb);
   XmlFont(const XmlFont& x);
   XmlFont& operator=(const XmlFont& x);
   XmlFontColor getColor() const {return color;}
   ~XmlFont();
   static void clear();
   GString* getFullName();
   GBool isItalic() const {return italic;}
   GBool isBold() const {return bold;}
   GBool isOblique() const {return oblique;}
   unsigned int getSize() const {return size;}
   int getLineSize() const {return lineSize;}
   void setLineSize(int _lineSize) { lineSize = _lineSize; }
   GString* getFontName();
   double getCharSpace() const {return charspace;}
   void setCharSpace(double _charspace){charspace = _charspace;}
   static GString* getDefaultFont();
   static void setDefaultFont(GString* defaultFont);
   GBool isEqual(const XmlFont& x) const;
   GBool isEqualIgnoreBold(const XmlFont& x) const;
   static GString* simple(XmlFont *font, Unicode *content, int uLen);
   void print() const {printf("font: %s %d %s%spos: %d\n", FontName->getCString(), size, bold ? "bold " : "", italic ? "italic " : "", pos);};
};

class XmlFontAccu{
private:
  GVector<XmlFont> *accu;
  
public:
  XmlFontAccu();
  ~XmlFontAccu();
  int AddFont(const XmlFont& font);
  XmlFont* Get(int i){
    GVector<XmlFont>::iterator g=accu->begin();
    g+=i;  
    return g;
  } 
  GString* getCSStyle (int i, GString* content);
 /* GString* EscapeSpecialChars(GString* content); */
  GString* CSStyle(int i,GBool textAsJSON);
  int size() const {return accu->size();}
  
};  
#endif
