//========================================================================
//
// ImgOutputDev.cc
//
// Copyright 2011 Devaldi Ltd
//
// Copyright 1997-2002 Glyph & Cog, LLC
//
// Changed 1999-2000 by G.Ovtcharov
//
// Changed 2002 by Mikhail Kruk
//
//========================================================================

#ifdef __GNUC__
#pragma implementation
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stddef.h>
#include <ctype.h>
#include <math.h>
#include "GString.h"
#include "GList.h"
#include "UnicodeMap.h"
#include "gmem.h"
#include "config.h"
#include "Error.h"
#include "GfxState.h"
#include "GlobalParams.h"
#include "ImgOutputDev.h"
#include "XmlFonts.h"


int HtmlPage::pgNum=0;
int ImgOutputDev::imgNum=1;

extern double scale;
extern GBool complexMode;
extern GBool ignore;
extern GBool printCommands;
extern GBool printHtml;
extern GBool noframes;
extern GBool stout;
extern GBool xml;
extern GBool showHidden;
extern GBool noMerge;

static GString* basename(GString* str){
  
  char *p=str->getCString();
  int len=str->getLength();
  for (int i=len-1;i>=0;i--)
    if (*(p+i)==SLASH) 
      return new GString((p+i+1),len-i-1);
  return new GString(str);
}

static GString* Dirname(GString* str){
  
  char *p=str->getCString();
  int len=str->getLength();
  for (int i=len-1;i>=0;i--)
    if (*(p+i)==SLASH) 
      return new GString(p,i+1);
  return new GString();
} 

//------------------------------------------------------------------------
// HtmlString
//------------------------------------------------------------------------

HtmlString::HtmlString(GfxState *state, double fontSize, double _charspace, XmlFontAccu* fonts) {
  GfxFont *font;
  double x, y;

  state->transform(state->getCurX(), state->getCurY(), &x, &y);
  if ((font = state->getFont())) {
    yMin = y - font->getAscent() * fontSize;
    yMax = y - font->getDescent() * fontSize;
    GfxRGB rgb;
    state->getFillRGB(&rgb);
    GString *name = state->getFont()->getName();
    if (!name) name = XmlFont::getDefaultFont(); //new GString("default");
    //XmlFont hfont=XmlFont(name, static_cast<int>(fontSize-1),_charspace, rgb);
    XmlFont hfont=XmlFont(name, static_cast<int>(fontSize-1),0.0, rgb);
    fontpos = fonts->AddFont(hfont);
  } else {
    // this means that the PDF file draws text without a current font,
    // which should never happen
    yMin = y - 0.95 * fontSize;
    yMax = y + 0.35 * fontSize;
    fontpos=0;
  }
  if (yMin == yMax) {
    // this is a sanity check for a case that shouldn't happen -- but
    // if it does happen, we want to avoid dividing by zero later
    yMin = y;
    yMax = y + 1;
  }
  col = 0;
  text = NULL;
  xRight = NULL;
  link = NULL;
  len = size = 0;
  yxNext = NULL;
  xyNext = NULL;
  strSize = 0;
  htext=new GString();
  htext2=new GString();
  dir = textDirUnknown;
}


HtmlString::~HtmlString() {
  delete text;
  delete htext;
  delete htext2;
//  delete strSize;
  gfree(xRight);
}

void HtmlString::addChar(GfxState *state, double x, double y,
			 double dx, double dy, Unicode u) {
    
  if ( !showHidden && (state->getRender() & 3) == 3) {
    return;
  }
    
  if (dir == textDirUnknown) {
    dir = UnicodeMap::getDirection(u);
  } 

  if (len == size) {
    size += 16;
    text = (Unicode *)grealloc(text, size * sizeof(Unicode));
    xRight = (double *)grealloc(xRight, size * sizeof(double));
  }
  text[len] = u;
  if (len == 0) {
    xMin = x;
  }
  xMax = xRight[len] = x + dx;
  //xMax = xRight[len] = x;
  ++strSize;
//printf("added char: %f %f xright = %f\n", x, dx, x+dx);
  ++len;
}

void HtmlString::endString()
{
  if( dir == textDirRightLeft && len > 1 )
  {
    //printf("will reverse!\n");
    for (int i = 0; i < len / 2; i++)
    {
      Unicode ch = text[i];
      text[i] = text[len - i - 1];
      text[len - i - 1] = ch;
    }
  }
}

//------------------------------------------------------------------------
// HtmlPage
//------------------------------------------------------------------------

HtmlPage::HtmlPage(GBool rawOrder, GBool textAsJSON, GBool compressData, char *imgExtVal) {
  this->rawOrder = rawOrder;
  this->textAsJSON = textAsJSON;
  this->compressData = compressData;
  curStr = NULL;
  yxStrings = NULL;
  xyStrings = NULL;
  yxCur1 = yxCur2 = NULL;
  fonts=new XmlFontAccu();
  links=new XmlLinks();
  pageWidth=0;
  pageHeight=0;
  X1=0;
  X2=0;  
  Y1=0;
  Y2=0;  
  fontsPageMarker = 0;
  DocName=NULL;
  firstPage = -1;
  imgExt = new GString(imgExtVal);
}

HtmlPage::~HtmlPage() {
  clear();
  if (DocName) delete DocName;
  if (fonts) delete fonts;
  if (links) delete links;
  if (imgExt) delete imgExt;  
}

