/*
 * File name: CEquids.h
 * Date:      2006/10/12 11:56
 * Author:    Petr Stepan, CTU from Prague
 */

#ifndef __CEQUIDS_H__
#define __CEQUIDS_H__

#include "CJockey.h"
#include <stdio.h>
#define MAX_JOCKEYS 20

class CEquids
{
   CJockey jockeys[MAX_JOCKEYS];
   int analyze(char *line, FILE *fp);
   bool start(void);
   int num_jockeys;
   int runningJockey;
public:
   CEquids();
   ~CEquids();
   bool init(const char *filename);
   void initJockey(int j);
   void switchToJockey(int j);
   void sendMessage(int j, CMessage &m);
   int  find(const char *name);
   void quit();
};

#endif
/* end of CEquids.h */
