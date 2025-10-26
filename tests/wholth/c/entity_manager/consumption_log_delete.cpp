#include "db/db.hpp"
#include "fmt/core.h"
#include "helpers.hpp"
#include "sqlw/statement.hpp"
#include "wholth/c/buffer.h"
#include "wholth/c/entity/consumption_log.h"
#include "wholth/c/entity_manager/consumption_log.h"
#include "wholth/entity_manager/consumption_log.hpp"
#include "wholth/entity_manager/user.hpp"
#include <string>
#include <unistd.h>
#include <vector>

static_assert(nullptr == (void*)NULL);

using bind_t = sqlw::Statement::bindable_t;

class Test_wholth_em_consumption_log_delete : public ApplicationAwareTest
{
    void SetUp() override
    {
        ApplicationAwareTest::SetUp();

        astmt(
            db::connection(),
            "INSERT OR REPLACE INTO user "
            " (id, name, password_hashed, locale_id) VALUES "
            " (1, 'test', 'standalone', 1),"
            " (2, 'test-2', 'standalone', 1);");
    }
};

TEST_F(Test_wholth_em_consumption_log_delete, when_log_is_nullptr)
{
    auto& con = db::connection();

    std::string id{"bogus"};
    astmt(
        con,
        "INSERT INTO consumption_log "
        "(food_id, user_id, mass, consumed_at) "
        "VALUES (100, 1, 210, 'bogus') "
        "RETURNING id",
        [&](auto e) { id = e.column_value; });
    ASSERT_STRNE2("bogus", id);

    std::string old_count{"bogus"};
    astmt(con, "SELECT COUNT(food_id) FROM consumption_log", [&](auto e) {
        old_count = e.column_value;
    });
    ASSERT_NE("bogus", old_count);

    wholth_Buffer* buf = wholth_buffer_ring_pool_element();
    wholth_User user{};
    const wholth_Error err = wholth_em_consumption_log_delete(NULL, &user, buf);

    ASSERT_NE(wholth_Error_OK.code, err.code) << err.code << wfsv(err.message);
    ASSERT_NE(wholth_Error_OK.message.size, err.message.size)
        << err.code << wfsv(err.message);
    std::error_code ec =
        wholth::entity_manager::consumption_log::Code(err.code);
    ASSERT_EQ(
        wholth::entity_manager::consumption_log::Code::CONSUMPTION_LOG_NULL, ec)
        << ec << ec.message();

    std::string new_count{"bogus"};
    astmt(con, "SELECT COUNT(food_id) FROM consumption_log", [&](auto e) {
        new_count = e.column_value;
    });
    ASSERT_EQ(old_count, new_count);

    std::stringstream check;
    astmt(
        con,
        fmt::format(
            "SELECT food_id, mass, consumed_at FROM consumption_log "
            "WHERE id = {0}",
            id),
        [&](auto e) { check << e.column_value << ";"; });
    ASSERT_STREQ2("100;210;bogus;", check.str())
        << "Should be no changes to existing record";
}