void HtmlPage::updateFont(GfxState *state) {
  GfxFont *font;
  double *fm;
  char *name;
  int code;
  double w;
  
  // adjust the font size
  fontSize = state->getTransformedFontSize();
  if ((font = state->getFont()) && font->getType() == fontType3) {
    // This is a hack which makes it possible to deal with some Type 3
    // fonts.  The problem is that it's impossible to know what the
    // base coordinate system used in the font is without actually
    // rendering the font.  This code tries to guess by looking at the
    // width of the character 'm' (which breaks if the font is a
    // subset that doesn't contain 'm').
    for (code = 0; code < 256; ++code) {
      if ((name = ((Gfx8BitFont *)font)->getCharName(code)) &&
	  name[0] == 'm' && name[1] == '\0') {
	break;
      }
    }
    if (code < 256) {
      w = ((Gfx8BitFont *)font)->getWidth(code);
      if (w != 0) {
	// 600 is a generic average 'm' width -- yes, this is a hack
	fontSize *= w / 0.6;
      }
    }
    fm = font->getFontMatrix();
    if (fm[0] != 0) {
      fontSize *= fabs(fm[3] / fm[0]);
    }
  }
}

void HtmlPage::beginString(GfxState *state, GString *s) {
  curStr = new HtmlString(state, fontSize,charspace, fonts);
}


void HtmlPage::conv(){
  HtmlString *tmp;

  int linkIndex = 0;
  XmlFont* h;

  for(tmp=yxStrings;tmp;tmp=tmp->yxNext){
     int pos=tmp->fontpos;
     //  printf("%d\n",pos);
     h=fonts->Get(pos);

     if (tmp->htext) delete tmp->htext; 
     tmp->htext=XmlFont::simple(h,tmp->text,tmp->len);
     tmp->htext2=XmlFont::simple(h,tmp->text,tmp->len);

     if (links->inLink(tmp->xMin,tmp->yMin,tmp->xMax,tmp->yMax, linkIndex)){
       tmp->link = links->getLink(linkIndex);
       /*GString *t=tmp->htext;
       tmp->htext=links->getLink(k)->Link(tmp->htext);
       delete t;*/
     }
  }

}


void HtmlPage::addChar(GfxState *state, double x, double y,
		       double dx, double dy, 
			double ox, double oy, Unicode *u, int uLen) {

  if ( !showHidden && (state->getRender() & 3) == 3) {
      return;
  }
    
  double x1, y1, w1, h1, dx2, dy2;
  int n, i, d;
  state->transform(x, y, &x1, &y1);
  n = curStr->len;
  d = 0;
 
  // check that new character is in the same direction as current string
  // and is not too far away from it before adding 
/*  if ((UnicodeMap::getDirection(u[0]) != curStr->dir) || 
     (n > 0 && 
      fabs(x1 - curStr->xRight[n-1]) > 0.1 * (curStr->yMax - curStr->yMin))) {
    endString();
    beginString(state, NULL);
  }*/
  state->textTransformDelta(state->getCharSpace() * state->getHorizScaling(),
			    0, &dx2, &dy2);
  dx -= dx2;
  dy -= dy2;
  state->transformDelta(dx, dy, &w1, &h1);
  if (uLen != 0) {
    w1 /= uLen;
    h1 /= uLen;
  }
/* if (d != 3)
 {
 endString();
 beginString(state, NULL);
 }
*/


  for (i = 0; i < uLen; ++i) 
  {
	if (u[i] == ' ')
        {
	    curStr->addChar(state, x1 + i*w1, y1 + i*h1, w1, h1, u[i]);
	    endString();
	    beginString(state, NULL);
	}
        else {
	    curStr->addChar(state, x1 + i*w1, y1 + i*h1, w1, h1, u[i]); /* xyString  */
        }
   }

/*
  for (i = 0; i < uLen; ++i) {
    curStr->addChar(state, x1 + i*w1, y1 + i*h1, w1, h1, u[i]);
  }
*/
}

void HtmlPage::endString() {
  HtmlString *p1, *p2;
  double h, y1, y2;

  // throw away zero-length strings -- they don't have valid xMin/xMax
  // values, and they're useless anyway
    
   if (curStr->len == 0) {
    delete curStr;
     curStr = NULL;
    return;
   }

  curStr->endString();

#if 0 //~tmp
  if (curStr->yMax - curStr->yMin > 20) {
    delete curStr;
    curStr = NULL;
    return;
  }
#endif

  // insert string in y-major list
  h = curStr->yMax - curStr->yMin;
  y1 = curStr->yMin + 0.5 * h;
  y2 = curStr->yMin + 0.8 * h;
  
  if (rawOrder) {
    p1 = yxCur1;
    p2 = NULL;
  } else if ((!yxCur1 ||
              (y1 >= yxCur1->yMin &&
               (y2 >= yxCur1->yMax || curStr->xMax >= yxCur1->xMin))) &&
             (!yxCur2 ||
              (y1 < yxCur2->yMin ||
               (y2 < yxCur2->yMax && curStr->xMax < yxCur2->xMin)))) { /* add space */
    p1 = yxCur1;
    p2 = yxCur2;
  } else {
    for (p1 = NULL, p2 = yxStrings; p2; p1 = p2, p2 = p2->yxNext) {
      if (y1 < p2->yMin || (y2 < p2->yMax && curStr->xMax < p2->xMin))
        break;
    }
    yxCur2 = p2;
  }

  yxCur1 = curStr;

  if (p1)
    p1->yxNext = curStr;
  else
    yxStrings = curStr;

  curStr->yxNext = p2;
  curStr = NULL;
}

