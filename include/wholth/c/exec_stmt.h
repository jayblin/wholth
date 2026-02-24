#ifndef WHOLTH_C_EXEC_STMT_H_
#define WHOLTH_C_EXEC_STMT_H_

#include "wholth/c/error.h"
#include "wholth/c/string_view.h"
#include "wholth/c/buffer.h"

#ifdef __cplusplus
extern "C"
{
#endif

    // typedef enum wholth_EntityFieldType_t
    // {
    //     weft_INT,
    //     weft_DOUBLE,
    //     weft_TEXT,
    //     weft_NULL,
    // } wholth_EntityFieldType;

    typedef struct wholth_exec_stmt_Bindable_t
    {
        wholth_StringView value;
        // wholth_EntityFieldType type;
    } wholth_exec_stmt_Bindable;

    typedef enum wholth_exec_stmt_Task_t
    {
        wholth_exec_stmt_Task_NOOP,
        wholth_exec_stmt_Task_DELETE,
    } wholth_exec_stmt_Task;

    typedef struct wholth_exec_stmt_Args_t
    {
        wholth_StringView                      sql_file;
        unsigned long                          binds_size;
        const wholth_exec_stmt_Bindable* const binds;
        wholth_Buffer* const                   buffer;
    } wholth_exec_stmt_Args;

    // typedef enum wholth_exec_stmt_ResultType_e
    // {
    //     wholth_exec_stmt_ResultType_EMPTY,
    //     wholth_exec_stmt_ResultType_ERROR,
    // } wholth_exec_stmt_ResultType;

    // typedef union wholth_exec_stmt_Result_u {
    //     wholth_Error error;
    // } wholth_exec_stmt_Result_u;

    // typedef struct wholth_exec_stmt_Result_t
    // {
    //     wholth_exec_stmt_ResultType tag{wholth_exec_stmt_ResultType_EMPTY};
    //     wholth_exec_stmt_Result_u   result;
    // } wholth_exec_stmt_Result;

    enum wholth_exec_stmt_Code
    {
        wholth_exec_stmt_Code_FIRST_ = 10000,
        wholth_exec_stmt_Code_SQL_FILE_DOES_NOT_EXIST,
        wholth_exec_stmt_Code_NOOP,
        wholth_exec_stmt_Code_SQL_FILE_ARG_MISCONF,
        wholth_exec_stmt_Code_SQL_FILE_VALIDATOR_MISCONF,
        wholth_exec_stmt_Code_SQL_FILE_VALIDATOR_UNKNOWN,
        wholth_exec_stmt_Code_SQL_FILE_BINDABLE_TYPE_MISCONF,
        wholth_exec_stmt_Code_SQL_FILE_BINDABLE_MISCOUNT,
        wholth_exec_stmt_Code_SQL_FILE_MISCONF,
        wholth_exec_stmt_Code_BINDS_MISCOUNT,
        wholth_exec_stmt_Code_ARGS_INVALID,
        wholth_exec_stmt_Code_LAST_,
        wholth_exec_stmt_Code_COUNT_ =
            wholth_exec_stmt_Code_LAST_ - wholth_exec_stmt_Code_FIRST_ - 1,
    };

    /**
     * Look into `???????` directory to find out what script does.
     */
    // wholth_exec_stmt_Result wholth_exec_stmt(
    wholth_Error wholth_exec_stmt(const wholth_exec_stmt_Args* const);

    // bool wholth_is_error(const wholth_exec_stmt_Result* const);

#ifdef __cplusplus
}
#endif

#endif // WHOLTH_C_EXEC_STMT_H_
