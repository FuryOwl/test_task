#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <chrono>
#include <thread>
#include <mutex>
#include <Windows.h>
#include <stdio.h>
#include <conio.h>
#include <ctime>

bool isRunning = true;
std::mutex eventsMutex;
std::mutex birthdayMutex;

struct Date {
    int day;
    int month;
    int year;
};

struct DateTime {
    Date date;
    int hour;
    int minutes;
};

struct FullName {
    std::string surname;
    std::string first_name;
    std::string patronymic;
};

struct Event {
    DateTime created;
    Date expires;
    std::string description;
};

struct Birthday {
    Date date;
    FullName full_name;
    int age;
};

struct Note {
    std::string data;
    Date date;
    std::string type;
};
// Проверка, является ли год високосным
bool isLeapYear(const int& year) {
    return ((year % 4 == 0) && (year % 100 != 0)) || (year % 400 == 0);
}
// Проверка, что дата действительно существует
bool isValidDate(const Date& new_date) {
    int day = new_date.day;
    int month = new_date.month;
    int year = new_date.year;
    if (day < 1 || day > 31 || month < 1 || month > 12) {
        return false;
    }
    if (month == 2) {
        if (isLeapYear(year)) {
            return day <= 29;
        }
        else {
            return day <= 28;
        }
    }
    else if (month == 4 || month == 6 || month == 9 || month == 11) {
        return day <= 30;
    }
    else {
        return day <= 31;
    }
}
// Проверка введенной даты на корректность
void ensureNextSymbolAndSkip(std::stringstream& stream) {
    if (stream.peek() != '.') {
        std::stringstream ss;
        ss << "expected '.' , but has : " << char(stream.peek());
        throw std::runtime_error(ss.str());
    }
    stream.ignore(1);
}
// Функция, извлекающая день, месяц и год из строки (ДД.ММ.ГГГГ)
Date parseDate(const std::string& s) {
    std::stringstream stream(s);
    Date date;
    stream >> date.day;
    ensureNextSymbolAndSkip(stream);
    stream >> date.month;
    ensureNextSymbolAndSkip(stream);
    stream >> date.year;
    if (isValidDate(date) == true) {
        return date;
    }
    else {
        std::stringstream ss;
        ss << "data is invalid";
        throw std::runtime_error(ss.str());
    }
}
// Функция, сравнивающая даты структуры Notes
bool toCompareDate(const Note& left, const Note& right) {
    if (left.date.year < right.date.year) {
        return true;
    }
    else if (left.date.year == right.date.year) {
        if (left.date.month < right.date.month) {
            return true;
        }
        else if (left.date.month == right.date.month) {
            if (left.date.day < right.date.day) {
                return true;
            }
        }
    }
    return false;
}
// Функция, которая возвращает указатель на структуру tm, которая в свою очередь содержит информацию о локальном времени, полученном из текущего времени
auto getLocalTime() {
    auto currentTime = std::chrono::system_clock::now(); // Получаем текущее время
    std::time_t currentTimet = std::chrono::system_clock::to_time_t(currentTime);
    struct tm timeinfo;
    localtime_s(&timeinfo, &currentTimet);
    return &timeinfo;
}
// Создание записи
void makeNote(std::vector<Event>& events, std::vector<Birthday>& birthdays) {
    char q;
    std::cout << "Какой тип записи следует произвести?" << std::endl;
    std::cout << "1 - Событие" << std::endl;
    std::cout << "2 - День рождения" << std::endl;
    std::cout << "Любая другая клавиша - Выйти" << std::endl;
    q = _getch();
    system("cls");
    if (q == '1') {
        Event new_event;
        std::string new_description;
        std::string new_expires;

        std::cout << "Напишите дату наступления события (в числовом формате ДД.ММ.ГГГГ):" << std::endl;
        std::cin >> new_expires;
        new_event.expires = parseDate(new_expires);
        std::cout << "Напишите описание события:" << std::endl;
        std::cin.ignore(1, '\n');
        std::getline(std::cin, new_description);
        new_event.description = new_description;
     
        auto currentLocaltime = getLocalTime(); // получаем текущее время в локальном часовом поясе

        new_event.created.date.year = currentLocaltime->tm_year + 1900;
        new_event.created.date.month = currentLocaltime->tm_mon + 1;
        new_event.created.date.day = currentLocaltime->tm_mday;
        new_event.created.hour = currentLocaltime->tm_hour;
        new_event.created.minutes = currentLocaltime->tm_min;
        events.push_back(new_event);
    }
    else if (q == '2') {
        Birthday new_birthday;
        bool isDuplicate = false;
        std::string new_surname;
        std::string new_first_name;
        std::string new_patronymic;
        std::string birth_date;

        std::cout << "Напишите дату рождения (в числовом формате ДД.ММ.ГГГГ):" << std::endl;
        std::cin >> birth_date;
        new_birthday.date = parseDate(birth_date);
        // Если у человека день рождения 29 февраля, то для упрощения задачи будет использоваться 28 февраля
        if (new_birthday.date.day == 29 && new_birthday.date.month == 2) {
            new_birthday.date.day == 28;
        }

        auto currentLocaltime = getLocalTime(); // Получаем текущее время в локальном часовом поясе

        new_birthday.age = currentLocaltime->tm_year + 1900 - new_birthday.date.year; // вычисляем возраст в годах
        if (new_birthday.date.month > currentLocaltime->tm_mon + 1 ||
            (new_birthday.date.month == currentLocaltime->tm_mon + 1 && new_birthday.date.day > currentLocaltime->tm_mday)) {
            new_birthday.age--; // Если день рождения еще не наступил в этом году, вычитаем 1 год из возраста
        }

        std::cout << "Напишите ФИО человека. Введите фамилию:" << std::endl;
        std::cin >> new_surname;
        new_birthday.full_name.surname = new_surname;
        std::cout << "Введите имя: " << std::endl;
        std::cin >> new_first_name;
        new_birthday.full_name.first_name = new_first_name;
        std::cout << "Введите отчество: " << std::endl;
        std::cin >> new_patronymic;
        new_birthday.full_name.patronymic = new_patronymic;
        for (const auto& item : birthdays) {
            if (item.full_name.surname == new_birthday.full_name.surname && item.full_name.first_name == new_birthday.full_name.first_name
                && item.full_name.patronymic == new_birthday.full_name.surname && item.date.year == new_birthday.date.year && item.date.month == new_birthday.date.month
                && item.date.day == new_birthday.date.year) {
                isDuplicate = true;
                std::cout << "Такая запись уже существует." << std::endl;
                break;
            }
        }
        if (isDuplicate != true) {
            birthdays.push_back(new_birthday);
        }
        else {
            std::cout << "Это дубликат." << std::endl;
        }
    }
    std::cout << std::endl << "Нажмите любую клавишу для выхода..." << std::endl;
    _getch();
    system("cls");
}
//Вывод данных, отсортированных по возрастанию даты
void printSortedNotesByDate(const std::vector<Event>& events, const std::vector<Birthday>& birthdays) {
    std::vector<Note> notes;
    Note new_note;
    for (const auto& item : events) {
        new_note.type = "Событие";
        new_note.data = item.description;
        new_note.date = item.expires;
        notes.push_back(new_note);
    }
    for (const auto& item : birthdays) {
        new_note.type = "День рождения";
        new_note.data = item.full_name.surname + " " + item.full_name.first_name + " " + item.full_name.patronymic + " Возраст: " + std::to_string(item.age);
        new_note.date = item.date;
        new_note.date.year = item.date.year + item.age + 1;
        notes.push_back(new_note);
    }
    sort(notes.begin(), notes.end(), toCompareDate);
    for (const auto& item : notes) {
        std::cout << item.type << ": " << item.data << " Наступит: " << item.date.day << "." << item.date.month << "." << item.date.year << std::endl;
    }
    std::cout << std::endl << "Нажмите любую клавишу для выхода..." << std::endl;
    _getch();
    system("cls");
}
//Вывод данных по дате наступления
void printNotesByExpiresDate(const std::vector<Event>& events, const std::vector<Birthday>& birthdays) {
    Date new_date;
    std::string new_str_date;
    std::cout << "Напишите дату наступления события (в числовом формате ДД.ММ.ГГГГ):" << std::endl;
    std::cin >> new_str_date;
    new_date = parseDate(new_str_date);
    for (const auto& item : events) {
        if (new_date.year == item.created.date.year && new_date.month == item.created.date.month && new_date.day == item.created.date.day) {
            std::cout << "Cобытие: " << item.description << " Дата наступления события: " << item.expires.day << "." << item.expires.month << "." << item.expires.year << std::endl;
        }
    }
    for (const auto& item : birthdays) {
        if (new_date.month == item.date.month && new_date.day == item.date.day) {
            std::cout << "ФИО: " << item.full_name.surname << " " << item.full_name.first_name << " " << item.full_name.patronymic << " " << " Дата наступления дня рождения: " << item.date.day << "." << item.date.month << "." << new_date.year << std::endl;
        }
    }
    std::cout << std::endl << "Нажмите любую клавишу для выхода..." << std::endl;
    _getch();
    system("cls");

}
//Вывод дня рождения по ФИО
void printBirthDateByFullName(const std::vector<Birthday>& birthdays) {
    std::string new_surname;
    std::string new_first_name;
    std::string new_patronymic;

    std::cout << "Введите фамилию:" << std::endl;
    std::cin >> new_surname;
    std::cout << "Введите имя: " << std::endl;
    std::cin >> new_first_name;
    std::cout << "Введите отчество: " << std::endl;
    std::cin >> new_patronymic;
    for (const auto& item : birthdays) {
        if (item.full_name.surname == new_surname && item.full_name.first_name == new_first_name && item.full_name.patronymic == new_patronymic) {
            std::cout << std::endl << "День рождения: " << std::setfill('0') << std::setw(2) << item.date.day << "." << std::setfill('0') << std::setw(2)
                << item.date.month << "." << item.date.year << std::endl << "Возраст: " << item.age << std::endl;
        }
    }
    std::cout << std::endl << "Нажмите любую клавишу для выхода..." << std::endl;
    _getch();
    system("cls");
}
//Вывод событий по дате создания
void printEventByCreationDate(std::vector<Event>& events) {
    Date new_date;
    std::string new_created;
    std::cout << "Напишите дату создания события (в числовом формате ДД.ММ.ГГГГ):" << std::endl;
    std::cin >> new_created;
    std::cout << std::endl;
    new_date = parseDate(new_created);
    for (const auto& item : events) {
        if (new_date.year == item.created.date.year && new_date.month == item.created.date.month && new_date.day == item.created.date.day) {
            std::cout << item.description << " Дата наступления события: " << item.expires.day << "." << item.expires.month << "." << item.expires.year
                << " Дата и время создания: " << item.created.date.day << "." << item.created.date.month << "." << item.created.date.year << " "
                << std::setfill('0') << std::setw(2) << item.created.hour << ":" << std::setfill('0') << std::setw(2) << item.created.minutes << std::endl;
        }
    }
    std::cout << std::endl << "Нажмите любую клавишу для выхода..." << std::endl;
    _getch();
    system("cls");
}
//Вывод событий, наступающих сегодня
void printEventByExpiresToday(const std::vector<Event>& events) {
    auto currentTime = std::chrono::system_clock::now(); // получаем текущее время
    std::time_t currentTimet = std::chrono::system_clock::to_time_t(currentTime);
    struct tm timeinfo;
    localtime_s(&timeinfo, &currentTimet);
    auto currentLocaltime = &timeinfo; // получаем текущее время в локальном часовом поясе
    std::cout << "Список событий, наступающих сегодня:" << std::endl;
    for (const auto& item : events) {
        if (item.expires.day == currentLocaltime->tm_mday && item.expires.month == currentLocaltime->tm_mon + 1 && item.expires.year == currentLocaltime->tm_year + 1900) {
            std::cout << item.description << std::endl;
        }
    }
    std::cout << std::endl << "Нажмите любую клавишу для выхода..." << std::endl;
    _getch();
    system("cls");
}
//Функция, которая сохраняет записи в файл output.txt
void saveNotesInFile(const std::vector<Event>& events, const std::vector<Birthday>& birthdays) {
    std::ofstream output("output.txt", std::ios::app);
    for (const auto& item : events) {
        output << "Event. Description: " << item.description << " Created: " << item.created.date.day << "." << item.created.date.month << "."
            << item.created.date.year << " " << std::setfill('0') << std::setw(2) << item.created.hour << ":" << std::setfill('0') << std::setw(2)
            << item.created.minutes << " Expires: " << item.expires.day << "." << item.expires.month << "." << item.expires.year << std::endl;
    }
    for (const auto& item : birthdays) {
        output << "Birthday. Full name: " << item.full_name.surname << " " << item.full_name.first_name << " " << item.full_name.patronymic
            << " Date: " << item.date.day << "." << item.date.month << "." << item.date.year << std::endl;
    }
}
//Функция, которая мониторит вектор событий Event и удаляет событие, если день наступления совпадает с текущей датой и время = 23:59:59
void eventMonitor(std::vector<Event>& events) {
    while (isRunning) {
        std::this_thread::sleep_for(std::chrono::seconds(1)); // ждем 1 секунду, чтобы не нагружать процессор
        auto currentTime = std::chrono::system_clock::now(); // получаем текущее время
        std::time_t currentTimet = std::chrono::system_clock::to_time_t(currentTime);
        struct tm timeinfo;
        localtime_s(&timeinfo, &currentTimet);
        auto currentLocaltime = &timeinfo; // получаем текущее время в локальном часовом поясе

        if (currentLocaltime->tm_hour == 23 && currentLocaltime->tm_min == 59 && currentLocaltime->tm_sec == 59) {
            for (auto item = events.begin(); item != events.end(); ) {
                eventsMutex.lock();
                if (item->expires.day == currentLocaltime->tm_mday && item->expires.month == currentLocaltime->tm_mon + 1 && item->expires.year == currentLocaltime->tm_year + 1900) {
                    item = events.erase(item);
                }
                else {
                    ++item;
                }
                eventsMutex.unlock();
            }
        }
    }
}
//Функция, которая мониторит вектор дней рождения и прибавляет к age единицу, если день наступил и время показывает 00:01
void birthdayMonitor(std::vector<Birthday>& birthdays) {
    while (isRunning) {
        std::this_thread::sleep_for(std::chrono::seconds(1)); // ждем 1 секунду, чтобы не нагружать процессор
        auto currentTime = std::chrono::system_clock::now(); // получаем текущее время
        std::time_t currentTimet = std::chrono::system_clock::to_time_t(currentTime);
        struct tm timeinfo;
        localtime_s(&timeinfo, &currentTimet);
        auto currentLocaltime = &timeinfo; // получаем текущее время в локальном часовом поясе

        if (currentLocaltime->tm_hour == 0 && currentLocaltime->tm_min == 1 && currentLocaltime->tm_sec == 0) {
            birthdayMutex.lock();
            for (auto& birthday : birthdays) {
                if (birthday.date.day == currentLocaltime->tm_mday && birthday.date.month == currentLocaltime->tm_mon + 1) {
                    ++birthday.age;                  
                }
            }
            birthdayMutex.unlock();
        }
    }
}