void HtmlPage::coalesce() {
  HtmlString *str1, *str2;
  XmlFont *hfont1, *hfont2;
  double space, horSpace, vertSpace, vertOverlap;
  GBool addSpace, addLineBreak;
  int n, i;
  double curX, curY, lastX, lastY;
  int sSize = 0;      
  double diff = 0.0;
  double pxSize = 0.0;
  double strSize = 0.0;
  double cspace = 0.0;

#if 0 //~ for debugging
  for (str1 = yxStrings; str1; str1 = str1->yxNext) {
    printf("x=%f..%f  y=%f..%f  size=%2d '",
	   str1->xMin, str1->xMax, str1->yMin, str1->yMax,
	   (int)(str1->yMax - str1->yMin));
    for (i = 0; i < str1->len; ++i) {
      fputc(str1->text[i] & 0xff, stdout);
    }
    printf("'\n");
  }
  printf("\n------------------------------------------------------------\n\n");
#endif
    
    
  str1 = yxStrings;
  if( !str1 ) return;
  
  hfont1 = getFont(str1);

  str1->htext2->append(str1->htext);
  if( str1->getLink() != NULL ) {
    GString *ls = str1->getLink()->getLinkStart();
    str1->htext->insert(0, ls);
    delete ls;
  }

  curX = str1->xMin; curY = str1->yMin;
  lastX = str1->xMin; lastY = str1->yMin;

  while (str1 && (str2 = str1->yxNext)) {
    hfont2 = getFont(str2);
    space = str1->yMax - str1->yMin;
    horSpace = str2->xMin - str1->xMax;
    addLineBreak = !noMerge && (fabs(str1->xMin - str2->xMin) < 0.4);
    vertSpace = str2->yMin - str1->yMax;

    //printf("coalesce %d %d %f? ", str1->dir, str2->dir, d);

    if (str2->yMin >= str1->yMin && str2->yMin <= str1->yMax)
    {
        vertOverlap = str1->yMax - str2->yMin;
    } else
        if (str2->yMax >= str1->yMin && str2->yMax <= str1->yMax)
        {
            vertOverlap = str2->yMax - str1->yMin;
        } else
        {
            vertOverlap = 0;
        } 
    
    // str1->dir == str2->dir, in complex mode fonts must be the same, in other modes fonts do not metter
    if ((((
           (rawOrder && vertOverlap > 0.5 * space)
           ||
           (!rawOrder && str2->yMin < str1->yMax)
           ) &&
            (horSpace > -0.5 * space && horSpace < space)
           ) ||
            (vertSpace >= 0 && vertSpace < 0.5 * space && addLineBreak)
           ) 
        &&
        str1->dir == str2->dir // text direction the same
        && 
        !(str2->len == 1 && str2->htext->getCString()[0] == ' ')
        &&
        !(str1->htext->getCString()[str1->len-1] == ' ')
        &&
        !(str1->htext->getLength() >= str1->len+1 && str1->htext->getCString()[str1->len+1] == ' ')
       ) 
    {
        diff = str2->xMax - str1->xMin;

        n = str1->len + str2->len;

        
        if ((addSpace = horSpace > 0.1 * space)) {
            ++n;
        }
         
              
        str1->size = (n + 15) & ~15;
        str1->text = (Unicode *)grealloc(str1->text,
				       str1->size * sizeof(Unicode));   
        str1->xRight = (double *)grealloc(str1->xRight,
					str1->size * sizeof(double));
        
        if (addSpace) {
            str1->text[str1->len] = 0x20;
            str1->htext->append(" ");
            str1->htext2->append(" ");
            str1->xRight[str1->len] = str2->xMin;
            ++str1->len;
            ++str1->strSize;
            
            str1->xMin = curX; str1->yMin = curY; 
	        str1 = str2;
	        curX = str1->xMin; curY = str1->yMin;
	        hfont1 = hfont2;
	
	        if( str1->getLink() != NULL ) {
	            GString *ls = str1->getLink()->getLinkStart();
	            str1->htext->insert(0, ls);
	            delete ls;
	        }
        }else{
	        str1->htext2->append(str2->htext2);
	
	        XmlLink *hlink1 = str1->getLink();
	        XmlLink *hlink2 = str2->getLink();
	
	        for (i = 0; i < str2->len; ++i) {
	            str1->text[str1->len] = str2->text[i];
	            str1->xRight[str1->len] = str2->xRight[i];
	            ++str1->len;
	        }
	
	        if( !hlink1 || !hlink2 || !hlink1->isEqualDest(*hlink2) ) {
	            
	            if(hlink1 != NULL ){
	                //str1->htext->append("\"]");
	            }
	            if(hlink2 != NULL ) {
	                GString *ls = hlink2->getLinkStart();
	                str1->htext->append(ls);
	                delete ls;
	            }
	        }
	
	        str1->htext->append(str2->htext);
	        sSize = str1->htext2->getLength();      
	        pxSize = xoutRoundLower(hfont1->getSize()/scale);
	        strSize = (pxSize*(sSize-2));   
	        cspace = (diff / strSize);//(strSize-pxSize));
	        // we check if the fonts are the same and create a new font to ajust the text
	        //      double diff = str2->xMin - str1->xMin;
	        //      printf("%s\n",str1->htext2->getCString());
	        // str1 now contains href for link of str2 (if it is defined)
	        str1->link = str2->link; 
	
	        //XmlFont *newfnt = new XmlFont(*hfont1);
	        //newfnt->setCharSpace(cspace);
	        //newfnt->setLineSize(curLineSize);
	        //str1->fontpos = fonts->AddFont(*newfnt);
	        //delete newfnt;
	        hfont1 = getFont(str1);
	        // we have to reget hfont2 because it's location could have
	        // changed on resize  GStri;ng *iStr=GString::fromInt(i);
	        hfont2 = getFont(str2); 
	
	        hfont1 = hfont2;
	
	        if (str2->xMax > str1->xMax) {
	            str1->xMax = str2->xMax;
	        }
	        
	        if (str2->yMax > str1->yMax) {
	            str1->yMax = str2->yMax;
	        }
	
	        str1->yxNext = str2->yxNext;
	
	        delete str2;
        }
    } else { 

        //printf("startX = %f, endX = %f, diff = %f, fontsize = %d, pxSize = %f, stringSize = %d, cspace = %f, strSize = %f\n",str1->xMin,str1->xMax,diff,hfont1->getSize(),pxSize,sSize,cspace,strSize);

        // keep strings separate
        //      printf("no\n"); 
        //      if( hfont1->isBold() )
//        if(str1->getLink() != NULL )
  //          str1->htext->append("\"]");  
     
        str1->xMin = curX; str1->yMin = curY; 
        str1 = str2;
        curX = str1->xMin; curY = str1->yMin;
        hfont1 = hfont2;

        if( str1->getLink() != NULL ) {
            GString *ls = str1->getLink()->getLinkStart();
            str1->htext->insert(0, ls);
            delete ls;
        }
    }
  }
  str1->xMin = curX; str1->yMin = curY;

//  if(str1->getLink() != NULL )
  //  str1->htext->append("]");

#if 0 //~ for debugging
  for (str1 = yxStrings; str1; str1 = str1->yxNext) {
    printf("x=%3d..%3d  y=%3d..%3d  size=%2d ",
	   (int)str1->xMin, (int)str1->xMax, (int)str1->yMin, (int)str1->yMax,
	   (int)(str1->yMax - str1->yMin));
    printf("'%s'\n", str1->htext->getCString());  
  }
  printf("\n-end--------------------------------------------------------\n\n");
#endif

}


