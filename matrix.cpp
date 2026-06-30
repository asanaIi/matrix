#include <iostream>
#include <vector>
#include <string>
#include <random>
#include <chrono>
#include <thread>
#include <windows.h>

// Диапазон символов Катаканы в Юникоде
const int KATAKANA_START = 0x30A1;
const int KATAKANA_END = 0x30F6;

// Функция для генерации случайного японского символа (UTF-8)
std::string getRandomKatakana(std::mt19937& gen) {
    std::uniform_int_distribution<> dis(KATAKANA_START, KATAKANA_END);
    wchar_t wch = static_cast<wchar_t>(dis(gen));
    
    // Переводим UTF-16 (wchar_t в Windows) в UTF-8 строчку
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wch, 1, NULL, 0, NULL, NULL);
    std::string strTo(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, &wch, 1, &strTo[0], size_needed, NULL, NULL);
    return strTo;
}

int main() {
    // Шаг 1. Включаем UTF-8 кодировку в консоли Windows
    SetConsoleOutputCP(CP_UTF8);

    // Включаем поддержку ANSI-последовательностей (для цветов и управления курсором)
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode = 0;
    GetConsoleMode(hOut, &dwMode);
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hOut, dwMode);

    // Прячем курсор
    std::cout << "\033[?25l" << std::flush;

    // Настройка генератора случайных чисел
    std::random_device rd;
    std::mt19937 gen(rd());

    // Получаем размеры окна консоли
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(hOut, &csbi);
    int cols = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    int rows = csbi.srWindow.Bottom - csbi.srWindow.Top; // Оставляем 1 строку в запасе

    // Градации цветов (ANSI 256-color)
    std::string colors[] = {
        "\033[38;5;231m", // Белый (Голова)
        "\033[38;5;46m",  // Ярко-зеленый
        "\033[38;5;40m",  // Зеленый
        "\033[38;5;34m",  // Темно-зеленый
        "\033[38;5;28m",  // Еле заметный
        "\033[38;5;22m"   // Болотный
    };
    int colors_len = 6;
    std::string reset = "\033[0m";

    // Инициализируем капли высоко над экраном для эффекта редкого дождя
    std::vector<int> drops(cols);
    std::uniform_int_distribution<> drop_dis(-rows * 2, 0);
    for (int i = 0; i < cols; ++i) {
        drops[i] = drop_dis(gen);
    }

    // Главный цикл анимации
    while (true) {
        // Создаем чистый кадр заполненный пробелами
        std::vector<std::vector<std::string>> grid(rows, std::vector<std::string>(cols, " "));

        for (int c = 0; c < cols; ++c) {
            int y = drops[c];

            // Прорисовываем каплю и её хвост назад
            for (int shadow = 0; shadow < colors_len; ++shadow) {
                int pos_y = y - shadow;
                if (pos_y >= 0 && pos_y < rows) {
                    grid[pos_y][c] = colors[shadow] + getRandomKatakana(gen) + reset;
                }
            }

            // Двигаем каплю вниз
            drops[c]++;

            // Если капля и хвост ушли за экран — перезапускаем её сверху случайным образом
            if (y - colors_len >= rows) {
                std::uniform_int_distribution<> restart_dis(-rows, 0);
                std::uniform_real_distribution<> chance_dis(0.0, 1.0);
                if (chance_dis(gen) > 0.92) {
                    drops[c] = restart_dis(gen);
                }
            }
        }

        // Собираем весь кадр в одну большую строку для мгновенного вывода без лагов
        std::string output = "\033[H"; // Перенос курсора в начало экрана (0,0)
        for (int r = 0; r < rows; ++r) {
            for (int c = 0; c < cols; ++c) {
                output += grid[r][c];
            }
            if (r < rows - 1) output += "\n";
        }

        // Выводим кадр в терминал
        std::cout << output << std::flush;

        // Задержка кадра (скорость падения дождя)
        std::this_thread::sleep_for(std::chrono::milliseconds(45));
    }

    return 0;
}