TEST_F(Test_wholth_em_consumption_log_delete, when_user_is_nullptr)
{
    auto& con = db::connection();

    std::string id{"bogus"};
    astmt(
        con,
        "INSERT INTO consumption_log "
        "(food_id, user_id, mass, consumed_at) "
        "VALUES (100, 1, 210, 'bogus') "
        "RETURNING id",
        [&](auto e) { id = e.column_value; });
    ASSERT_STRNE2("bogus", id);

    std::string old_count{"bogus"};
    astmt(con, "SELECT COUNT(food_id) FROM consumption_log", [&](auto e) {
        old_count = e.column_value;
    });
    ASSERT_NE("bogus", old_count);

    wholth_Buffer* buf = wholth_buffer_ring_pool_element();
    wholth_ConsumptionLog log = wholth_entity_consumption_log_init();
    const wholth_Error err = wholth_em_consumption_log_delete(&log, NULL, buf);

    ASSERT_NE(wholth_Error_OK.code, err.code) << err.code << wfsv(err.message);
    ASSERT_NE(wholth_Error_OK.message.size, err.message.size)
        << err.code << wfsv(err.message);
    std::error_code ec = wholth::entity_manager::user::Code(err.code);
    ASSERT_EQ(wholth::entity_manager::user::Code::USER_NULL, ec)
        << ec << ec.message();

    std::string new_count{"bogus"};
    astmt(con, "SELECT COUNT(food_id) FROM consumption_log", [&](auto e) {
        new_count = e.column_value;
    });
    ASSERT_EQ(old_count, new_count);

    std::stringstream check;
    astmt(
        con,
        fmt::format(
            "SELECT food_id, mass, consumed_at FROM consumption_log "
            "WHERE id = {0}",
            id),
        [&](auto e) { check << e.column_value << ";"; });
    ASSERT_STREQ2("100;210;bogus;", check.str())
        << "Should be no changes to existing record";
}

TEST_F(Test_wholth_em_consumption_log_delete, when_buffer_is_nullptr)
{
    auto& con = db::connection();

    std::string id{"bogus"};
    astmt(
        con,
        "INSERT INTO consumption_log "
        "(food_id, user_id, mass, consumed_at) "
        "VALUES (100, 1, 210, 'bogus') "
        "RETURNING id",
        [&](auto e) { id = e.column_value; });
    ASSERT_STRNE2("bogus", id);

    std::string old_count{"bogus"};
    astmt(con, "SELECT COUNT(food_id) FROM consumption_log", [&](auto e) {
        old_count = e.column_value;
    });
    ASSERT_NE("bogus", old_count);

    wholth_ConsumptionLog log = wholth_entity_consumption_log_init();
    wholth_User user{};
    const wholth_Error err =
        wholth_em_consumption_log_delete(&log, &user, NULL);

    ASSERT_NE(wholth_Error_OK.code, err.code) << err.code << wfsv(err.message);
    ASSERT_NE(wholth_Error_OK.message.size, err.message.size)
        << err.code << wfsv(err.message);

    std::string new_count{"bogus"};
    astmt(con, "SELECT COUNT(food_id) FROM consumption_log", [&](auto e) {
        new_count = e.column_value;
    });
    ASSERT_EQ(old_count, new_count);

    std::stringstream check;
    astmt(
        con,
        fmt::format(
            "SELECT food_id, mass, consumed_at FROM consumption_log "
            "WHERE id = {0}",
            id),
        [&](auto e) { check << e.column_value << ";"; });
    ASSERT_STREQ2("100;210;bogus;", check.str())
        << "Should be no changes to existing record";
}

TEST_F(Test_wholth_em_consumption_log_delete, when_bad_id)
{
    auto& con = db::connection();

    std::string id{"bogus"};
    astmt(
        con,
        "INSERT INTO consumption_log "
        "(food_id, user_id, mass, consumed_at) "
        "VALUES (100, 1, 210, 'bogus') "
        "RETURNING id",
        [&](auto e) { id = e.column_value; });
    ASSERT_STRNE2("bogus", id);

    std::string old_count{"bogus"};
    astmt(con, "SELECT COUNT(food_id) FROM consumption_log", [&](auto e) {
        old_count = e.column_value;
    });
    ASSERT_NE("bogus", old_count);

    std::vector<wholth_ConsumptionLog> cases{{
        {},
        {.id = wtsv("")},
        {.id = wtsv("-1")},
        {.id = wtsv("12cde3")},
        {.id = wtsv("aboba")},
    }};
    for (const auto& log : cases)
    {
        wholth_Buffer* buf = wholth_buffer_ring_pool_element();
        wholth_User user{
            .id = wtsv("1"),
        };
        const wholth_Error err =
            wholth_em_consumption_log_delete(&log, &user, buf);

        ASSERT_NE(wholth_Error_OK.code, err.code)
            << err.code << wfsv(err.message);
        ASSERT_NE(wholth_Error_OK.message.size, err.message.size)
            << err.code << wfsv(err.message);
        std::error_code ec =
            wholth::entity_manager::consumption_log::Code(err.code);
        ASSERT_EQ(
            wholth::entity_manager::consumption_log::Code::
                CONSUMPTION_LOG_INVALID_ID,
            ec)
            << ec << ec.message();

        std::string new_count{"bogus"};
        astmt(con, "SELECT COUNT(food_id) FROM consumption_log", [&](auto e) {
            new_count = e.column_value;
        });
        ASSERT_EQ(old_count, new_count);

        std::stringstream check;
        astmt(
            con,
            fmt::format(
                "SELECT food_id, mass, consumed_at FROM consumption_log "
                "WHERE id = {0}",
                id),
            [&](auto e) { check << e.column_value << ";"; });
        ASSERT_STREQ2("100;210;bogus;", check.str())
            << "Should be no changes to existing record";
    }
}

