#include "db/db.hpp"
#include "fmt/core.h"
#include "helpers.hpp"
#include "sqlw/statement.hpp"
#include "wholth/c/app.h"
#include "wholth/c/entity/user.h"
#include "wholth/c/entity_manager/user.h"
#include "wholth/entity_manager/user.hpp"
#include <gtest/gtest.h>
#include <sstream>
#include <type_traits>

static_assert(nullptr == (void*)NULL);

using bind_t = sqlw::Statement::bindable_t;

class Test_wholth_em_user_insert : public ApplicationAwareTest
{
};

constexpr std::string_view count_users_sql = "SELECT COUNT(id) FROM user";

TEST_F(Test_wholth_em_user_insert, when_buffer_nullptr)
{
    std::string old_count = "bogus";
    astmt(db::connection(), count_users_sql, [&](auto e) {
        old_count = e.column_value;
    });
    ASSERT_STRNEQ2("bogus", old_count);

    wholth_User user = wholth_entity_user_init();
    user.name = wtsv("Test User");
    const auto err = wholth_em_user_insert(
        &user, wtsv("12345678901234567890123456789"), NULL);

    ASSERT_NE(wholth_Error_OK.code, err.code) << err.code << wfsv(err.message);
    ASSERT_NE(wholth_Error_OK.message.size, err.message.size)
        << err.code << wfsv(err.message);

    ASSERT_EQ(0, user.id.size);
    ASSERT_EQ(nullptr, user.id.data);

    std::string new_count = "bogus";
    astmt(db::connection(), count_users_sql, [&](auto e) {
        new_count = e.column_value;
    });
    ASSERT_STREQ3(new_count, old_count);
}

TEST_F(Test_wholth_em_user_insert, when_user_nullptr)
{
    std::string old_count = "bogus";
    astmt(db::connection(), count_users_sql, [&](auto e) {
        old_count = e.column_value;
    });
    ASSERT_STRNEQ2("bogus", old_count);

    wholth_Buffer* scratch = wholth_buffer_ring_pool_element();
    const auto err = wholth_em_user_insert(
        NULL, wtsv("12345678901234567890123456789"), scratch);

    ASSERT_NE(wholth_Error_OK.code, err.code) << err.code << wfsv(err.message);
    ASSERT_NE(wholth_Error_OK.message.size, err.message.size)
        << err.code << wfsv(err.message);

    std::error_code ec = wholth::entity_manager::user::Code(err.code);
    ASSERT_EQ(wholth::entity_manager::user::Code::USER_NULL, ec)
        << ec << ec.message();

    std::string new_count = "bogus";
    astmt(db::connection(), count_users_sql, [&](auto e) {
        new_count = e.column_value;
    });
    ASSERT_STREQ3(new_count, old_count);
}

TEST_F(Test_wholth_em_user_insert, when_password_too_small)
{
    std::string old_count = "bogus";
    astmt(db::connection(), count_users_sql, [&](auto e) {
        old_count = e.column_value;
    });
    ASSERT_STRNEQ2("bogus", old_count);

    wholth_Buffer* scratch = wholth_buffer_ring_pool_element();
    wholth_User user = wholth_entity_user_init();
    user.name = wtsv("Test User");

    const std::vector<wholth_StringView> passwords{{
        {.data = nullptr, .size = 0},
        {.data = "", .size = 0},
        {.data = "0", .size = 1},
        {.data = "12345657", .size = 0},
        {.data = "123456789012345", .size = 15},
    }};
    for (const auto& pass : passwords)
    {
        const auto err = wholth_em_user_insert(&user, pass, scratch);

        ASSERT_NE(wholth_Error_OK.code, err.code)
            << err.code << wfsv(err.message);
        ASSERT_NE(wholth_Error_OK.message.size, err.message.size)
            << err.code << wfsv(err.message);

        std::error_code ec = wholth::entity_manager::user::Code(err.code);
        ASSERT_EQ(
            wholth::entity_manager::user::Code::USER_PASSWORD_TOO_SHORT, ec)
            << ec << ec.message();

        ASSERT_EQ(0, user.id.size);
        ASSERT_EQ(nullptr, user.id.data);

        std::string new_count = "bogus";
        astmt(db::connection(), count_users_sql, [&](auto e) {
            new_count = e.column_value;
        });
        ASSERT_STREQ3(new_count, old_count);
    }
}

