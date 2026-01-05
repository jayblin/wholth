#include "db/db.hpp"
#include "sqlw/statement.hpp"
#include "wholth/c/buffer.h"
#include "wholth/c/entity_manager/user.h"
#include "wholth/c/error.h"
#include "wholth/c/internal.hpp"
#include "wholth/entity_manager/user.hpp"
#include "wholth/utils/is_valid_id.hpp"
#include "wholth/utils/to_string_view.hpp"
#include <cassert>
#include <cstdlib>
#include <memory>
#include <openssl/sha.h>
#include <openssl/core_names.h>
#include <openssl/params.h>
#include <openssl/thread.h>
#include <openssl/kdf.h>
#include <sstream>
#include <string_view>

using wholth::c::internal::ec_to_error;
using wholth::entity_manager::user::Code;
using wholth::utils::is_valid_id;
using wholth::utils::to_string_view;

struct HashHexed
{
    static constexpr auto size = 2 * SHA256_DIGEST_LENGTH;
    char represenation[size];
};

static std::error_code sha256_string(HashHexed& hex, std::string_view value)
{
    unsigned char hash[SHA256_DIGEST_LENGTH];

    SHA256(
        reinterpret_cast<const unsigned char*>(value.data()),
        value.size(),
        hash);

    // thanks
    // https://github.com/vikramls/openssl_examples/blob/master/crypto.cpp#L347
    // Convert md to hex string.
    for (size_t i = 0; i < SHA256_DIGEST_LENGTH; i++)
    {
        // todo check hash[i] is valid???
        const int r = hash[i] % 16;
        const int l = (hash[i] - r) / 16;

        // @see https://www.ascii-code.com/
        hex.represenation[2 * i] = '0' + l;
        if (l > 9)
        {
            hex.represenation[2 * i] += 7;
        }

        hex.represenation[2 * i + 1] = '0' + r;
        if (r > 9)
        {
            hex.represenation[2 * i + 1] += 7;
        }
    }

    return {};
}

/**
 * @see https://www.rfc-editor.org/rfc/rfc9106.html#section-4-6.2
 *
 * If much less memory is available, a uniformly safe option is Argon2id
 * with t=3 iterations, p=4 lanes, m=2^(16) (64 MiB of RAM), 128-bit salt,
 * and 256-bit tag size. This is the SECOND RECOMMENDED option.
 */
struct Argon2Context
{
    EVP_KDF* kdf = nullptr;
    EVP_KDF_CTX* kctx = nullptr;
    OSSL_PARAM params[6];
    uint32_t iter = 3;
    uint32_t lanes = 4;
    uint32_t memcost_1kb_blocks = 64 * 1024;
    // uint32_t threads = 1;
    static constexpr size_t outlen = 256;
    unsigned char result[outlen];
};

/**
 * @see https://docs.openssl.org/3.2/man7/EVP_KDF-ARGON2/
 */
struct Argon2
{
    Argon2Context ctx;

    Argon2(
        Argon2Context&& context,
        std::string_view password,
        std::string_view salt)
        : ctx(context)
    {
        // /* required if threads > 1 */
        // if (OSSL_set_max_threads(NULL, threads) != 1)
        //     goto fail;

        OSSL_PARAM* p = ctx.params;
        *p++ = OSSL_PARAM_construct_uint32(OSSL_KDF_PARAM_ITER, &ctx.iter);
        *p++ = OSSL_PARAM_construct_uint32(
            OSSL_KDF_PARAM_ARGON2_LANES, &ctx.lanes);
        *p++ = OSSL_PARAM_construct_uint32(
            OSSL_KDF_PARAM_ARGON2_MEMCOST, &ctx.memcost_1kb_blocks);
        // *p++ =
        //     OSSL_PARAM_construct_uint32(OSSL_KDF_PARAM_THREADS,
        //     &ctx.threads);
        *p++ = OSSL_PARAM_construct_octet_string(
            OSSL_KDF_PARAM_SALT, (void*)salt.data(), salt.size());
        *p++ = OSSL_PARAM_construct_octet_string(
            OSSL_KDF_PARAM_PASSWORD, (void*)password.data(), password.size());
        *p++ = OSSL_PARAM_construct_end();
    }

    ~Argon2()
    {
        EVP_KDF_free(ctx.kdf);
        EVP_KDF_CTX_free(ctx.kctx);
        // OSSL_set_max_threads(nullptr, 0);
    }

