#ifndef WHOLTH_SERIALIZATION_DEFINITIONS_H_
#define WHOLTH_SERIALIZATION_DEFINITIONS_H_

#include "db/db.hpp"
#include "ui/style.hpp"
#include "utils/serializer.hpp"
#include "wholth/buffer_view.hpp"
#include "wholth/context.hpp"
#include "wholth/entity/food.hpp"
#include "wholth/model/expanded_food.hpp"
#include "wholth/model/foods_page.hpp"
#include "wholth/swappable.hpp"
#include "wholth/task_list.hpp"
#include <type_traits>

template <> struct serialization::Serializable<wholth::Context> : std::true_type
{
    const wholth::Context& o;

    template <typename Serializer> void serialize(Serializer& serializer)
    {
        serializer //
            << ONVP(connection.status())
            /* << NVP(style) */   // this segfaults!
            << ONVP(locale_id())  //
            << ONVP(task_list())  //
            << ONVP(foods_page()) //
            /* << NVP(m_task_mutex) */
            << ONVP(migrate_result)    //
            << ONVP(sql_errors)        //
            << ONVP(exception_message) //
            ;
    }
};

template <> struct serialization::Serializable<wholth::Page> : std::true_type
{
    const wholth::Page& o;
    template <typename Serializer> inline void serialize(Serializer& serializer)
    {
        serializer                  //
            << ONVP(current_page()) //
            << ONVP(count())        //
            << ONVP(pagination())   //
            ;
    }
};

template <>
struct serialization::Serializable<db::migration::MigrateResult>
    : std::true_type
{
    const db::migration::MigrateResult& o;
    template <typename Serializer> inline void serialize(Serializer& serializer)
    {
        serializer                         //
            << ONVP(error_code)            //
            << ONVP(executed_migrations)   //
            << ONVP(problematic_migration) //
            ;
    }
};

template <>
struct serialization::Serializable<wholth::model::FoodsPage> : std::true_type
{
    const wholth::model::FoodsPage& o;

    template <typename Serializer> inline void serialize(Serializer& serializer)
    {
        serializer                    //
            << ONVP(title_buffer)     //
            << ONVP(title_input_size) //
            << ONVP(page)             //
            << ONVP(swappable_list)   //
            /* << ONVP(swappable_list.view_current().buffer) // */
            /* << ONVP(swappable_list.view_next().buffer)    // */
            << ONVP(input_timer.count()) //
            << ONVP(is_fetching.load())  //
            << ONVP(expanded_food)       //
            ;
    }
};

template <>
struct serialization::Serializable<wholth::model::ExpandedFood> : std::true_type
{
    const wholth::model::ExpandedFood& o;
    template <typename Serializer> inline void serialize(Serializer& serializer)
    {
        serializer               //
            << ONVP(should_show) //
            << ONVP(food)        //
            << ONVP(food_buffer) //
            /* << NVP(std::span( */
            /*        nutrients.begin(), */
            /*        nutrients.begin() + 20)) // */
            << ONVP(nutrients_buffer) //
            << ONVP(steps)            //
            << ONVP(steps_buffer)     //
            ;
    }
};

template <>
struct serialization::Serializable<wholth::entity::expanded::food::RecipeStep>
    : std::true_type
{
    const wholth::entity::expanded::food::RecipeStep& o;
    template <typename Serializer> inline void serialize(Serializer& serializer)
    {
        serializer               //
            << ONVP(id)          //
            << ONVP(time)        //
            << ONVP(description) //
            ;
    }
};

template <>
struct serialization::Serializable<wholth::entity::expanded::food::Ingredient>
    : std::true_type
{
    const wholth::entity::expanded::food::Ingredient& o;
    template <typename Serializer> inline void serialize(Serializer& serializer)
    {
        serializer                    //
            << ONVP(food_id)          //
            << ONVP(title)            //
            << ONVP(canonical_mass)   //
            << ONVP(ingredient_count) //
            ;
    }
};

template <>
struct serialization::Serializable<wholth::entity::expanded::food::Nutrient>
    : std::true_type
{
    const wholth::entity::expanded::food::Nutrient& o;
    template <typename Serializer> inline void serialize(Serializer& serializer)
    {
        serializer              //
            << ONVP(id)         //
            << ONVP(title)      //
            << ONVP(value)      //
            << ONVP(unit)       //
            << ONVP(user_value) //
            ;
    }
};

template <>
struct serialization::Serializable<wholth::entity::expanded::Food>
    : std::true_type
{
    const wholth::entity::expanded::Food& o;
    template <typename Serializer> inline void serialize(Serializer& serializer)
    {
        serializer                    //
            << ONVP(id)               //
            << ONVP(title)            //
            << ONVP(description)      //
            << ONVP(preparation_time) //
            ;
    }
};

template <>
struct serialization::Serializable<wholth::entity::shortened::Food>
    : std::true_type
{
    const wholth::entity::shortened::Food& o;
    template <typename Serializer> inline void serialize(Serializer& serializer)
    {
        serializer                    //
            << ONVP(id)               //
            << ONVP(title)            //
            << ONVP(preparation_time) //
            << ONVP(top_nutrient)     //
            ;
    }
};

template <> struct serialization::Serializable<ui::Style> : std::true_type
{
    const ui::Style& o;
    template <typename Serializer> inline void serialize(Serializer& serializer)
    {
        // todo check for nullptr
        serializer                                  //
            << ONVP(font_normal->ConfigData->Name)  //
            << ONVP(font_heading->ConfigData->Name) //
            ;
    }
};

template <>
struct serialization::Serializable<wholth::TaskList> : std::true_type
{
    const wholth::TaskList& o;
    template <typename Serializer> inline void serialize(Serializer& serializer)
    {
        serializer          //
            << ONVP(mask()) //
            ;
    }
};

template <typename T>
struct serialization::Serializable<wholth::Swappable<T>> : std::true_type
{
    const wholth::Swappable<T>& o;
    template <typename Serializer> inline void serialize(Serializer& serializer)
    {
        serializer                   //
            << ONVP(values())        //
            << ONVP(current_index()) //
            ;
    }
};

template <typename T>
struct serialization::Serializable<wholth::BufferView<T>> : std::true_type
{
    const wholth::BufferView<T>& o;
    template <typename Serializer> inline void serialize(Serializer& serializer)
    {
        serializer          //
            << ONVP(buffer) //
            << ONVP(view)   //
            ;
    }
};

#endif // WHOLTH_SERIALIZATION_DEFINITIONS_H_
