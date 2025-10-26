#include "db/db.hpp"
#include "fmt/core.h"
#include "helpers.hpp"
#include "sqlw/statement.hpp"
#include "wholth/c/buffer.h"
#include "wholth/c/entity/consumption_log.h"
#include "wholth/c/entity/user.h"
#include "wholth/c/entity_manager/consumption_log.h"
#include "wholth/entity_manager/consumption_log.hpp"
#include "wholth/entity_manager/user.hpp"
#include <string>
#include <unistd.h>
#include <vector>

static_assert(nullptr == (void*)NULL);

using bind_t = sqlw::Statement::bindable_t;

class Test_wholth_em_consumption_log_insert : public ApplicationAwareTest
{
};

TEST_F(Test_wholth_em_consumption_log_insert, when_log_is_nullptr)
{
    auto& con = db::connection();

    std::string old_count{"bogus"};
    astmt(con, "SELECT COUNT(food_id) FROM consumption_log", [&](auto e) {
        old_count = e.column_value;
    });
    ASSERT_NE("bogus", old_count);

    wholth_Buffer* buf = wholth_buffer_ring_pool_element();
    wholth_User user{};
    const wholth_Error err = wholth_em_consumption_log_insert(NULL, &user, buf);

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
}

TEST_F(Test_wholth_em_consumption_log_insert, when_user_is_nullptr)
{
    auto& con = db::connection();

    std::string old_count{"bogus"};
    astmt(con, "SELECT COUNT(food_id) FROM consumption_log", [&](auto e) {
        old_count = e.column_value;
    });
    ASSERT_NE("bogus", old_count);

    wholth_Buffer* buf = wholth_buffer_ring_pool_element();
    wholth_ConsumptionLog log = wholth_entity_consumption_log_init();
    const wholth_Error err = wholth_em_consumption_log_insert(&log, NULL, buf);

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
}

TEST_F(Test_wholth_em_consumption_log_insert, when_buffer_is_nullptr)
{
    auto& con = db::connection();

    std::string old_count{"bogus"};
    astmt(con, "SELECT COUNT(food_id) FROM consumption_log", [&](auto e) {
        old_count = e.column_value;
    });
    ASSERT_NE("bogus", old_count);

    wholth_ConsumptionLog log = wholth_entity_consumption_log_init();
    wholth_User user{};
    const wholth_Error err =
        wholth_em_consumption_log_insert(&log, &user, NULL);

    ASSERT_NE(wholth_Error_OK.code, err.code) << err.code << wfsv(err.message);
    ASSERT_NE(wholth_Error_OK.message.size, err.message.size)
        << err.code << wfsv(err.message);

    ASSERT_EQ(nullptr, log.id.data);
    ASSERT_EQ(0, log.id.size);

    std::string new_count{"bogus"};
    astmt(con, "SELECT COUNT(food_id) FROM consumption_log", [&](auto e) {
        new_count = e.column_value;
    });
    ASSERT_EQ(old_count, new_count);
}

TEST_F(Test_wholth_em_consumption_log_insert, when_bad_food_id)
{
    auto& con = db::connection();

    std::string old_count{"bogus"};
    astmt(con, "SELECT COUNT(food_id) FROM consumption_log", [&](auto e) {
        old_count = e.column_value;
    });
    ASSERT_NE("bogus", old_count);

    std::vector<wholth_ConsumptionLog> cases{{
        {},
        {.food_id = wtsv("")},
        {.food_id = wtsv("-1")},
        {.food_id = wtsv("12cde3")},
        {.food_id = wtsv("aboba")},
    }};
    for (auto& log : cases)
    {
        wholth_Buffer* buf = wholth_buffer_ring_pool_element();
        wholth_User user{
            .id = wtsv("1"),
        };
        const wholth_Error err =
            wholth_em_consumption_log_insert(&log, &user, buf);

        ASSERT_NE(wholth_Error_OK.code, err.code)
            << err.code << wfsv(err.message);
        ASSERT_NE(wholth_Error_OK.message.size, err.message.size)
            << err.code << wfsv(err.message);
        std::error_code ec =
            wholth::entity_manager::consumption_log::Code(err.code);
        ASSERT_EQ(
            wholth::entity_manager::consumption_log::Code::
                CONSUMPTION_LOG_INVALID_FOOD_ID,
            ec)
            << ec << ec.message();

        ASSERT_EQ(nullptr, log.id.data);
        ASSERT_EQ(0, log.id.size);

        std::string new_count{"bogus"};
        astmt(con, "SELECT COUNT(food_id) FROM consumption_log", [&](auto e) {
            new_count = e.column_value;
        });
        ASSERT_EQ(old_count, new_count);
    }
}

