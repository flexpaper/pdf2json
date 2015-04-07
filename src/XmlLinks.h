#ifndef _HTML_LINKS
#define _HTML_LINKS

#include "GVector.h"
#include "GString.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef _WIN32
#include <sys/unistd.h>
#endif

class XmlLink{

private:  
  double Xmin;
  double Ymin;
  double Xmax;
  double Ymax;
  GString* dest;

public:
  XmlLink(){dest=NULL;}
  XmlLink(const XmlLink& x);
  XmlLink& operator=(const XmlLink& x);
  XmlLink(double xmin,double ymin,double xmax,double ymax,GString *_dest);
  ~XmlLink();
  //GBool XmlLink::isEqualDest(const XmlLink& x) const; // this kills gcc 4.*
  GBool isEqualDest(const XmlLink& x) const;
  GString *getDest(){return new GString(dest);}
  double getX1() const {return Xmin;}
  double getX2() const {return Xmax;}
  double getY1() const {return Ymin;}
  double getY2() const {return Ymax;}
  GBool inLink(double xmin,double ymin,double xmax,double ymax) const ;
  //GString *Link(GString *content);
  GString* getLinkStart();
  
};

class XmlLinks{
private:
 GVector<XmlLink> *accu;
public:
 XmlLinks();
 ~XmlLinks();
 void AddLink(const XmlLink& x) {accu->push_back(x);}
 GBool inLink(double xmin,double ymin,double xmax,double ymax,int& p) const;
 XmlLink* getLink(int i) const;

};

#endif
   