void HtmlPage::dumpAsXML(FILE* f,int page, GBool passedFirstPage, int totalPages){  
//    printf(this->numPages);
    printf("");
  if(textAsJSON){
    if(passedFirstPage){
	fprintf(f, ",");
    }
    fprintf(f, "{\"number\":%d,\"pages\":%d,\"height\":%d,\"width\":%d,", page,totalPages,pageHeight,pageWidth);
  }else{ 
    fprintf(f, "<page number=\"%d\" pages=\"%d\"", page,totalPages);
    fprintf(f," height=\"%d\" width=\"%d\">", pageHeight,pageWidth);
  }

  GBool passedFirst = false;

  // output fonts
  if(textAsJSON){fprintf(f,"\"fonts\":[");}
  for(int i=fontsPageMarker;i < fonts->size();i++) {
    GString *fontCSStyle = fonts->CSStyle(i,textAsJSON);
    if(textAsJSON && passedFirst) {fprintf(f,",");}
    fprintf(f,"%s",fontCSStyle->getCString());
    passedFirst = true;
    delete fontCSStyle;
  }
  if(textAsJSON){fprintf(f,"]");}
  
  GString *str, *str1;
  
  passedFirst = false;
  if(textAsJSON){
     fprintf(f,",\"text\":[");
      //fprintf(f,"\"text\":[");
  }

  for(HtmlString *tmp=yxStrings;tmp;tmp=tmp->yxNext){
    if (tmp->htext){
      str=new GString(tmp->htext);
      
      if(!compressData){
	      if(textAsJSON){      
              if(passedFirst){
                  fprintf(f,",");
              }
              fprintf(f,"{\"top\":%d,\"left\":%d,",xoutRound(tmp->yMin+this->movey),xoutRound(tmp->xMin+this->movex));	
              fprintf(f,"\"width\":%d,\"height\":%d,",xoutRound(tmp->xMax-tmp->xMin),xoutRound(tmp->yMax-tmp->yMin));
              fprintf(f,"\"font\":%d,\"data\":\"", tmp->fontpos);
              if (tmp->fontpos!=-1){
                  str1=fonts->getCSStyle(tmp->fontpos, str);
              }
              fputs(str1->getCString(),f);
              fprintf(f,"\"}");
              //fprintf(f,"\"}");
              passedFirst = true;
	      }else{
              fprintf(f,"<text top=\"%d\" left=\"%d\" ",xoutRound(tmp->yMin+this->movey),xoutRound(tmp->xMin+this->movey));
              fprintf(f,"width=\"%d\" height=\"%d\" ",xoutRound(tmp->xMax-tmp->xMin),xoutRound(tmp->yMax-tmp->yMin));
              fprintf(f,"font=\"%d\">", tmp->fontpos);
              if (tmp->fontpos!=-1){
                  str1=fonts->getCSStyle(tmp->fontpos, str);
              }
              fputs(str1->getCString(),f);
              delete str;
              delete str1;
              fputs("</text>\n",f);
	      }
      }else{
		if(textAsJSON){      
            if(passedFirst){
              fprintf(f,",");
            }
          
            //fprintf(f,"{\"t\":%d,\"l\":%d,",xoutRound(tmp->yMin),xoutRound(tmp->xMin));	
            //fprintf(f,"\"w\":%d,\"h\":%d,",xoutRound(tmp->xMax-tmp->xMin),xoutRound(tmp->yMax-tmp->yMin));
            //fprintf(f,"\"f\":%d,\"d\":\"", tmp->fontpos);
            fprintf(f,"[%d,%d,",xoutRound(tmp->yMin+this->movey),xoutRound(tmp->xMin+this->movex));	
            fprintf(f,"%d,%d,",xoutRound(tmp->xMax-tmp->xMin),xoutRound(tmp->yMax-tmp->yMin));
            fprintf(f,"%d,\"", tmp->fontpos);
		  
            if (tmp->fontpos!=-1){
                str1=fonts->getCSStyle(tmp->fontpos, str);
            }
            fputs(str1->getCString(),f);
            fprintf(f,"\"]");
            
            passedFirst = true;
        }else{
              fprintf(f,"<t t=\"%d\" l=\"%d\" ",xoutRound(tmp->yMin+this->movey),xoutRound(tmp->xMin+this->movex));
              fprintf(f,"w=\"%d\" h=\"%d\" ",xoutRound(tmp->xMax-tmp->xMin),xoutRound(tmp->yMax-tmp->yMin));
              fprintf(f,"f=\"%d\">", tmp->fontpos);
              if (tmp->fontpos!=-1){
                  str1=fonts->getCSStyle(tmp->fontpos, str);
              }
              fputs(str1->getCString(),f);
              delete str;
              delete str1;
              fputs("</t>\n",f);
	      }
      }
    }
  }

  if(textAsJSON){
  	fputs("]}",f);
  }else{
  	fputs("</page>",f);
  }
}

