```sh
cmake -B build -S . \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
    -DCMAKE_INSTALL_PREFIX=/Users/jiorn/Projects/wholth/build \
    -DVULKAN_SDK_INCLUDE_DIR=/Users/jiorn/VulkanSDK/1.3.250.1/MoltenVK/include \
    -DVULKAN_SDK_LIBRARY_PATH=/Users/jiorn/VulkanSDK/1.3.250.1/macOS/Frameworks/vulkan.framework/vulkan \
    -DDEFAULT_FONT_PATH=/Users/jiorn/Projects/Open_Sans/static/OpenSans-Medium.ttf
```

- check that in test locales do exist when creating `*_localisation` records;
- test rollbacks;
- fix Statement::exec(), when it's not iterating over results as expected;
- посмотреть на ошибки СИКВЛ в тестах;
- в прод-коде добавить `PRAGMA automatic_index=off;`
- переделать байндинг параметров в `list_foods`;
- Заполнять только первые 3-5 нутриента??
- Сделать фунционал отчетов:
    - Отчет пищи, у которой значения микроэлементов не соответствуют суммарному
      по его ингридиентам;
    - Пища, у которой не указан какой-то из 4-х основных микроэлементов;
- Сортировать список рецептов сначала по кол-ву ингридиентов (MAX(?,1)) затем по
  рандому;
- Пройтись по todo в коде;
- Переделать std::string buffer на какой-нибудь `buffer_t`;
- написать тесты для `current_time_and_date`;
- `slqw::utils::is_numeric` работает не правильно, т.к. не берет в учет знак `-`;
- попоробовать имплементировать sql'ный `RETURN`;
- спрятать GlfwWindow;
- `get_allocators` перенести в `include/allocator.hpp`;
-  уйти от использования `vk::check()` и сделать кастомные эксепшены;
-  уйти от `throw runtime_error()` и сделать кастомные эксепшены;
- [ ] Применить профилирование - беспокоюсь по поводу испольуемого объема стэка; 
- [ ] Узнать устанавливается ли `NDEBUG` при релизе/дебаге;
- [ ] move wholth/list.hpp to another file cause i dont like the naming;
- [ ] `cooking_action` appears to not be needed?
- [ ] обдумать структура папок, чтобы было пологичнее;
- [ ] опечатка в `sqlw error:-96some paramaters where not bound`;
- [ ] переделать апиху так, чтобы дефиниции не были hpp;
- [ ]  `update_important_nutrients()` должен возвращать список нутриентов и
  ингридиентов, для которых не удалось рассчитать верное кол-во нутриента; 
- [ ] добавить тесты на `created_at` в `insert_food`;
- [ ] Изменить реурн типы `wholth_ErrorCode` на `wholth_Error`;
- [ ] move stuff from `controller`  to `entity_manager`;
- [ ] подумать можно ли убрать копию строки из ::prepare();
- [ ] убрать в wholth::pages исопльзование ф-ции wholth::entity::count();
- [ ] удалить `cooking_action`;
- [ ] сделать тесты на размеры контейнеров больше 20;
- [ ] подумать на удаление еды:
    - нельзя просто удалить из-за `consumption_log` - может разрешать удалять
      записи страше года?
- [ ] зависимости от `default_locale_id` нужно удалить;
- [ ] `FoodQuery.ingredients` работает неправильно;
