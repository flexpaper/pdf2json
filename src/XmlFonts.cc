#include "XmlFonts.h"
#include "GlobalParams.h"
#include "UnicodeMap.h"
#include <stdio.h>

#ifndef _WIN32
#include <unistd.h>
#endif

 struct Fonts{
    char *Fontname;
    char *name;
  };

const int font_num=13;

static Fonts fonts[font_num+1]={  
     {"Courier",               "Courier" },
     {"Courier-Bold",           "Courier"},
     {"Courier-BoldOblique",    "Courier"},
     {"Courier-Oblique",        "Courier"},
     {"Helvetica",              "Helvetica"},
     {"Helvetica-Bold",         "Helvetica"},
     {"Helvetica-BoldOblique",  "Helvetica"},
     {"Helvetica-Oblique",      "Helvetica"},
     {"Symbol",                 "Symbol"   },
     {"Times-Bold",             "Times"    },
     {"Times-BoldItalic",       "Times"    },
     {"Times-Italic",           "Times"    },
     {"Times-Roman",            "Times"    },
     {" "          ,            "Times"    },
};

#define xoutRound(x) ((int)(x + 0.5))
extern GBool xml;

GString* XmlFont::DefaultFont=new GString("Times"); // Arial,Helvetica,sans-serif

XmlFontColor::XmlFontColor(GfxRGB rgb){
 /*
  r=static_cast<int>(255*rgb.r);
  g=static_cast<int>(255*rgb.g);
  b=static_cast<int>(255*rgb.b);
*/
  r=colToByte(rgb.r);
  g=colToByte(rgb.g);
  b=colToByte(rgb.b);
  if (!(Ok(r)&&Ok(b)&&Ok(g))) {printf("Error : Bad color \n");r=0;g=0;b=0;}

}

GString *XmlFontColor::convtoX(unsigned int xcol) const{
  GString *xret=new GString();
  char tmp;
  unsigned  int k;
  k = (xcol/16);
  if ((k>=0)&&(k<10)) tmp=(char) ('0'+k); else tmp=(char)('a'+k-10);
  xret->append(tmp);
  k = (xcol%16);
  if ((k>=0)&&(k<10)) tmp=(char) ('0'+k); else tmp=(char)('a'+k-10);
  xret->append(tmp);
 return xret;
}

GString *XmlFontColor::toString() const{
  GString *tmp=new GString("#");
  GString *tmpr=convtoX(r); 
  GString *tmpg=convtoX(g);
  GString *tmpb=convtoX(b);
  tmp->append(tmpr);
  tmp->append(tmpg);
  tmp->append(tmpb);
  delete tmpr;
  delete tmpg;
  delete tmpb;
  return tmp;
} 

XmlFont::XmlFont(GString* ftname,int _size, double _charspace, GfxRGB rgb){
  //if (col) color=XmlFontColor(col); 
  //else color=XmlFontColor();
  color=XmlFontColor(rgb);

  GString *fontname = NULL;
  charspace = _charspace;

  if( ftname ){
    fontname = new GString(ftname);
    FontName=new GString(ftname);
  }
  else {
    fontname = NULL;
    FontName = NULL;
  }
  
  lineSize = -1;

  size=(_size-1);
  italic = gFalse;
  bold = gFalse;
  oblique = gFalse;

  if (fontname){
    if (strstr(fontname->lowerCase()->getCString(),"bold"))  bold=gTrue;    
    if (strstr(fontname->lowerCase()->getCString(),"italic")) italic=gTrue;
    if (strstr(fontname->lowerCase()->getCString(),"oblique")) oblique=gTrue;
    /*||strstr(fontname->lowerCase()->getCString(),"oblique"))  italic=gTrue;*/ 
    
    int i=0;
    while (strcmp(ftname->getCString(),fonts[i].Fontname)&&(i<font_num)) 
	{
		i++;
	}
    pos=i;
    delete fontname;
  }  
  if (!DefaultFont) DefaultFont=new GString(fonts[font_num].name);

}
 