void HtmlPage::dump(FILE *f, int pageNum, GBool passedFirstPage, int totalPages) 
{
  if (complexMode)
  {
    if (xml) dumpAsXML(f, pageNum, passedFirstPage, totalPages);
  }
}

void HtmlPage::clear() {
  HtmlString *p1, *p2;

  if (curStr) {
    delete curStr;
    curStr = NULL;
  }
  for (p1 = yxStrings; p1; p1 = p2) {
    p2 = p1->yxNext;
    delete p1;
  }
  yxStrings = NULL;
  xyStrings = NULL;
  yxCur1 = yxCur2 = NULL;

  if( !noframes )
  {
      delete fonts;
      fonts=new XmlFontAccu();
      fontsPageMarker = 0;
  }
  else
  {
      fontsPageMarker = fonts->size();
  }

  delete links;
  links=new XmlLinks();
 

}

void HtmlPage::setDocName(char *fname){
  DocName=new GString(fname);
}

void HtmlPage::updateCharSpace(GfxState *state)
{
	charspace = state->getCharSpace();
}

//------------------------------------------------------------------------
// HtmlMetaVar
//------------------------------------------------------------------------

HtmlMetaVar::HtmlMetaVar(char *_name, char *_content)
{
    name = new GString(_name);
    content = new GString(_content);
}

HtmlMetaVar::~HtmlMetaVar()
{
   delete name;
   delete content;
} 
    
GString* HtmlMetaVar::toString()	
{
    GString *result = new GString("<META name=\"");
    result->append(name);
    result->append("\" content=\"");
    result->append(content);
    result->append("\">"); 
    return result;
}

//------------------------------------------------------------------------
// ImgOutputDev
//------------------------------------------------------------------------

static char* HtmlEncodings[][2] = {
    {"Latin1", "ISO-8859-1"},
    {NULL, NULL}
};


char* ImgOutputDev::mapEncodingToHtml(GString* encoding)
{
    char* enc = encoding->getCString();
    for(int i = 0; HtmlEncodings[i][0] != NULL; i++)
    {
	if( strcmp(enc, HtmlEncodings[i][0]) == 0 )
	{
	    return HtmlEncodings[i][1];
	}
    }
    return enc; 
}

ImgOutputDev::ImgOutputDev(char *fileName, char *title, 
	char *author, char *keywords, char *subject, char *date,
	char *extension,
	GBool rawOrder, GBool textAsJSON, GBool compressData, int split, int firstPage, GBool outline, int numPages) 
{
  char *htmlEncoding;
  this->numPages = numPages;
  fContentsFrame = NULL;
  docTitle = new GString(title);
  pages = NULL;
  dumpJPEG=gTrue;
  //write = gTrue;
  this->rawOrder = rawOrder;
  this->textAsJSON = textAsJSON;
  this->compressData = compressData;
  this->split = split;  
  this->doOutline = outline;
  ok = gFalse;
  passedFirstPage = gFalse;
  imgNum=1;
  //this->firstPage = firstPage;
  //pageNum=firstPage;
  // open file
  needClose = gFalse;
  pages = new HtmlPage(rawOrder, textAsJSON, compressData, extension);
  
  glMetaVars = new GList();
  glMetaVars->append(new HtmlMetaVar("generator", "pdf2json 0.68"));  
  if( author ) glMetaVars->append(new HtmlMetaVar("author", author));  
  if( keywords ) glMetaVars->append(new HtmlMetaVar("keywords", keywords));  
  if( date ) glMetaVars->append(new HtmlMetaVar("date", date));  
  if( subject ) glMetaVars->append(new HtmlMetaVar("subject", subject));
 
  maxPageWidth = 0;
  maxPageHeight = 0;

  pages->setDocName(fileName);
  Docname=new GString (fileName);

    
  if (noframes) {
    if (stout) page=stdout;
    else {
      GString* right=new GString(fileName);
      //if (xml && !textAsJSON) right->append(".xml");
      //else if (textAsJSON) right->append(".js");
      
      if(this->split>0 && this->hasValidSplitFileName()){
        this->setSplitFileName(10,false);
      }else if (!(page=fopen(right->getCString(),"w"))){
          delete right;
          error(-1, "Couldn't open html file '%s'", right->getCString());
          return;
      } 
        
      delete right;
    }

    htmlEncoding = mapEncodingToHtml(globalParams->getTextEncodingName()); 
    if (xml && !textAsJSON) 
    {
      fprintf(page, "<?xml version=\"1.0\" encoding=\"%s\"?>\n", htmlEncoding);
      fputs("<!DOCTYPE pdf2xml SYSTEM \"pdf2xml.dtd\">\n\n", page);
      fputs("<pdf2xml>\n",page);
    }else if(textAsJSON){
      fputs("[",page);
    } 
  }
  ok = gTrue; 
}

