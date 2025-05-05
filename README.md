# Club Tracker

**Тестовое задание.** 

Консольная утилита на C++20, моделирующая рабочий день компьютерного клуба.
Читает конфигурацию и последовательность событий из текстового файла, валидирует входные данные, эмулирует действия посетителей и выводит статистику по выручке и занятости столов.

---

## Клонирование репозитория
```bash
git clone https://github.com/sectapunterx/club_tracker.git
cd club_tracker
```
> Важно: дополнительных модулей нет — Google Test (разрешено по ТЗ) подтягивается автоматически через FetchContent при первой конфигурации CMake.

---

## 1. Требования

* CMake ≥ 3.24
* Clang 14+ или GCC 12+ с поддержкой C++20
* Ninja — система сборки
* Google Test подтягивается автоматически через `FetchContent`

### Arch Linux

```bash
sudo pacman -S clang cmake ninja
```

### Windows (через Cygwin)

1. Скачайте установщик Cygwin `setup‑x86_64.exe`, отметьте пакеты `gcc-g++` (или `clang`),
   а также `cmake`, `ninja` (или `make`).
2. Убедитесь, что компилятор виден в PATH
   (`c++ --version` или `clang++ --version`).

---

## 2. Сборка

### Linux 

```bash
cmake -S . -B build -G Ninja
cmake --build build
```


### Cygwin + GCC (используется g++ 12.4)

```bash
cmake -S . -B build -G Ninja
cmake --build build
```

---

## 3. Запуск программы

```bash
./build/task input.txt
```
Или
```bash
cd build
./task ../input.txt
```

---

## 4. Юнит‑тесты

```bash
ctest --test-dir build --output-on-failure
```
