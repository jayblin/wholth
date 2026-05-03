#ifndef WHOLTH_C_EXEC_STMT_H_
#define WHOLTH_C_EXEC_STMT_H_

#include "wholth/c/error.h"
#include "wholth/c/string_view.h"

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
        wholth_exec_stmt_Task_INSERT,
        wholth_exec_stmt_Task_SELECT,
        wholth_exec_stmt_Task_UPDATE,
    } wholth_exec_stmt_Task;

    struct wholth_exec_stmt_Result_t;
    typedef struct wholth_exec_stmt_Result_t wholth_exec_stmt_Result;

    wholth_Error       wholth_exec_stmt_Result_new(wholth_exec_stmt_Result**);
    wholth_Error       wholth_exec_stmt_Result_del(wholth_exec_stmt_Result*);
    unsigned long long wholth_exec_stmt_Result_row_count(
        const wholth_exec_stmt_Result*);
    const wholth_StringView wholth_exec_stmt_Result_at(
        const wholth_exec_stmt_Result*,
        unsigned long long row,
        unsigned long long column);
    const wholth_StringView wholth_exec_stmt_Result_full_error_msg(
        const wholth_exec_stmt_Result*);

    typedef struct wholth_exec_stmt_Args_t
    {
        wholth_StringView                      sql_file;
        unsigned long long                     binds_size;
        const wholth_exec_stmt_Bindable* const binds;
        bool                                   skip_cache;
        // wholth_Buffer* const                   buffer;
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
        wholth_exec_stmt_Code_NOOP,
        wholth_exec_stmt_Code_SQL_FILE_DOES_NOT_EXIST,
        wholth_exec_stmt_Code_SQL_FILE_ARG_MISCONF,
        wholth_exec_stmt_Code_SQL_FILE_VALIDATOR_MISCONF,
        wholth_exec_stmt_Code_SQL_FILE_VALIDATOR_UNKNOWN,
        wholth_exec_stmt_Code_SQL_FILE_BINDABLE_TYPE_MISCONF,
        wholth_exec_stmt_Code_SQL_FILE_BINDABLE_MISCOUNT,
        wholth_exec_stmt_Code_SQL_FILE_MISCONF,
        wholth_exec_stmt_Code_BINDS_MISCOUNT,
        wholth_exec_stmt_Code_ARGS_NULLPTR,
        wholth_exec_stmt_Code_ARGS_RESULT_IS_NULLPTR,
        wholth_exec_stmt_Code_BINDABLE_VALUE_IS_NULLPTR_BUT_SIZE_IS_GT_0,
        wholth_exec_stmt_Code_BINDABLE_VALIDATION_FAIL,
        wholth_exec_stmt_Code_LAST_,
        wholth_exec_stmt_Code_COUNT_ =
            wholth_exec_stmt_Code_LAST_ - wholth_exec_stmt_Code_FIRST_ - 1,
    };

    /**
     * Look into `sql_statements` directory at the core of this project to find
     * out what script does.
     */
    wholth_Error wholth_exec_stmt(
        const wholth_exec_stmt_Args* const,
        wholth_exec_stmt_Result*);

    // bool wholth_is_error(const wholth_exec_stmt_Result* const);

#ifdef __cplusplus
}
#endif

#endif // WHOLTH_C_EXEC_STMT_H_
