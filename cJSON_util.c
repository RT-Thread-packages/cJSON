#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include <rtthread.h>

#include "cJSON.h"
#include "cJSON_util.h"

void cJSON_free(void *ptr)
{
    rt_free(ptr);
}

const char * cJSON_item_get_string(cJSON *object, const char *item_name)
{
    cJSON *item;

    item = cJSON_GetObjectItem(object, item_name);

    if(!item)
        return 0;

    if( (item->type != cJSON_String) && (item->type != cJSON_Array) )
        return 0;

    if(item->type == cJSON_Array)
        return item->child->valuestring; // TODO

    return item->valuestring;
}

int cJSON_item_get_number(cJSON *object, const char *item_name, int * result)
{
    cJSON *item;

    item = cJSON_GetObjectItem(object, item_name);

    if(!item)
        return -1;

    if(item->type != cJSON_Number)
        return -1;

    if(result)
        *result = item->valueint;

    return 0;
}

void cJSON_AddInteger2StringToObject(cJSON *object, const char *name, int i)
{
    char str_buf[10+2];

    sprintf(str_buf, "%d", i);
    cJSON_AddStringToObject(object, name, str_buf);
}
/*for example, json object is "{ "phoneNumber":[ 
                           {
                            "type": "home",
                            "number": "212 555-1234"
                           },
                          {
                            "type": "fax",
                            "number": "646 555-4567"
                          }
                          ]}"
 *use path "phoneNumber[0].type" can get cJSON object "type": "home"
*/
int cJSON_getJsonItem(const cJSON* SrcItem, const char* path, cJSON** ResultItem)
{
    char* indexOfDot;
    char* dotPropertyName;
    char* dotSubPath;
    uint8_t pathlen=strlen(path);
    uint8_t length;
    char * emptystring="";
    indexOfDot = strchr(path,'.');
    if(indexOfDot==NULL)//no "." in path, copy  entire path to dotPropertyName,dotSubPath is empty string
    {
        length=pathlen+1;
        dotPropertyName=(char *)malloc(length);
        strncpy(dotPropertyName,path,length);

        dotSubPath=emptystring;
    }
    else
    {
        length=indexOfDot-path+1;
        dotPropertyName=(char *)malloc(length);
        strncpy(dotPropertyName,path,length);
        dotPropertyName[length-1]='\0';

        dotSubPath=indexOfDot+1;
    }

    const char* indexOfSquareBracketOpen = strchr(path,'[');
    const char* indexOfSquareBracketClose = strchr(path,']');
    uint8_t arrayIndex;
    char * squareBracketPropertyName;
    //no "[]" in path, copy  entire path to squareBracketPropertyName,squareBracketSubPath is empty string
    if((indexOfSquareBracketOpen==NULL)||(indexOfSquareBracketClose==NULL))
    {
        length=pathlen+1;
        arrayIndex =0;
        squareBracketPropertyName =(char *)malloc(length);
        strncpy(squareBracketPropertyName,path,length);

    }
    else
    {
        length=indexOfSquareBracketOpen-path+1;
        arrayIndex = atoi(indexOfSquareBracketOpen + 1);
        squareBracketPropertyName =(char *)malloc(length);
        strncpy(squareBracketPropertyName,path,length);
        squareBracketPropertyName[length-1]='\0';
    }

    char* squareBracketSubPath;
    if(squareBracketPropertyName[0]=='\0')// squareBracketPropertyName is empty
        squareBracketSubPath = indexOfSquareBracketClose > 0 ? (path[indexOfSquareBracketClose-path + 1] == '.' ? indexOfSquareBracketClose + 2 : indexOfSquareBracketClose + 1 ): emptystring;
    else
        squareBracketSubPath = indexOfSquareBracketOpen > 0 ? indexOfSquareBracketOpen : emptystring;

    // determine what is first in path. dot or bracket
    uint8_t useDot = 1;
    if (indexOfDot >= path) // there is a dot in path
    {
        if (indexOfSquareBracketOpen >= path) // there is squarebracket in path
        {
            if (indexOfDot > indexOfSquareBracketOpen)
                useDot = 0;
            else
                useDot = 1;
        }
        else
            useDot = 1;
    }
    else
    {
        if (indexOfSquareBracketOpen >= path)
            useDot = 0;
        else
            useDot = 1; // acutally, id doesn't matter, both dot and square bracket don't exist
    }

    char* usedPropertyName = useDot ? dotPropertyName : squareBracketPropertyName;
    char* usedSubPath = useDot ? dotSubPath : squareBracketSubPath;

    cJSON* subItem;
    if (SrcItem->type==cJSON_Array)
    {
        uint8_t arraysize=cJSON_GetArraySize(SrcItem);
        if(arrayIndex>=arraysize) return -1;
        subItem = cJSON_GetArrayItem(SrcItem,arrayIndex);

        if(usedSubPath[0]=='\0')//empty
            *ResultItem=subItem;
        else
            cJSON_getJsonItem(subItem,usedSubPath,ResultItem);

    }
    else if (SrcItem->type==cJSON_Object)
    {
        subItem = cJSON_GetObjectItem(SrcItem,usedPropertyName);

        if(usedSubPath[0]=='\0')//empty
            *ResultItem=subItem;
        else
            cJSON_getJsonItem(subItem,usedSubPath,ResultItem);
    }
    else
        *ResultItem=SrcItem;//return ResultItem

    free(dotPropertyName);
    free(squareBracketPropertyName);
}