ImgOutputDev::~ImgOutputDev() {
    XmlFont::clear(); 
    
    delete Docname;
    delete docTitle;

    deleteGList(glMetaVars, HtmlMetaVar);

    if (xml && !textAsJSON) {
      fputs("</pdf2xml>\n",page);  
      fclose(page);
    } else if(textAsJSON){
      fputs("]",page);
      fclose(page);
    } 
    if (pages)
      delete pages;
}



void ImgOutputDev::startPage(int pageNum, GfxState *state,double crop_x1, double crop_y1, double crop_x2, double crop_y2) {
    double x1,y1,x2,y2;
    state->transform(crop_x1,crop_y1,&x1,&y1);
    state->transform(crop_x2,crop_y2,&x2,&y2);
    if(x2<x1) {double x3=x1;x1=x2;x2=x3;}
    if(y2<y1) {double y3=y1;y1=y2;y2=y3;}
    
    pages->movex = -(int)x1;
    pages->movey = -(int)y1; 
    
  this->pageNum = pageNum;
  GString *str=basename(Docname);
  pages->clear(); 
  if(!noframes)
  {
    if (fContentsFrame)
	{
      if (complexMode)
		fprintf(fContentsFrame,"<A href=\"%s-%d.html\"",str->getCString(),pageNum);
      else 
		fprintf(fContentsFrame,"<A href=\"%ss.html#%d\"",str->getCString(),pageNum);
      fprintf(fContentsFrame," target=\"contents\" >Page %d</a><br>\n",pageNum);
    }
  }

//  pages->pageWidth=static_cast<int>(state->getPageWidth());
  //pages->pageHeight=static_cast<int>(state->getPageHeight());
    pages->pageWidth = (int)(x2-x1);
    pages->pageHeight = (int)(y2-y1);
    
    
  delete str;
} 

GBool ImgOutputDev::hasValidSplitFileName() {
    for( int i = 0, j = 0; i < Docname->getLength(); i++, j++ ){
        if(Docname->getChar(i) == '\%'){
            return gTrue;
        }
    }
    
    return gFalse;
}

void ImgOutputDev::setSplitFileName(int pageNum, GBool closeprev) {
    GString* tmp = NULL;
    
    char *pn, ptext[32];
    
    for( int i = 0, j = 0; i < Docname->getLength(); i++, j++ ){
        const char *replace = NULL;
        switch ( Docname->getChar(i) ){
            case '\%': 
                sprintf(ptext,"%d",pageNum);
                pn = ptext;
                
                replace = pn;  
                break;
        }
        
        if( replace ){
            if( !tmp ) tmp = new GString( Docname );
            if( tmp ){
                tmp->del( j, 1 );
                int l = strlen( replace );
                tmp->insert( j, replace, l );
                j += l - 1;
            }
        }
        
    }
    
    if(closeprev)
        fclose(page);
    
    page=fopen(tmp->getCString(),"w");
}

void ImgOutputDev::endPage() {
  pages->conv();
  pages->coalesce();

  // reassign page if running split mode ever nth page  
  /*
   if(split>0){
   int p;
   for(p = 0; p < split; p++)
   {
   GString* tmp = NULL;
   
   char *pn, ptext[32];
   
   for( int i = 0, j = 0; i < Docname->getLength(); i++, j++ ){
   const char *replace = NULL;
   switch ( Docname->getChar(i) ){
   case '\%': 
   sprintf(ptext,"%d",p);
   pn = ptext;
   
   replace = pn;  
   break;
   }
   
   if( replace ){
   if( !tmp ) tmp = new GString( Docname );
   if( tmp ){
   tmp->del( j, 1 );
   int l = strlen( replace );
   tmp->insert( j, replace, l );
   j += l - 1;
   }
   }
   
   }
   
   printf("PROCESS: %s",tmp->getCString());
   }        
   }
   */
    
  //printf("doc: %s",Docname->getCString());
  //printf("split: %d", this->split);  
  
  // reassign page based on split if split is higher than 1
  if(this->split>0 && (pageNum % this->split) == 0 && this->hasValidSplitFileName()){
      if(textAsJSON){fputs("]",page);}else{fputs("</pdf2xml>\n",page);}
      this->passedFirstPage = gFalse;
      this->setSplitFileName(pageNum + this->split,true);
      if(textAsJSON){fputs("[",page);}else{fputs("<pdf2xml>\n",page);  }
      //printf("PROCESS: %s",tmp->getCString());
      //printf("mod %d",(this->split-(this->split/pageNum)*pageNum)); 
  }
    
  pages->dump(page, pageNum, passedFirstPage,(this->numPages));
  passedFirstPage = gTrue;
  // I don't yet know what to do in the case when there are pages of different
  // sizes and we want complex output: running ghostscript many times 
  // seems very inefficient. So for now I'll just use last page's size
  maxPageWidth = pages->pageWidth;
  maxPageHeight = pages->pageHeight;
  
  if(!stout && !globalParams->getErrQuiet()) printf("Page-%d\n",(pageNum));
}