XmlFont::XmlFont(const XmlFont& x){
   size=x.size;
   lineSize=x.lineSize;
   italic=x.italic;
   oblique=x.oblique;
   bold=x.bold;
   pos=x.pos;
   charspace=x.charspace;
   color=x.color;
   if (x.FontName) FontName=new GString(x.FontName);
 }


XmlFont::~XmlFont(){
  if (FontName) delete FontName;
}

XmlFont& XmlFont::operator=(const XmlFont& x){
   if (this==&x) return *this; 
   size=x.size;
   lineSize=x.lineSize;
   italic=x.italic;
   oblique=x.oblique;
   bold=x.bold;
   pos=x.pos;
   color=x.color;
   charspace=x.charspace;
   if (FontName) delete FontName;
   if (x.FontName) FontName=new GString(x.FontName);
   return *this;
}

void XmlFont::clear(){
  if(DefaultFont) delete DefaultFont;
  DefaultFont = NULL;
}



/*
  This function is used to compare font uniquily for insertion into
  the list of all encountered fonts
*/
GBool XmlFont::isEqual(const XmlFont& x) const{
  return ((size==x.size) && (lineSize==x.lineSize) && (charspace==x.charspace) &&
	  (pos==x.pos) && (bold==x.bold) && (oblique==x.oblique) && (italic==x.italic) &&
	  (color.isEqual(x.getColor())));
}

/*
  This one is used to decide whether two pieces of text can be joined together
  and therefore we don't care about bold/italics properties
*/
GBool XmlFont::isEqualIgnoreBold(const XmlFont& x) const{
  return ((size==x.size) &&
	  (color.isEqual(x.getColor())));
}

GString* XmlFont::getFontName(){
   if (pos!=font_num) return new GString(fonts[pos].name);
    else return new GString(DefaultFont);
//    return new GString(FontName);
}

GString* XmlFont::getFullName(){
  if (FontName)
    return new GString(FontName);
  else return new GString(DefaultFont);
} 

void XmlFont::setDefaultFont(GString* defaultFont){
  if (DefaultFont) delete DefaultFont;
  DefaultFont=new GString(defaultFont);
}


GString* XmlFont::getDefaultFont(){
  return DefaultFont;
}

// this method if plain wrong todo
GString* XmlFont::HtmlFilter(Unicode* u, int uLen) {
  GString *tmp = new GString();
  UnicodeMap *uMap;
  char buf[8];
  int n;

  // get the output encoding
  if (!(uMap = globalParams->getTextEncoding())) {
    return tmp;
  }

  for (int i = 0; i < uLen; ++i) {
    switch (u[i])
      { 
	case '"': tmp->append("&quot;");  break;
	case '&': tmp->append("&amp;");  break;
	case '<': tmp->append("&lt;");  break;
	case '>': tmp->append("&gt;");  break;
	default:  
	  {
	    // convert unicode to string
	    if ((n = uMap->mapUnicode(u[i], buf, sizeof(buf))) > 0) {
	      tmp->append(buf, n); 
	  }
      }
    }
  }

  uMap->decRefCnt();
  return tmp;
}

GString* XmlFont::simple(XmlFont* font, Unicode* content, int uLen){
  GString *cont=HtmlFilter (content, uLen); 

  /*if (font.isBold()) {
    cont->insert(0,"<b>",3);
    cont->append("</b>",4);
  }
  if (font.isItalic()) {
    cont->insert(0,"<i>",3);
    cont->append("</i>",4);x.oblique
    } */

  return cont;
}

XmlFontAccu::XmlFontAccu(){
  accu=new GVector<XmlFont>();
}

XmlFontAccu::~XmlFontAccu(){
  if (accu) delete accu;
}

int XmlFontAccu::AddFont(const XmlFont& font){
 GVector<XmlFont>::iterator i; 
 for (i=accu->begin();i!=accu->end();i++)
 {
	if (font.isEqual(*i)) 
	{
		return (int)(i-(accu->begin()));
	}
 }

 accu->push_back(font);
 return (accu->size()-1);
}

