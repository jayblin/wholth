#include "vk/vk.hpp"

void check_no_msg(VkResult result)
{
    vk::check(result, "[No message]");
}