void ImgOutputDev::updateFont(GfxState *state) {
  pages->updateFont(state);
}

void ImgOutputDev::beginString(GfxState *state, GString *s) {
  pages->beginString(state, s);
}

void ImgOutputDev::endString(GfxState *state) {
  pages->endString();
}

void ImgOutputDev::drawChar(GfxState *state, double x, double y,
	      double dx, double dy,
	      double originX, double originY,
	      CharCode code, int nBytes, Unicode *u, int uLen) 
{
  if ( !showHidden && (state->getRender() & 3) == 3) {
    return;
  }
//    printf("movex: %f",dx);
  pages->addChar(state, x, y, dx, dy, originX, originY, u, uLen);
}

void ImgOutputDev::drawImageMask(GfxState *state, Object *ref, Stream *str,
			      int width, int height, GBool invert,
			      GBool inlineImg) {

  int i, j;

  if (ignore||complexMode) {
    OutputDev::drawImageMask(state, ref, str, width, height, invert, inlineImg);
    return;
  }
  
  FILE *f1;
  int c;
  
  int x0, y0;			// top left corner of image
  int w0, h0, w1, h1;		// size of image
  double xt, yt, wt, ht;
  GBool rotate, xFlip, yFlip;
  GBool dither;
  int x, y;
  int ix, iy;
  int px1, px2, qx, dx;
  int py1, py2, qy, dy;
  Gulong pixel;
  int nComps, nVals, nBits;
  double r1, g1, b1;
 
  // get image position and size
  state->transform(0, 0, &xt, &yt);
  state->transformDelta(1, 1, &wt, &ht);
  if (wt > 0) {
    x0 = xoutRound(xt);
    w0 = xoutRound(wt);
  } else {
    x0 = xoutRound(xt + wt);
    w0 = xoutRound(-wt);
  }
  if (ht > 0) {
    y0 = xoutRound(yt);
    h0 = xoutRound(ht);
  } else {
    y0 = xoutRound(yt + ht);
    h0 = xoutRound(-ht);
  }
  state->transformDelta(1, 0, &xt, &yt);
  rotate = fabs(xt) < fabs(yt);
  if (rotate) {
    w1 = h0;
    h1 = w0;
    xFlip = ht < 0;
    yFlip = wt > 0;
  } else {
    w1 = w0;
    h1 = h0;
    xFlip = wt < 0;
    yFlip = ht > 0;
  }

  // dump JPEG file
  if (dumpJPEG  && str->getKind() == strDCT) {
    GString *fName=new GString(Docname);
    fName->append("-");
    GString *pgNum=GString::fromInt(pageNum);
    GString *imgnum=GString::fromInt(imgNum);
    // open the image file
    fName->append(pgNum)->append("_")->append(imgnum)->append(".jpg");
    ++imgNum;
    if (!(f1 = fopen(fName->getCString(), "wb"))) {
      error(-1, "Couldn't open image file '%s'", fName->getCString());
      return;
    }

    // initialize stream
    str = ((DCTStream *)str)->getRawStream();
    str->reset();

    // copy the stream
    while ((c = str->getChar()) != EOF)
      fputc(c, f1);

    fclose(f1);
   
  if (pgNum) delete pgNum;
  if (imgnum) delete imgnum;
  if (fName) delete fName;
  }
  else {
    OutputDev::drawImageMask(state, ref, str, width, height, invert, inlineImg);
  }
}

void ImgOutputDev::drawImage(GfxState *state, Object *ref, Stream *str,
			  int width, int height, GfxImageColorMap *colorMap,
			  int *maskColors, GBool inlineImg) {

  int i, j;

  if (ignore||complexMode) {
    OutputDev::drawImage(state, ref, str, width, height, colorMap, 
			 maskColors, inlineImg);
    return;
  }

  FILE *f1;
  ImageStream *imgStr;
  Guchar pixBuf[4];
  GfxColor color;
  int c;
  
  int x0, y0;			// top left corner of image
  int w0, h0, w1, h1;		// size of image
  double xt, yt, wt, ht;
  GBool rotate, xFlip, yFlip;
  GBool dither;
  int x, y;
  int ix, iy;
  int px1, px2, qx, dx;
  int py1, py2, qy, dy;
  Gulong pixel;
  int nComps, nVals, nBits;
  double r1, g1, b1;
 
  // get image position and size
  state->transform(0, 0, &xt, &yt);
  state->transformDelta(1, 1, &wt, &ht);
  if (wt > 0) {
    x0 = xoutRound(xt);
    w0 = xoutRound(wt);
  } else {
    x0 = xoutRound(xt + wt);
    w0 = xoutRound(-wt);
  }
  if (ht > 0) {
    y0 = xoutRound(yt);
    h0 = xoutRound(ht);
  } else {
    y0 = xoutRound(yt + ht);
    h0 = xoutRound(-ht);
  }
  state->transformDelta(1, 0, &xt, &yt);
  rotate = fabs(xt) < fabs(yt);
  if (rotate) {
    w1 = h0;
    h1 = w0;
    xFlip = ht < 0;
    yFlip = wt > 0;
  } else {
    w1 = w0;
    h1 = h0;
    xFlip = wt < 0;
    yFlip = ht > 0;
  }

  /*if( !globalParams->getErrQuiet() )
    printf("image stream of kind %d\n", str->getKind());*/
  // dump JPEG file
  if (dumpJPEG && str->getKind() == strDCT) {
    GString *fName=new GString(Docname);
    fName->append("-");
    GString *pgNum= GString::fromInt(pageNum);
    GString *imgnum= GString::fromInt(imgNum);  
    
    // open the image file
    fName->append(pgNum)->append("_")->append(imgnum)->append(".jpg");
    ++imgNum;
    
    if (!(f1 = fopen(fName->getCString(), "wb"))) {
      error(-1, "Couldn't open image file '%s'", fName->getCString());
      return;
    }

    // initialize stream
    str = ((DCTStream *)str)->getRawStream();
    str->reset();

    // copy the stream
    while ((c = str->getChar()) != EOF)
      fputc(c, f1);
    
    fclose(f1);
  
    delete fName;
    delete pgNum;
    delete imgnum;
  }
  else {
    OutputDev::drawImage(state, ref, str, width, height, colorMap,
			 maskColors, inlineImg);
  }
}

