#ifndef WHOLTH_CONTEXT_H_
#define WHOLTH_CONTEXT_H_

#include "db/db.hpp"
#include "utils/serializer.hpp"

namespace wholth
{
    struct Context
    {
        db::migration::MigrateResult migrate_result;
        std::vector<std::string> sql_errors;
        std::string exception_message;

        template <typename Serializer>
        auto serialize(Serializer& serializer) const noexcept -> void
        {
            serializer << NVP(migrate_result)
                << NVP(sql_errors)
                << NVP(exception_message)
                ;
        }
    };
}

#endif // WHOLTH_CONTEXT_H_
