#include <gtest/gtest.h>

#define ASSERT_STMT_OK(con, sql, callback)                                     \
    {                                                                          \
        sqlw::Statement _stmt{&con};                                           \
        std::error_code _ec = _stmt(sql, callback);                            \
        ASSERT_TRUE(sqlw::status::Condition::OK == _ec)                        \
            << _ec.value() << _ec.message() << sql;                            \
    }

#define ASSERT_ERR_NOK(err)                                                    \
    {                                                                          \
        ASSERT_NE(wholth_Error_OK.code, err.code)                              \
            << err.code << wfsv(err.message);                                  \
        ASSERT_NE(wholth_Error_OK.message.size, err.message.size)              \
            << err.code << wfsv(err.message);                                  \
    }

#define ASSERT_ERR_NOK2(err, msg)                                              \
    {                                                                          \
        ASSERT_NE(wholth_Error_OK.code, err.code)                              \
            << err.code << wfsv(err.message) << msg;                           \
        ASSERT_NE(wholth_Error_OK.message.size, err.message.size)              \
            << err.code << wfsv(err.message) << msg;                           \
    }

#define ASSERT_ERR_OK(err)                                                     \
    {                                                                          \
        ASSERT_EQ(wholth_Error_OK.code, err.code)                              \
            << err.code << wfsv(err.message);                                  \
        ASSERT_EQ(wholth_Error_OK.message.size, err.message.size)              \
            << err.code << wfsv(err.message);                                  \
    }

#define ASSERT_ERR_OK2(err, msg)                                               \
    {                                                                          \
        ASSERT_EQ(wholth_Error_OK.code, err.code)                              \
            << err.code << wfsv(err.message) << msg;                           \
        ASSERT_EQ(wholth_Error_OK.message.size, err.message.size)              \
            << err.code << wfsv(err.message) << msg;                           \
    }

#define ASSERT_ERR_MSG(err, expected)                                          \
    {                                                                          \
        ASSERT_STREQ2(expected, wfsv(err.message));                            \
    }