void ImgOutputDev::drawLink(Link* link,Catalog *cat){
  double _x1,_y1,_x2,_y2,w;
  int x1,y1,x2,y2;
  
  link->getBorder(&_x1,&_y1,&_x2,&_y2,&w);
  cvtUserToDev(_x1,_y1,&x1,&y1);
  
  cvtUserToDev(_x2,_y2,&x2,&y2); 


  GString* _dest=getLinkDest(link,cat);
  XmlLink t((double) x1,(double) y2,(double) x2,(double) y1,_dest);
  pages->AddLink(t);
  delete _dest;
}

GString* ImgOutputDev::getLinkDest(Link *link,Catalog* catalog){
  char *p;
  switch(link->getAction()->getKind()) 
  {
      case actionGoTo:
	  { 
          GString* file=new GString("actionGoTo:");
          int page=1;
          LinkGoTo *ha=(LinkGoTo *)link->getAction();
          LinkDest *dest=NULL;
          if (ha->getDest()==NULL) 
              dest=catalog->findDest(ha->getNamedDest());
          else if (ha->getNamedDest()!=NULL)
          	  dest=catalog->findDest(ha->getNamedDest());
          else
	      	  dest=ha->getDest()->copy();
	      
          if (dest){ 
              if (dest->isPageRef()){
                  Ref pageref=dest->getPageRef();
                  page=catalog->findPage(pageref.num,pageref.gen);
              }
	      else {
              page=dest->getPageNum();
	      }

	      delete dest;

	      GString *str=GString::fromInt(page);
          file->append(str);
          file->append(",");
              
          if (printCommands) printf(" link to page %d ",page);
              delete str;
              return file;
          }
          else 
          {
              return new GString();
          }
	  }
      case actionGoToR:
	  {
          LinkGoToR *ha=(LinkGoToR *) link->getAction();
          LinkDest *dest=NULL;
          int page=1;
          GString *file=new GString("actionGoToR:");
          
          if (ha->getDest()!=NULL)  dest=ha->getDest()->copy();
          
          if (dest&&file){
              if (!(dest->isPageRef()))  page=dest->getPageNum();
              delete dest;

              if (printCommands) printf(" link to page %d ",page);
              if (printHtml){
                  p=file->getCString()+file->getLength()-4;
                  file->append(GString::fromInt(page));
                  file->append(",");
              }
          }
          if (printCommands) printf("filename %s\n",file->getCString());
            return file;
      }
      case actionURI:
	  { 
          LinkURI *ha=(LinkURI *) link->getAction();
          //GString* file=new GString(ha->getURI()->getCString());
          GString *file=new GString("actionURI");
          file->append("(");file->append(ha->getURI()->getCString());file->append("):");
          //file->append(ha->getURI()->getCString());
          // printf("uri : %s\n",file->getCString());
          return file;
	  }
      case actionLaunch:
	  {
          LinkLaunch *ha=(LinkLaunch *) link->getAction();
          GString* file=new GString(ha->getFileName()->getCString());
          if (printHtml) { 
              p=file->getCString()+file->getLength()-4;
              if (!strcmp(p, ".pdf") || !strcmp(p, ".PDF")){
                  file->del(file->getLength()-4,4);
                  file->append(".html");
              }
              if (printCommands) printf("filename %s",file->getCString());
    
              return file;     
          }
	  }
      default:
	  return new GString();
  }
}

void ImgOutputDev::dumpMetaVars(FILE *file)
{
  GString *var;

  for(int i = 0; i < glMetaVars->getLength(); i++)
  {
     HtmlMetaVar *t = (HtmlMetaVar*)glMetaVars->get(i); 
     var = t->toString(); 
     fprintf(file, "%s\n", var->getCString());
     delete var;
  }
}

GBool ImgOutputDev::dumpDocOutline(Catalog* catalog)
{ 
	FILE * output;
	GBool bClose = gFalse;

	if (!ok || xml)
    	return gFalse;
}

void ImgOutputDev::updateCharSpace(GfxState *state)
{
	pages->updateCharSpace(state);
}