TEST_F(Test_wholth_em_consumption_log_delete, when_bad_user_id)
{
    auto& con = db::connection();

    std::string id{"bogus"};
    astmt(
        con,
        "INSERT INTO consumption_log "
        "(food_id, user_id, mass, consumed_at) "
        "VALUES (100, 1, 210, 'bogus') "
        "RETURNING id",
        [&](auto e) { id = e.column_value; });
    ASSERT_STRNE2("bogus", id);

    std::string old_count{"bogus"};
    astmt(con, "SELECT COUNT(food_id) FROM consumption_log", [&](auto e) {
        old_count = e.column_value;
    });
    ASSERT_NE("bogus", old_count);

    std::vector<std::string_view> cases{{
        {},
        {""},
        {"-1"},
        {"12cde3"},
        {"aboba"},
    }};
    for (const auto& user_id : cases)
    {
        wholth_Buffer* buf = wholth_buffer_ring_pool_element();
        wholth_ConsumptionLog log{
            .id = wtsv(id),
            .food_id = wtsv("100"),
        };
        wholth_User user{
            .id = wtsv(user_id),
        };
        const wholth_Error err =
            wholth_em_consumption_log_delete(&log, &user, buf);

        ASSERT_NE(wholth_Error_OK.code, err.code)
            << err.code << wfsv(err.message);
        ASSERT_NE(wholth_Error_OK.message.size, err.message.size)
            << err.code << wfsv(err.message);
        std::error_code ec = wholth::entity_manager::user::Code(err.code);
        ASSERT_EQ(wholth::entity_manager::user::Code::USER_INVALID_ID, ec)
            << ec << ec.message();

        std::string new_count{"bogus"};
        astmt(con, "SELECT COUNT(food_id) FROM consumption_log", [&](auto e) {
            new_count = e.column_value;
        });
        ASSERT_EQ(old_count, new_count);

        std::stringstream check;
        astmt(
            con,
            fmt::format(
                "SELECT food_id, mass, consumed_at FROM consumption_log "
                "WHERE id = {0}",
                id),
            [&](auto e) { check << e.column_value << ";"; });
        ASSERT_STREQ2("100;210;bogus;", check.str())
            << "Should be no changes to existing record";
    }
}

