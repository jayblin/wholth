# Как билдить

> В системе должен быть установлен `cmake >=v3.23` .

Для разработки:
```sh
# подготовка окружения
make prep-dev-[clang|gnu]
# компиляуция
make build-dev
# установка
make install-dev
``` 

Для прода:
```sh
# подготовка окружения
make prep-release
# компиляуция
make build-release
# установка
make install-release
``` 

# Тесты

Запустить все тесты:
```sh
make test-dev TEST_CASE="*"
```

Запустить определённый тест:
```sh
make test-dev TEST_CASE="Test_wholth_sql_statements_exercise_log_update.*"
```

# Roadmap

Задачи, которые я когда-нибудь хотел бы сделать лежат в папке `tasks`.

# Что понял

## KISS
Изначальный подход к менеджменту сущностями через отдельные функции по типу
`wholth_em_*`,`wholth_pages_*` - это было долго и неудобно.
Мне теперь кажется, что удобнее и правильней подход менеджемента через
функционал `wholth/c/exec_stmt.h` и sql-файлики в папке `sql_statements` - это
просто, это легко тестировать и клиент этой либы пишет в разы меьше кода.