void Run() {
    std::vector<Event> events;
    std::vector<Birthday> birthdays;
    std::thread eventThread(eventMonitor, std::ref(events));
    std::thread birthdayThread(birthdayMonitor, std::ref(birthdays));
    int q;

    while (true) {
        std::cout << "1 - Добавить запись" << std::endl;
        std::cout << "2 - Вывести список записей, отсортированных по возрасанию даты" << std::endl;
        std::cout << "3 - Вывод событий по дате наступления" << std::endl;
        std::cout << "4 - Вывод дня рождения по ФИО" << std::endl;
        std::cout << "5 - Вывод событий по дате создания" << std::endl;
        std::cout << "6 - Вывод событий, которые наступают сегодня" << std::endl;
        std::cout << "7 - Сохранить записи в файл" << std::endl;
        std::cout << "0 - Выйти из программы" << std::endl;
        q = _getch();
        system("cls");
        if (q == '1') {
            eventsMutex.lock();
            birthdayMutex.lock();
            makeNote(events, birthdays);
            eventsMutex.unlock();
            birthdayMutex.unlock();
        }
        if (q == '2') printSortedNotesByDate(events, birthdays);
        if (q == '3') printNotesByExpiresDate(events, birthdays);
        if (q == '4') printBirthDateByFullName(birthdays);
        if (q == '5') printEventByCreationDate(events);
        if (q == '6') printEventByExpiresToday(events);
        if (q == '7') saveNotesInFile(events, birthdays);
        if (q == '0') break;
    }

    isRunning = false;
    eventThread.join();
    birthdayThread.join();
}

int main() {
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);

    Run();

    return 0;
}