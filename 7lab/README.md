# TextCompareLab (Лабораторна робота № 7, Варіант 14 — Студент Б)

Статична бібліотека `TextCompareLib` для порівняння текстових файлів та рядків мовою C++20 із повною підтримкою обробки винятків, гарантій безпеки та модульних тестів Google Test.

## 1. Структура проєкту
* `CMakeLists.txt` — конфігурація збирання статичної бібліотеки `TextCompareLib`, демонстраційних програм `TextCompareDemo` та `AntipatternDemo`, а також тестів `TextCompareTests`.
* `vcpkg.json` та `CMakePresets.json` — налаштування підключення `gtest` через менеджер пакетів `vcpkg`.
* `include/` — заголовкові файли (`TextCompare.h`, `TextCompareExceptions.h`, `ScopeLogger.h`).
* `src/` — реалізація методів бібліотеки (`TextCompare.cpp`).
* `demo/` — демонстрація роботи бібліотеки, розкручування стека (`ScopeLogger`) та сильної гарантії безпеки.
* `antipattern/` — демонстрація антипатернів (`throw int`, `throw std::string`, `throw ex;` object slicing).
* `tests/` — модульні тести на базі Google Test (`TextCompareTests.cpp`).

## 2. Інваріанти класу `TextCompare`
Для кожного об'єкта `TextCompare` завжди виконуються такі умови:
1. Кількість збережених документів у колекції не перевищує `MAX_TEXTS` (`0 <= texts_.size() <= 100`).
2. Розмір кожного документа не перевищує `MAX_TEXT_LENGTH` (`10 000 000` байтів).
3. Жоден збережений текстовий документ не містить нульового символу `'\0'`.

## 3. Ієрархія винятків та сценарії помилок
* `TextCompareException` (успадкований від `std::runtime_error`) — базовий клас власних винятків бібліотеки:
  * `FileAccessException` — генерується, якщо файл не існує, шлях вказує на каталог або виникла помилка читання; зберігає `filePath()` та `reason()`.
  * `TextSizeLimitException` — генерується при перевищенні `MAX_TEXTS`, `MAX_TEXT_LENGTH` або ліміту клітинок LCS-матриці `MAX_LCS_MATRIX_CELLS = 25 000 000`; зберігає `resourceName()`, `maxAllowed()` та `actualSize()`.
  * `InvalidTextParameterException` — генерується при передачі неприпустимого вмісту тексту (наприклад, нульового байта `'\0'`).
* Стандартні винятки C++:
  * `std::invalid_argument` — передача порожнього шляху до файлу (`""`).
  * `std::out_of_range` — звернення до тексту за неіснуючим індексом у `textAt()`, `similarity()`, `commonWords()`, `differenceCount()`.

## 4. Гарантії безпеки винятків
* **No-throw guarantee (`noexcept`):** конструктор за замовчуванням `TextCompare()`, методи `size()`, `empty()`, `texts()`, а також деструктор `~ScopeLogger() noexcept`.
* **Strong guarantee:** конструктор `TextCompare(const std::vector<std::string>&)`, `textAt()`, `addText()`, `addFile()`, `commonWords()`, `similarity()`, `differenceCount()`, `compareFiles()` та модифікуючий метод `replaceTexts()`. У `replaceTexts()` реалізовано підхід **prepare-then-commit**: новий стан формується і валідується у тимчасовому об'єкті `temp`, після чого фіксується викликом `texts_.swap(temp.texts_)`, який не генерує винятків.
* **Basic guarantee:** метод `replaceTextsUnsafe()` (навчальний приклад порушення сильної гарантії, де `texts_.clear()` викликається до перевірки нових елементів).

## 5. Дослідження розкручування стека (RAII та `ScopeLogger`)
**Очікувана послідовність повідомлень до запуску `TextCompareDemo`:**
```text
[ENTER] level1_pipeline
[ENTER] level2_processBatch
[ENTER] level3_updateStorage
Preparing invalid batch with embedded NUL byte...
[EXIT ] level3_updateStorage
[LOG] level2_processBatch caught: Invalid parameter [text]: Binary NUL ('\0') character is not allowed in plain text
[LOG] Rethrowing via 'throw;' to preserve dynamic type...
[EXIT ] level2_processBatch
[EXIT ] level1_pipeline
[MAIN] InvalidTextParameterException: Invalid parameter [text]: Binary NUL ('\0') character is not allowed in plain text | param=text
```
**Порівняння з фактичним результатом:** фактичний вивід у консолі повністю збігається з очікуваним. Під час розкручування стека деструктори локальних об'єктів `ScopeLogger` викликаються автоматично у порядку LIFO (`level3_updateStorage` -> `level2_processBatch` -> `level1_pipeline`).

## 6. Висновок щодо антипатернів (`AntipatternDemo`)
Використання `throw -1;` або `throw std::string(...)` порушує поліморфну обробку помилок через єдиний базовий клас `std::exception` (такі винятки ловить лише `catch (...)` або точний збіг типу). Повторне генерування через `throw ex;` замість `throw;` виконує копіювання об'єкта у базовий тип (`object slicing`), через що втрачається похідний тип винятку (`std::invalid_argument` перетворюється на базовий `std::exception`).

## 7. Збирання та запуск
```bash
vcpkg x-update-baseline --add-initial-baseline
cmake --preset x64-debug
cmake --build --preset x64-debug
ctest --test-dir out/build/x64-debug --output-on-failure
```
