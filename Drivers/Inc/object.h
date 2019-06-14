#ifndef _OBJECT_H_
#define _OBJECT_H_

#include "zu_def.h"
#include "zu_config.h"

struct zu_object_information *
zu_object_get_information(enum zu_object_class_type type);
void zu_object_init(struct zu_object         *object,
                    enum zu_object_class_type type,
                    const char               *name);
void zu_object_detach(p_zu_object_t object);
zu_bool_t zu_object_is_systemobject(zu_object_t object);
zu_object_t rt_object_find(const char *name, zu_uint8_t type);

#endif
