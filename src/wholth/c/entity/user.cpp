#include "wholth/c/entity/user.h"

extern "C" wholth_User wholth_entity_user_init()
{
    return {
        .id = wholth_StringView_default,
        .name = wholth_StringView_default};
}