TEST_F(Test_wholth_em_user_insert, when_password_too_large)
{
    std::string old_count = "bogus";
    astmt(db::connection(), count_users_sql, [&](auto e) {
        old_count = e.column_value;
    });
    ASSERT_STRNEQ2("bogus", old_count);

    wholth_Buffer* scratch = wholth_buffer_ring_pool_element();
    wholth_User user = wholth_entity_user_init();
    user.name = wtsv("Test User");

    const std::vector<wholth_StringView> passwords{{
        {.data = "", .size = 257},
        {.data = "1234567890123456789012345678901234567890123456789012345678901"
                 "2345678901234567890123456789012345678901234567890123456789012"
                 "3456789012345678901234567890123456789012345678901234567890123"
                 "4567890123456789012345678901234567890123456789012345678901234"
                 "5678901234567890",
         .size = 257},
    }};
    for (const auto& pass : passwords)
    {
        const auto err = wholth_em_user_insert(&user, pass, scratch);

        ASSERT_NE(wholth_Error_OK.code, err.code)
            << err.code << wfsv(err.message);
        ASSERT_NE(wholth_Error_OK.message.size, err.message.size)
            << err.code << wfsv(err.message);

        std::error_code ec = wholth::entity_manager::user::Code(err.code);
        ASSERT_EQ(
            wholth::entity_manager::user::Code::USER_PASSWORD_TOO_LONG, ec)
            << ec << ec.message();

        ASSERT_EQ(0, user.id.size);
        ASSERT_EQ(nullptr, user.id.data);

        std::string new_count = "bogus";
        astmt(db::connection(), count_users_sql, [&](auto e) {
            new_count = e.column_value;
        });
        ASSERT_STREQ3(new_count, old_count);
    }
}

TEST_F(Test_wholth_em_user_insert, when_name_is_bad)
{
    std::string old_count = "bogus";
    astmt(db::connection(), count_users_sql, [&](auto e) {
        old_count = e.column_value;
    });
    ASSERT_STRNEQ2("bogus", old_count);

    wholth_Buffer* scratch = wholth_buffer_ring_pool_element();
    wholth_User user = wholth_entity_user_init();
    user.name = wtsv("Test User");

    const std::vector<wholth_StringView> names{{
        {.data = nullptr, .size = 0},
        wtsv(""),
    }};
    for (const auto& name : names)
    {
        user.name = name;
        const auto err =
            wholth_em_user_insert(&user, wtsv("1234567890123456"), scratch);

        ASSERT_NE(wholth_Error_OK.code, err.code)
            << err.code << wfsv(err.message);
        ASSERT_NE(wholth_Error_OK.message.size, err.message.size)
            << err.code << wfsv(err.message);

        std::error_code ec = wholth::entity_manager::user::Code(err.code);
        ASSERT_EQ(wholth::entity_manager::user::Code::USER_NO_NAME, ec)
            << ec << ec.message();

        ASSERT_EQ(0, user.id.size);
        ASSERT_EQ(nullptr, user.id.data);

        std::string new_count = "bogus";
        astmt(db::connection(), count_users_sql, [&](auto e) {
            new_count = e.column_value;
        });
        ASSERT_STREQ3(new_count, old_count);
    }
}

TEST_F(Test_wholth_em_user_insert, when_name_not_unique)
{
    astmt(
        db::connection(),
        "INSERT INTO user (id, name, password_hashed, locale_id) "
        "VALUES (1, 'Test User', 'asd', 1)");

    std::string old_count = "bogus";
    astmt(db::connection(), count_users_sql, [&](auto e) {
        old_count = e.column_value;
    });
    ASSERT_STRNEQ2("bogus", old_count);

    wholth_Buffer* scratch = wholth_buffer_ring_pool_element();
    wholth_User user = wholth_entity_user_init();
    user.name = wtsv(" Test User       ");

    const std::vector<wholth_StringView> names{{
        {.data = nullptr, .size = 0},
        wtsv(""),
    }};
    const auto err =
        wholth_em_user_insert(&user, wtsv("1234567890123456"), scratch);

    ASSERT_NE(wholth_Error_OK.code, err.code) << err.code << wfsv(err.message);
    ASSERT_NE(wholth_Error_OK.message.size, err.message.size)
        << err.code << wfsv(err.message);

    std::error_code ec = wholth::entity_manager::user::Code(err.code);
    ASSERT_EQ(wholth::entity_manager::user::Code::USER_AUTHENTICATION_FAILED_GONKED, ec)
        << ec << ec.message();

    ASSERT_EQ(0, user.id.size);
    ASSERT_EQ(nullptr, user.id.data);

    std::string new_count = "bogus";
    astmt(db::connection(), count_users_sql, [&](auto e) {
        new_count = e.column_value;
    });
    ASSERT_STREQ3(new_count, old_count);
}