TEST_F(Test_wholth_em_consumption_log_insert, when_bad_mass)
{
    auto& con = db::connection();

    std::string old_count{"bogus"};
    astmt(con, "SELECT COUNT(food_id) FROM consumption_log", [&](auto e) {
        old_count = e.column_value;
    });
    ASSERT_NE("bogus", old_count);

    wholth_ConsumptionLog log = wholth_entity_consumption_log_init();
    log.food_id = wtsv("1");
    std::vector<std::string_view> cases{{
        {},
        "",
        "100g",
        "-3",
        "aboba",
        "-0.33",
        "0,33",
    }};
    for (const auto& mass : cases)
    {
        wholth_Buffer* buf = wholth_buffer_ring_pool_element();
        wholth_User user{
            .id = wtsv("1"),
        };
        log.mass = wtsv(mass);
        const wholth_Error err =
            wholth_em_consumption_log_insert(&log, &user, buf);

        ASSERT_NE(wholth_Error_OK.code, err.code)
            << err.code << wfsv(err.message);
        ASSERT_NE(wholth_Error_OK.message.size, err.message.size)
            << err.code << wfsv(err.message);
        std::error_code ec =
            wholth::entity_manager::consumption_log::Code(err.code);
        ASSERT_EQ(
            wholth::entity_manager::consumption_log::Code::
                CONSUMPTION_LOG_INVALID_MASS,
            ec)
            << ec << ec.message();

        ASSERT_EQ(nullptr, log.id.data);
        ASSERT_EQ(0, log.id.size);

        std::string new_count{"bogus"};
        astmt(con, "SELECT COUNT(food_id) FROM consumption_log", [&](auto e) {
            new_count = e.column_value;
        });
        ASSERT_EQ(old_count, new_count);
    }
}

TEST_F(Test_wholth_em_consumption_log_insert, when_bad_consumed_at)
{
    auto& con = db::connection();

    std::string old_count{"bogus"};
    astmt(con, "SELECT COUNT(food_id) FROM consumption_log", [&](auto e) {
        old_count = e.column_value;
    });
    ASSERT_NE("bogus", old_count);

    wholth_ConsumptionLog log = wholth_entity_consumption_log_init();
    log.food_id = wtsv("1");
    log.mass = wtsv("210");
    std::vector<std::string_view> cases{{
        {},
        "",
        "aboba",
        "100",
        "2025-13-22T01:01:01",
        "0,9",
        "-0.88",
    }};
    for (const auto& consumed_at : cases)
    {
        wholth_Buffer* buf = wholth_buffer_ring_pool_element();
        wholth_User user{
            .id = wtsv("1"),
        };
        log.consumed_at = wtsv(consumed_at);
        const wholth_Error err =
            wholth_em_consumption_log_insert(&log, &user, buf);

        ASSERT_NE(wholth_Error_OK.code, err.code)
            << err.code << wfsv(err.message);
        ASSERT_NE(wholth_Error_OK.message.size, err.message.size)
            << err.code << wfsv(err.message);
        std::error_code ec =
            wholth::entity_manager::consumption_log::Code(err.code);
        ASSERT_EQ(
            wholth::entity_manager::consumption_log::Code::
                CONSUMPTION_LOG_INVALID_DATE,
            ec)
            << ec << ec.message();

        ASSERT_EQ(nullptr, log.id.data);
        ASSERT_EQ(0, log.id.size);

        std::string new_count{"bogus"};
        astmt(con, "SELECT COUNT(food_id) FROM consumption_log", [&](auto e) {
            new_count = e.column_value;
        });
        ASSERT_EQ(old_count, new_count);
    }
}

