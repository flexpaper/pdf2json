#include "XmlLinks.h"

XmlLink::XmlLink(const XmlLink& x){
  Xmin=x.Xmin;
  Ymin=x.Ymin;
  Xmax=x.Xmax;
  Ymax=x.Ymax;
  dest=new GString(x.dest);
}

XmlLink::XmlLink(double xmin,double ymin,double xmax,double ymax,GString * _dest)
{
   if (xmin < xmax) {
    Xmin=xmin;
    Xmax=xmax;
  } else {
    Xmin=xmax;
    Xmax=xmin;
  }
  if (ymin < ymax) {
    Ymin=ymin;
    Ymax=ymax;
  } else {
    Ymin=ymax;
    Ymax=ymin;
  }                    
  dest=new GString(_dest);
}

XmlLink::~XmlLink(){
 if (dest) delete dest;
}

GBool XmlLink::isEqualDest(const XmlLink& x) const{
  return (!strcmp(dest->getCString(), x.dest->getCString()));
}

GBool XmlLink::inLink(double xmin,double ymin,double xmax,double ymax) const {
  double y=(ymin+ymax)/2;
  if (y>Ymax) return gFalse;
  return (y>Ymin)&&(xmin<Xmax)&&(xmax>Xmin);
 }
  

XmlLink& XmlLink::operator=(const XmlLink& x){
  if (this==&x) return *this;
  if (dest) {delete dest;dest=NULL;} 
  Xmin=x.Xmin;
  Ymin=x.Ymin;
  Xmax=x.Xmax;
  Ymax=x.Ymax;
  dest=new GString(x.dest);
  return *this;
} 

GString* XmlLink::getLinkStart() {
  GString *res = new GString("");
  res->append(dest);
//  res->append("\">");
  return res;
}

/*GString* XmlLink::Link(GString* content){
  //GString* _dest=new GString(dest);
  GString *tmp=new GString("<a href=\"");
  tmp->append(dest);
  tmp->append("\">");
  tmp->append(content);
  tmp->append("</a>");
  //delete _dest;
  return tmp;
  }*/

   

XmlLinks::XmlLinks(){
  accu=new GVector<XmlLink>();
}

XmlLinks::~XmlLinks(){
  delete accu;
  accu=NULL; 
}

GBool XmlLinks::inLink(double xmin,double ymin,double xmax,double ymax,int& p)const {
  
  for(GVector<XmlLink>::iterator i=accu->begin();i!=accu->end();i++){
    if (i->inLink(xmin,ymin,xmax,ymax)) {
        p=(i - accu->begin());
        return 1;
    }
   }
  return 0;
}

XmlLink* XmlLinks::getLink(int i) const{
  GVector<XmlLink>::iterator g=accu->begin();
  g+=i; 
  return g;
}