TEST_F(Test_wholth_em_user_insert, when_no_salt_env)
{
    astmt(
        db::connection(),
        "INSERT INTO user (id, name, password_hashed, locale_id) "
        "VALUES (1, 'Test User', 'asd', 1)");

    std::string old_count = "bogus";
    astmt(db::connection(), count_users_sql, [&](auto e) {
        old_count = e.column_value;
    });
    ASSERT_STRNEQ2("bogus", old_count);

    wholth_Buffer* scratch = wholth_buffer_ring_pool_element();
    wholth_User user = wholth_entity_user_init();
    user.name = wtsv("  Test User   ");
    const auto err = wholth_em_user_insert(
        &user, wtsv("12345678901234567890123456789"), scratch);

    ASSERT_NE(wholth_Error_OK.code, err.code) << err.code << wfsv(err.message);
    ASSERT_NE(wholth_Error_OK.message.size, err.message.size)
        << err.code << wfsv(err.message);

    ASSERT_EQ(0, user.id.size);
    ASSERT_EQ(nullptr, user.id.data);

    std::string new_count = "bogus";
    astmt(db::connection(), count_users_sql, [&](auto e) {
        new_count = e.column_value;
    });
    ASSERT_STREQ3(new_count, old_count);
}

TEST_F(Test_wholth_em_user_insert, when_good_case)
{
    // static char* salt = "WHOLTH_PASSWORD_SALT=THE_SALT";
    // putenv(salt);
    wholth_app_password_encryption_secret(wtsv("THE_SALT"));

    wholth_Buffer* scratch = wholth_buffer_ring_pool_element();
    wholth_User user = wholth_entity_user_init();
    user.name = wtsv("  Test User   ");
    user.locale_id = wtsv("2");
    const auto err = wholth_em_user_insert(
        &user, wtsv("12345678901234567890123456789"), scratch);

    ASSERT_EQ(wholth_Error_OK.code, err.code) << err.code << wfsv(err.message);
    ASSERT_EQ(wholth_Error_OK.message.size, err.message.size)
        << err.code << wfsv(err.message);

    ASSERT_NE(0, user.id.size);
    ASSERT_NE(nullptr, user.id.data);

    const std::string check_sql = fmt::format(
        R"sql(
        SELECT
            name,
            password_hashed,
            locale_id
        FROM user
        WHERE id = {0}
        )sql",
        wfsv(user.id));
    std::stringstream ss;
    astmt(db::connection(), check_sql, [&](auto e) {
        ss << e.column_name << ":" << e.column_value << ';';
    });
    const std::string check_data_post = ss.str();
    ASSERT_STREQ2(
        "name:Test "
        "User;password_hashed:"
        // "CACF2E22E136A38F05D199AEA8B2C80C2FF55431853E4AEA0A67D21C1C6FF6C6;"
        // <-- this was sha256 version
        "8BE12DEC4427C65FC4F0C8A8E796DACB92FEAE963AD669"
        "B40711408393857C1E8C996177419C393F3F74F5C5BB36"
        "A4BA2C34AEAEEBC7D134E43FBD5D26CD58F262A1228730"
        "D92D4F052AD07AFF3331F706E10965B32A986FAECD9390"
        "489B46ACDD4122600DF75D110417F4684AE60F889A983B"
        "EF5B0E4A015F255E159A92DF82343296AA95F7C1A00D1E"
        "5ED26E71165AF22931CE3FE3C01AEB412C8F317BFD1960"
        "FA669AF9B9D90C91F2D612E594758824B82F1905D4A175"
        "62593D0AE6AFF78FC29D9A14A5E61E9D9AFFFD8F0123B3"
        "96F79D7E5487192A0F398E1BCBB47DCA26CB84ABF0DE6D"
        "3E16582380490CDAF09B465B57DB3B85EC8A46464FA9AA"
        "263F41;"
        "locale_id:2;",
        check_data_post);
}