TEST_F(Test_wholth_em_consumption_log_insert, when_bad_user_id)
{
    auto& con = db::connection();

    std::string old_count{"bogus"};
    astmt(con, "SELECT COUNT(food_id) FROM consumption_log", [&](auto e) {
        old_count = e.column_value;
    });
    ASSERT_NE("bogus", old_count);

    wholth_ConsumptionLog log = wholth_entity_consumption_log_init();
    log.food_id = wtsv("1");
    log.mass = wtsv("102.3");
    log.consumed_at = wtsv("9999-01-22T13:14:55");
    std::vector<std::string_view> cases{{
        {},
        "",
        "100g",
        "-3",
        "aboba",
        "-0.33",
        "0,33",
    }};
    for (const auto& user_id : cases)
    {
        wholth_Buffer* buf = wholth_buffer_ring_pool_element();
        wholth_User user{
            .id = wtsv(user_id),
        };
        const wholth_Error err =
            wholth_em_consumption_log_insert(&log, &user, buf);

        ASSERT_NE(wholth_Error_OK.code, err.code)
            << err.code << wfsv(err.message);
        ASSERT_NE(wholth_Error_OK.message.size, err.message.size)
            << err.code << wfsv(err.message);
        std::error_code ec = wholth::entity_manager::user::Code(err.code);
        ASSERT_EQ(wholth::entity_manager::user::Code::USER_INVALID_ID, ec)
            << ec << ec.message();

        ASSERT_EQ(nullptr, log.id.data);
        ASSERT_EQ(0, log.id.size);

        std::string new_count{"bogus"};
        astmt(con, "SELECT COUNT(food_id) FROM consumption_log", [&](auto e) {
            new_count = e.column_value;
        });
        ASSERT_EQ(old_count, new_count);
    }
}

TEST_F(Test_wholth_em_consumption_log_insert, when_good_case)
{
    auto& con = db::connection();

    std::string id_check;
    astmt(con, "SELECT id FROM food where id = 100", [&](auto e) {
        id_check = e.column_value;
    });
    ASSERT_EQ("100", id_check) << "Couldn't find food with id 100!";

    std::string old_count{"bogus"};
    astmt(con, "SELECT COUNT(food_id) FROM consumption_log", [&](auto e) {
        old_count = e.column_value;
    });
    ASSERT_NE("bogus", old_count);

    astmt(
        db::connection(),
        "INSERT OR REPLACE INTO user "
        " (id, name, password_hashed, locale_id) VALUES "
        " (1, 'test', 'standalone', 1),"
        " (2, 'test-2', 'standalone', 1);");

    wholth_ConsumptionLog log = wholth_entity_consumption_log_init();
    log.food_id = wtsv("100");
    log.mass = wtsv("102.3");
    log.consumed_at = wtsv("9999-01-22T13:14:55");
    wholth_Buffer* buf = wholth_buffer_ring_pool_element();
    wholth_User user{
        .id = wtsv("1"),
    };
    const wholth_Error err = wholth_em_consumption_log_insert(&log, &user, buf);

    ASSERT_EQ(wholth_Error_OK.code, err.code) << err.code << wfsv(err.message);
    ASSERT_EQ(wholth_Error_OK.message.size, err.message.size)
        << err.code << wfsv(err.message);

    ASSERT_TRUE(nullptr != log.id.data);
    ASSERT_TRUE(log.id.size > 0);

    std::string new_count{"bogus"};
    astmt(con, "SELECT COUNT(food_id) FROM consumption_log", [&](auto e) {
        new_count = e.column_value;
    });
    ASSERT_NE(old_count, new_count);

    std::stringstream ss;
    astmt(
        con,
        fmt::format(
            "SELECT food_id, user_id, mass, consumed_at "
            "FROM consumption_log "
            "WHERE id = {0}",
            wfsv(log.id)),
        [&](auto e) { ss << e.column_value << ";"; });
    const auto ent = ss.str();
    ASSERT_STREQ2("100;1;102.3;9999-01-22T13:14:55;", ent);
}
