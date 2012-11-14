/*
     Pippolo - a nosql distributed database.
     Copyright (C) 2012 Andrea Nardinocchi (nardinocchi@psychogames.net)
     
     This program is free software: you can redistribute it and/or modify
     it under the terms of the GNU General Public License as published by
     the Free Software Foundation, either version 3 of the License, or
     (at your option) any later version.
     
     This program is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
     GNU General Public License for more details.
     
     You should have received a copy of the GNU General Public License
     along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef pippolo_string_h
#define pippolo_string_h
#include "p_api_memory.h"
#define pippolo_channel_length 128
#define finalchar(x)\
    (((x)=='\n')||((x)=='\r')||((x)=='\0'))
#define spacechar(x)\
    (((x)=='\t')||((x)==' '))
#define realmchar(x)\
    (((x)!='\t')&&((x)!='\n')&&((x)!=' '))
#define constchar(x)\
    ((((x)>=32)&&((x)<=126))||((x)==9)||((x)==10))
#define pippolo_kill(frm)\
    do{\
        fprintf(stderr,"[pippolo::kill] (%s|%d) %s\n",__FILE__,__LINE__,frm);\
        exit(1);\
    }while(0)
#define pippolo_capitalize(str)\
    do{\
        for(int i=0;i<p_string_len((str));i++)\
            str[i]=toupper(str[i]);\
    }while(0)
#define pippolo_append(str,app)\
    do{\
        size_t strsiz=p_string_len(str),appsiz=p_string_len(app),tot;\
        tot=strsiz+appsiz;\
        if (((str)=(char*)pippolo_realloc((str),tot+1)))\
            strcpy((str)+(strsiz),(app));\
    }while(0)
#define pippolo_allocated(a,b)\
    ((((a)=(char*)pippolo_malloc(p_string_len((b)+1)))&&(snprintf((a),p_string_len(b)+1,"%s",(b))))?(a):NULL)
#define pippolo_len(a)\
    (a),p_string_len(a)
#define p_string_len(x)\
    ((x)?strlen(x):0)
#define p_string_cmp(x,y)\
    ((!x&&!y)?0:((!x)?1:((!y)?-1:strcmp(x,y))))
#define p_string_case_cmp(x,y)\
    ((!x&&!y)?0:((!x)?1:((!y)?-1:strcasecmp(x,y))))
#define p_string_atoi_unsigned(a)\
    (abs(atoi((a))))
#define p_string_random_token(str,len)\
    do {\
        size_t i,res;\
        for (i=0;i<len;i++){\
            do {\
                res=(rand()%(122-48))+48;\
            } while (((res>57)&&(res<65))||((res>90)&&(res<95))||((res>95)&&(res<97)));\
            str[i]=(char)res;\
        }\
        str[i]='\0';\
    } while(0)
void p_string_trim (char *string);
#endif
