#include "myMemory.h"
#include "CjsonApp.h"
#include "string.h"

void CJSON_init()
{
    cJSON_Hooks hooks;
    hooks.malloc_fn = &MemMalloc;
    hooks.free_fn = &MemFree;
    cJSON_InitHooks(&hooks);
}

//parse a key-value pair
int cJSON_to_str(char *json_string, char *str_val)
{
    cJSON *root=cJSON_Parse(json_string);
    void *p;
    if (!root)
    {
//        printf("Error before: [%s]\n",cJSON_GetErrorPtr());
        return -1;
    }
	else
    {
		cJSON *item=cJSON_GetObjectItem(root, str_val);
        if(item!=NULL)
        {
            p = &item;
//            printf("cJSON_GetObjectItem: type=%d, key is %s, value is %s\n",item->type,item->string,item->valuestring);
            memcpy(str_val, item->valuestring, strlen(item->valuestring));
        }
        cJSON_Delete(root);
    }
    return 0;
}