static GString* EscapeSpecialChars( GString* s)
{
    GString* tmp = NULL;
    for( int i = 0, j = 0; i < s->getLength(); i++, j++ ){
        const char *replace = NULL;
            switch ( s->getChar(i) ){
                case '"': replace = "\\u0022";  break;
                case '\\': replace = "\\u005C";  break;
                case '<': replace = "\\u003C";  break;
                case '>': replace = "\\u003E";  break;
                case '\'': replace = "\\u0027";  break;
                case '&': replace = "\\u0026";  break;
                case 0x01: replace = "";  break;
                case 0x02: replace = "";  break;                
                case 0x03: replace = "";  break;                
                case 0x04: replace = "";  break;                
                case 0x05: replace = "";  break;                
                case 0x06: replace = "";  break;                                
                case 0x07: replace = "";  break;
                case 0x08: replace = "";  break;                
                case 0x09: replace = "";  break;                
                case 0x0a: replace = "";  break;                
                case 0x0b: replace = "";  break;                
                case 0x0c: replace = "";  break;                
                case 0x0d: replace = "";  break;                
                case 0x0e: replace = "";  break;                
                case 0x0f: replace = "";  break;                
                case 0x10: replace = "";  break;                                
                case 0x11: replace = "";  break;                                                
                case 0x12: replace = "";  break;                                                
                case 0x13: replace = "";  break;                                                
                case 0x14: replace = "";  break;                                                
                case 0x15: replace = "";  break;                                                
                case 0x16: replace = "";  break;                                                
                case 0x17: replace = "";  break;                                                
                case 0x18: replace = "";  break;                                                
                case 0x19: replace = "";  break;  
                case 0x1a: replace = "";  break;                                                                
                case 0x1b: replace = "";  break;                  
                case 0x1c: replace = "";  break;                  
                case 0x1d: replace = "";  break;                  
                case 0x1e: replace = "";  break;                  
                case 0x1f: replace = "";  break;                  
                default: continue;
            }    
	    if( replace ){
	        if( !tmp ) tmp = new GString( s );
	        if( tmp ){
	            tmp->del( j, 1 );
	            int l = strlen( replace );
	            tmp->insert( j, replace, l );
	            j += l - 1;
	        }
	    }
	}
	return tmp ? tmp : s;
}

// get CSS font name for font #i 
GString* XmlFontAccu::getCSStyle(int i, GString* content){
  GString *tmp;
  GString *iStr=GString::fromInt(i);
  
  if (!xml) {
    tmp = new GString("<span class=\"ft");
    tmp->append(iStr);
    tmp->append("\">");
    tmp->append(content);
    tmp->append("</span>");
  } else {
    tmp = EscapeSpecialChars(content);
    //tmp->append(content);
  } 

  delete iStr;
  return tmp;
}


// get CSS font definition for font #i 
GString* XmlFontAccu::CSStyle(int i,GBool textAsJSON){
   GString *tmp=new GString();
   GString *iStr=GString::fromInt(i);

   GVector<XmlFont>::iterator g=accu->begin();
   g+=i;
   XmlFont font=*g;
   GString *Size=GString::fromInt(font.getSize());
   GString *colorStr=font.getColor().toString();
   GString *fontName=font.getFontName();
   GString *lSize;
   double _charspace = font.getCharSpace();
   char *cspace = new char [20];
   sprintf(cspace,"%0.05f",_charspace);
   
   if (xml && !textAsJSON) {
     tmp->append("<fontspec id=\"");
     tmp->append(iStr);
     tmp->append("\" size=\"");
     tmp->append(Size);
     tmp->append("\" family=\"");
     tmp->append(fontName); //font.getFontName());
     tmp->append("\" color=\"");
     tmp->append(colorStr);
     tmp->append("\"/>");
   }else if(textAsJSON){
     tmp->append("{\"fontspec\":\"");
     tmp->append(iStr);
     tmp->append("\",\"size\":\"");
     tmp->append(Size);
     tmp->append("\",\"family\":\"");
     tmp->append(fontName); //font.getFontName());
     tmp->append("\",\"color\":\"");
     tmp->append(colorStr);
     tmp->append("\"}");
   }

   delete fontName;
   delete colorStr;
   delete iStr;
   delete Size;
   delete cspace;
   return tmp;
}
 