    [[nodiscard]]
    std::pair<std::string, std::error_code> result()
    {
        ctx.kdf = EVP_KDF_fetch(NULL, "ARGON2D", NULL);

        if (nullptr == ctx.kdf)
        {
            return {{}, Code::USER_AUTHENTICATION_FAILED_DRAGOON};
        }

        ctx.kctx = EVP_KDF_CTX_new(ctx.kdf);

        if (nullptr == ctx.kctx)
        {
            return {{}, Code::USER_AUTHENTICATION_FAILED_CHOOMBA};
        }

        if (EVP_KDF_derive(ctx.kctx, &ctx.result[0], ctx.outlen, ctx.params) !=
            1)
        {
            return {{}, Code::USER_AUTHENTICATION_FAILED_CHROME};
        }

        size_t strlength;
        OPENSSL_buf2hexstr_ex(
            nullptr, 10, &strlength, &ctx.result[0], ctx.outlen, '\0');

        if (513 != strlength)
        {
            return {{}, Code::USER_AUTHENTICATION_FAILED_PREEM};
        }

        std::string result_hex(strlength, '0');

        const auto res = OPENSSL_buf2hexstr_ex(
            result_hex.data(),
            result_hex.size(),
            nullptr,
            &ctx.result[0],
            ctx.outlen,
            '\0');

        if (1 != res)
        {
            return {{}, Code::USER_AUTHENTICATION_FAILED_KLEPPED};
        }

        return {{std::move(result_hex), 0, result_hex.size() - 1}, {}};
    }
};

static std::pair<std::string, std::error_code> hash_password(
    std::string_view password)
{
    // const char* salt = std::getenv("WHOLTH_PASSWORD_SALT");
    const auto salt =
        wholth::c::internal::global_context().password_encryption_secret;

    // if (nullptr == salt)
    if (salt.empty())
    {
        return {{}, Code::USER_AUTHENTICATION_FAILED_GONKED};
    }

    // std::stringstream ss;
    // ss << salt << password;
    // const auto salted = ss.str();
    //
    // return sha256_string(hash, salted);

    Argon2 argon2 = {Argon2Context{}, password, salt};

    return argon2.result();
}

namespace wholth::entity_manager::user
{

struct ErrorCategory : std::error_category
{
    const char* name() const noexcept override final
    {
        return "entity_manager::user";
    }

    std::string message(int ev) const override final
    {
        switch (static_cast<Code>(ev))
        {
        case USER_NULL:
            return "USER_NULL";
        case USER_INVALID_ID:
            return "USER_INVALID_ID";
        case USER_INVALID_LOCALE_ID:
            return "USER_INVALID_LOCALE_ID";
        case USER_AUTHENTICATION_FAILED:
            return "USER_AUTHENTICATION_FAILED";
        case USER_UNAUTHORIZED:
            return "USER_UNAUTHORIZED";
        case USER_PASSWORD_TOO_SHORT:
            return "USER_PASSWORD_TOO_SHORT";
        case USER_PASSWORD_TOO_LONG:
            return "USER_PASSWORD_TOO_LONG";
        case USER_NO_NAME:
            return "USER_NO_NAME";
        case USER_AUTHENTICATION_FAILED_GONKED:
            return "USER_NO_SALT";
        case _USER_COUNT_:
            return "not an error";
        case USER_AUTHENTICATION_FAILED_DRAGOON:
            return "USER_AUTHENTICATION_FAILED_DRAGOON";
        case USER_AUTHENTICATION_FAILED_CHOOMBA:
            return "USER_AUTHENTICATION_FAILED_CHOOMBA";
        case USER_AUTHENTICATION_FAILED_CHROME:
            return "USER_AUTHENTICATION_FAILED_CHROME";
        case USER_AUTHENTICATION_FAILED_PREEM:
            return "USER_AUTHENTICATION_FAILED_PREEM";
        case USER_AUTHENTICATION_FAILED_KLEPPED:
            return "USER_AUTHENTICATION_FAILED_KLEPPED";
        case USER_DOES_NOT_EXIST:
            return "USER_DOES_NOT_EXIST";
        }

        return "(unrecognized error)";
    }
};

const ErrorCategory error_category{};
} // namespace wholth::entity_manager::user

std::error_code make_error_code(wholth::entity_manager::user::Code e)
{
    return {static_cast<int>(e), wholth::entity_manager::user::error_category};
}

extern "C" wholth_Error wholth_em_user_update(
    const wholth_User* const user,
    wholth_Buffer* const buffer)
{
    if (nullptr == buffer)
    {
        return wholth::c::internal::bad_buffer_error();
    }

    if (nullptr == user)
    {
        return ec_to_error(Code::USER_NULL, buffer);
    }

    const auto user_id = to_string_view(user->id);
    if (!is_valid_id(user_id))
    {
        return ec_to_error(Code::USER_INVALID_ID, buffer);
    }

    const auto locale_id = to_string_view(user->locale_id);

    if (!is_valid_id(locale_id))
    {
        return ec_to_error(Code::USER_INVALID_LOCALE_ID, buffer);
    }

    const auto ec = (sqlw::Statement{&db::connection()})(
        "UPDATE user SET locale_id = ?1 WHERE id = ?2",
        std::array<sqlw::Statement::bindable_t, 2>{
            {{locale_id, sqlw::Type::SQL_INT},
             {user_id, sqlw::Type::SQL_INT}}});

    if (sqlw::status::Condition::OK != ec)
    {
        return ec_to_error(ec, buffer);
    }

    return wholth_Error_OK;
}

