
#include <iostream>
#include <fstream>
#include <string>
#include <stdexcept>
#include <windows.h>
using namespace std;

class Hero {
    string name;
    int level, gold;
public:
    Hero(string n = "", int l = 1, int g = 0)
        : name(n), level(l), gold(g) {
    }
    string getName() const { return name; }
    int getLevel() const { return level; }
    int getGold() const { return gold; }
    void levelUp() { ++level; }
    void addGold(int n) { gold += n; }
    void print() const {
        cout << name << ": уровень " << level << ", золото " << gold << '\n';
    }
};

class HeroStorage {
    string fileName;
public:
    HeroStorage(string f) : fileName(f) {}

    void save(const Hero& h) const {
        ofstream f(fileName);
        if (!f) throw runtime_error("Ошибка сохранения");
        f << h.getLevel() << ' ' << h.getGold() << ' ' << h.getName() << '\n';
    }

    bool load(Hero& h) const {
        ifstream f(fileName);
        if (!f) return false;
        int l, g;
        string n;
        if (!(f >> l >> g) || !getline(f >> ws, n) || n.empty())
            throw runtime_error("Ошибка чтения");
        h = Hero(n, l, g);
        return true;
    }
};

struct LevelResult { int stars, bestTime; };

void printLevel(const LevelResult& r, int i) {
    cout << "Уровень " << i + 1 << ": " << r.stars
        << " звёзд, " << r.bestTime << " с\n";
}

class LevelProgress {
    string fileName;
public:
    LevelProgress(string f) : fileName(f) {}

    void createIfMissing(int count) const {
        if (ifstream(fileName, ios::binary)) return;
        ofstream f(fileName, ios::binary);
        if (!f) throw runtime_error("Ошибка создания");
        LevelResult r{ 0, 0 };
        while (count--)
            f.write(reinterpret_cast<const char*>(&r), sizeof(r));
    }

    int getLevelCount() const {
        ifstream f(fileName, ios::binary | ios::ate);
        if (!f) throw runtime_error("Ошибка файла");
        return static_cast<int>(f.tellg() / sizeof(LevelResult));
    }

    LevelResult readLevel(int i) const {
        if (i < 0 || i >= getLevelCount())
            throw out_of_range("Нет такого уровня");
        ifstream f(fileName, ios::binary);
        if (!f) throw runtime_error("Ошибка файла");
        LevelResult r{};
        f.seekg(i * sizeof(r));
        if (!f.read(reinterpret_cast<char*>(&r), sizeof(r)))
            throw runtime_error("Ошибка чтения");
        return r;
    }

    void saveResult(int i, const LevelResult& r) const {
        LevelResult old = readLevel(i);
        fstream f(fileName, ios::binary | ios::in | ios::out);
        if (!f) throw runtime_error("Ошибка файла");
        cout << "Уровень " << i + 1 << ": ";
        if (r.stars > old.stars) {
            f.seekp(i * sizeof(r));
            if (!f.write(reinterpret_cast<const char*>(&r), sizeof(r)))
                throw runtime_error("Ошибка записи");
            cout << "новый рекорд!\n";
        }
        else cout << "рекорд не побит\n";
    }
};

void writeLog(const string& s) {
    ofstream f("log.txt", ios::app);
    if (!f) throw runtime_error("Ошибка журнала");
    f << s << '\n';
}

int main() {
    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);

    try {
        Hero h;
        HeroStorage s("hero.txt");

        if (s.load(h)) cout << "Загружено сохранение\n";
        else {
            h = Hero("Сер Галахад", 1, 0);
            cout << "Новая игра\n";
        }

        h.levelUp();
        h.addGold(100);
        h.print();
        s.save(h);

        writeLog(h.getName() + " достиг уровня " + to_string(h.getLevel()));

        ifstream log("log.txt");
        string line;
        while (getline(log, line)) cout << line << '\n';

        LevelProgress p("progress.dat");
        p.createIfMissing(5);

        p.saveResult(0, { 3, 45 });
        p.saveResult(1, { 1, 120 });
        p.saveResult(1, { 2, 95 });
        p.saveResult(0, { 2, 30 });

        int stars = 0;
        for (int i = 0; i < p.getLevelCount(); ++i) {
            auto r = p.readLevel(i);
            printLevel(r, i);
            stars += r.stars;
        }
        cout << "Звёзд: " << stars << " из 15\n";
    }
    catch (const exception& e) {
        cout << "Ошибка: " << e.what() << '\n';
    }
}

/*

1. Поток — это передача данных между программой
   и файлом или консолью. Операторы <<, >> и getline
   работают одинаково благодаря общим интерфейсам потоков.

2. Без ios::app старый журнал стирался бы.
   Если close() не вызвать, файл автоматически
   закроется при уничтожении файлового потока.

3. HeroStorage отделяет данные героя от работы
   с файлами. Так код проще поддерживать.
   load возвращает false, если сохранения ещё нет.
   save выбрасывает исключение, если запись невозможна.

4. Текстовый файл хранит читаемые символы,
   бинарный — данные в виде байтов.
   В progress.dat записи одинакового размера,
   поэтому можно перейти сразу к нужному уровню.
   В log.txt строки разной длины.
   Hero нельзя записывать целиком через write,
   потому что string использует динамическую память.
*/