TEST_F(Test_wholth_em_consumption_log_delete, when_wrong_user)
{
    auto& con = db::connection();

    std::string id_check;
    astmt(con, "SELECT id FROM food where id = 100", [&](auto e) {
        id_check = e.column_value;
    });
    ASSERT_EQ("100", id_check) << "Couldn't find food with id 100!";

    std::string id_1{"bogus"};
    astmt(
        con,
        "INSERT INTO consumption_log "
        "(food_id, user_id, mass, consumed_at) "
        "VALUES (100, 1, 110, 'bogus') "
        "RETURNING id",
        [&](auto e) { id_1 = e.column_value; });
    ASSERT_STRNE2("bogus", id_1);

    std::string id_2{"bogus"};
    astmt(
        con,
        "INSERT INTO consumption_log "
        "(food_id, user_id, mass, consumed_at) "
        "VALUES (100, 2, 210, 'bogus') "
        "RETURNING id",
        [&](auto e) { id_2 = e.column_value; });
    ASSERT_STRNE2("bogus", id_2);

    std::string old_count{"bogus"};
    astmt(con, "SELECT COUNT(food_id) FROM consumption_log", [&](auto e) {
        old_count = e.column_value;
    });
    ASSERT_NE("bogus", old_count);

    wholth_ConsumptionLog log = wholth_entity_consumption_log_init();
    log.id = wtsv(id_1);
    wholth_Buffer* buf = wholth_buffer_ring_pool_element();
    wholth_User user{
        .id = wtsv("2"),
    };
    const wholth_Error err = wholth_em_consumption_log_delete(&log, &user, buf);

    ASSERT_EQ(wholth_Error_OK.code, err.code) << err.code << wfsv(err.message);
    ASSERT_EQ(wholth_Error_OK.message.size, err.message.size)
        << err.code << wfsv(err.message);

    std::string new_count{"bogus"};
    astmt(con, "SELECT COUNT(food_id) FROM consumption_log", [&](auto e) {
        new_count = e.column_value;
    });
    ASSERT_EQ(old_count, new_count);

    std::stringstream check;
    astmt(
        con,
        fmt::format(
            "SELECT food_id, mass, consumed_at FROM consumption_log "
            "WHERE id = {0}",
            id_1),
        [&](auto e) { check << e.column_value << ";"; });
    ASSERT_STREQ2("100;110;bogus;", check.str());

    check = {};
    astmt(
        con,
        fmt::format(
            "SELECT food_id, mass, consumed_at FROM consumption_log "
            "WHERE id = {0}",
            id_2),
        [&](auto e) { check << e.column_value << ";"; });
    ASSERT_STREQ2("100;210;bogus;", check.str());
}

TEST_F(Test_wholth_em_consumption_log_delete, when_good_case)
{
    auto& con = db::connection();

    std::string id_check;
    astmt(con, "SELECT id FROM food where id = 100", [&](auto e) {
        id_check = e.column_value;
    });
    ASSERT_EQ("100", id_check) << "Couldn't find food with id 100!";

    std::string id{"bogus"};
    astmt(
        con,
        "INSERT INTO consumption_log "
        "(food_id, user_id, mass, consumed_at) "
        "VALUES (100, 1, 210, 'bogus') "
        "RETURNING id",
        [&](auto e) { id = e.column_value; });
    ASSERT_STRNE2("bogus", id);

    std::string old_count{"bogus"};
    astmt(con, "SELECT COUNT(food_id) FROM consumption_log", [&](auto e) {
        old_count = e.column_value;
    });
    ASSERT_NE("bogus", old_count);

    wholth_ConsumptionLog log = wholth_entity_consumption_log_init();
    log.id = wtsv(id);
    wholth_Buffer* buf = wholth_buffer_ring_pool_element();
    wholth_User user{
        .id = wtsv("1"),
    };
    const wholth_Error err = wholth_em_consumption_log_delete(&log, &user, buf);

    ASSERT_EQ(wholth_Error_OK.code, err.code) << err.code << wfsv(err.message);
    ASSERT_EQ(wholth_Error_OK.message.size, err.message.size)
        << err.code << wfsv(err.message);

    std::string new_count{"bogus"};
    astmt(con, "SELECT COUNT(food_id) FROM consumption_log", [&](auto e) {
        new_count = e.column_value;
    });
    ASSERT_NE(old_count, new_count);

    std::stringstream check;
    astmt(
        con,
        fmt::format(
            "SELECT food_id, mass, consumed_at FROM consumption_log "
            "WHERE id = {0}",
            id),
        [&](auto e) { check << e.column_value << ";"; });
    ASSERT_STREQ2("", check.str()) << "Should be no changes to existing record";
}