extern "C" wholth_Error wholth_em_user_locale_id(
    const wholth_StringView user_id,
    const wholth_StringView locale_id,
    wholth_Buffer* const buffer)
{
    const wholth_User user{
        .id = user_id,
        .locale_id = locale_id,
    };
    return wholth_em_user_update(&user, buffer);
}

extern "C" wholth_Error wholth_em_user_insert(
    wholth_User* const user,
    const wholth_StringView password,
    wholth_Buffer* const buffer)
{
    if (nullptr == buffer)
    {
        return wholth::c::internal::bad_buffer_error();
    }

    std::error_code err;
    std::string_view _password;
    std::string_view name;
    std::string password_hashed;

    if (nullptr == user)
    {
        err = Code::USER_NULL;
    }
    else if (name = to_string_view(user->name); name.empty())
    {
        err = Code::USER_NO_NAME;
    }
    else if (_password = to_string_view(password); _password.size() < 16)
    {
        err = Code::USER_PASSWORD_TOO_SHORT;
    }
    else if (_password.size() > 256)
    {
        err = Code::USER_PASSWORD_TOO_LONG;
    }
    else
    {
        auto res = hash_password(_password);
        password_hashed = std::move(res.first);
        err = std::move(res.second);
    }

    if (err)
    {
        return ec_to_error(err, buffer);
    }

    // const std::string_view password_hashed{hash.represenation, hash.size};

    std::stringstream ss{};
    const auto ec = (sqlw::Statement{&db::connection()})(
        "INSERT INTO user (name, password_hashed, locale_id) "
        "VALUES (trim(?1), ?2, ?3) "
        "RETURNING id",
        [&ss](sqlw::Statement::ExecArgs e) { ss << e.column_value; },
        std::array<sqlw::Statement::bindable_t, 3>{
            {{name, sqlw::Type::SQL_TEXT},
             {password_hashed, sqlw::Type::SQL_TEXT},
             {to_string_view(user->locale_id), sqlw::Type::SQL_INT}}});

    // todo add wait 500ms for security

    if (sqlw::status::Condition::OK != ec)
    {
        return ec_to_error(ec, buffer);
    }

    std::string id = ss.str();
    wholth_buffer_move_data_to(buffer, &id);
    user->id = wholth_buffer_view(buffer);

    return wholth_Error_OK;
}

extern "C" wholth_Error wholth_em_user_exists(
    const wholth_StringView name,
    wholth_StringView* const id,
    wholth_Buffer* const buffer)
{
    if (nullptr == buffer)
    {
        return wholth::c::internal::bad_buffer_error();
    }

    if (nullptr == id)
    {
        return wholth::c::internal::bad_buffer_error();
    }

    std::error_code err;

    std::string _id{};
    err = (sqlw::Statement{&db::connection()})(
        "SELECT id "
        "FROM user "
        "WHERE name = ?1",
        [&_id](sqlw::Statement::ExecArgs e) { _id = e.column_value; },
        std::array<sqlw::Statement::bindable_t, 1>{{
            {to_string_view(name), sqlw::Type::SQL_TEXT},
        }});

    if (sqlw::status::Condition::OK != err)
    {
        return ec_to_error(err, buffer);
    }

    if (_id.empty())
    {
        return ec_to_error(Code::USER_DOES_NOT_EXIST, buffer);
    }

    wholth_buffer_move_data_to(buffer, &_id);

    *id = wholth_buffer_view(buffer);

    return wholth_Error_OK;
}

extern "C" wholth_Error wholth_em_user_authenticate(
    wholth_User* const user,
    const wholth_StringView password,
    wholth_Buffer* const buffer)
{
    if (nullptr == buffer)
    {
        return wholth::c::internal::bad_buffer_error();
    }

    std::error_code err;
    std::string_view _password;
    std::string password_hashed;

    if (nullptr == user)
    {
        err = Code::USER_NULL;
    }
    else if (_password = to_string_view(password); _password.empty())
    {
        err = Code::USER_AUTHENTICATION_FAILED;
    }
    else if (_password.size() > 256)
    {
        err = Code::USER_AUTHENTICATION_FAILED;
    }
    else
    {
        auto res = hash_password(_password);
        password_hashed = std::move(res.first);
        err = std::move(res.second);
    }

    if (err)
    {
        return ec_to_error(err, buffer);
    }

    // todo add wait 500ms for security

    std::string id{};
    err = (sqlw::Statement{&db::connection()})(
        "SELECT id "
        "FROM user "
        "WHERE name = ?1 AND password_hashed = ?2",
        [&id](sqlw::Statement::ExecArgs e) { id = e.column_value; },
        std::array<sqlw::Statement::bindable_t, 2>{
            {{to_string_view(user->name), sqlw::Type::SQL_TEXT},
             {password_hashed, sqlw::Type::SQL_TEXT}}});

    if (sqlw::status::Condition::OK != err)
    {
        return ec_to_error(err, buffer);
    }

    if (id.empty())
    {
        return ec_to_error(Code::USER_AUTHENTICATION_FAILED, buffer);
    }

    wholth_buffer_move_data_to(buffer, &id);
    user->id = wholth_buffer_view(buffer);

    return wholth_Error_OK;
}
