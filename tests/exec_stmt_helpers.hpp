#include "wholth/c/exec_stmt.h"

namespace wholth::exec_stmt
{
struct ResultWrap
{
    wholth_exec_stmt_Result* handle = nullptr;

    ResultWrap()
    {
        wholth_exec_stmt_Result_new(&handle);
    }

    ~ResultWrap()
    {
        wholth_exec_stmt_Result_del(handle);
    }
};
} // namespace exec_stmt
