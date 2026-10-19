#include <iostream>
#include <fstream>
#include <conio.h>
#include <windows.h>
#include <ctime>
#include <cstdlib>
#include <clocale>
using namespace std;

// размеры игрового поля
#define WIDTH 30
#define HEIGHT 15

// структура сегмента змеи (элемент односвязного списка)
struct Segment
{
    int x;           // координата x сегмента
    int y;           // координата y сегмента
    Segment* next;   // указатель на следующий сегмент
};

// структура записи рекорда
struct Record
{
    char name[30];   // имя игрока
    int score;       // набранные очки
};

// глобальные переменные игры
Segment* head = nullptr;   // голова змеи (начало списка)
int direction = 4;         // текущее направление (1-вверх, 2-вниз, 3-влево, 4-вправо)
int score = 0;             // текущий счёт
int length = 0;            // текущая длина змеи
int foodX = 0;             // координата x еды
int foodY = 0;             // координата y еды

// освобождение памяти, занятой змеёй
void freeSnake()
{
    while (head != nullptr)
    {
        Segment* temp = head;
        head = head->next;
        delete temp;
    }
    length = 0;
}

// добавление нового части в конец змеи
void addToEnd(int x, int y)
{
    Segment* s = new Segment;
    s->x = x;
    s->y = y;
    s->next = nullptr;
    if (head == nullptr)
    {
        head = s;
    }
    else
    {
        Segment* cur = head;
        while (cur->next != nullptr)
        {
            cur = cur->next;
        }
        cur->next = s;
    }
    length++;
}

// проверка, находится ли змея в указанных координатах
int isSnake(int x, int y)
{
    Segment* cur = head;
    while (cur != nullptr)
    {
        if (cur->x == x && cur->y == y)
        {
            return 1;
        }
        cur = cur->next;
    }
    return 0;
}

// размещение еды в случайной свободной клетке
void placeFood()
{
    do
    {
        foodX = rand() % WIDTH;
        foodY = rand() % HEIGHT;
    } while (isSnake(foodX, foodY) == 1);
}

// инициализация новой игры
void startNewGame()
{
    freeSnake();
    score = 0;
    direction = 4;
    // создание начальной змеи длиной 3 частей тела
    addToEnd(10, 7);
    addToEnd(9, 7);
    addToEnd(8, 7);
    placeFood();
}

// отрисовка игрового поля в консоли
void draw()
{
    system("cls");
    // верхняя граница
    for (int i = 0; i < WIDTH + 2; i++)
    {
        cout << '#';
    }
    cout << endl;
    // игровое поле
    for (int y = 0; y < HEIGHT; y++)
    {
        cout << '#';
        for (int x = 0; x < WIDTH; x++)
        {
            if (head->x == x && head->y == y)
            {
                cout << 'O';  // голова змеи
            }
            else if (isSnake(x, y) == 1)
            {
                cout << 'o';  // тело змеи
            }
            else if (x == foodX && y == foodY)
            {
                cout << '*';  // еда
            }
            else
            {
                cout << ' ';  // пустая клетка
            }
        }
        cout << '#' << endl;
    }
    // нижняя граница
    for (int i = 0; i < WIDTH + 2; i++)
    {
        cout << '#';
    }
    cout << endl;
    cout << "счёт: " << score << "  длина: " << length << endl;
    cout << "w a s d - движение, e - сохранить" << endl;
}

// перемещение змеи на один шаг
int moveSnake()
{
    int newX = head->x;
    int newY = head->y;

    // вычисление новой позиции головы в зависимости от направления
    if (direction == 1)
    {
        newY--;
    }
    else if (direction == 2)
    {
        newY++;
    }
    else if (direction == 3)
    {
        newX--;
    }
    else if (direction == 4)
    {
        newX++;
    }

    // проверка столкновения со стеной
    if (newX < 0 || newX >= WIDTH || newY < 0 || newY >= HEIGHT)
    {
        return 0;
    }
    // проверка столкновения с собственным телом
    if (isSnake(newX, newY) == 1)
    {
        return 0;
    }

    // добавление новой головы
    Segment* s = new Segment;
    s->x = newX;
    s->y = newY;
    s->next = head;
    head = s;
    length++;

    // проверка поедания еды
    if (newX == foodX && newY == foodY)
    {
        score += 10;
        placeFood();
    }
    else
    {
        // удаление хвоста
        Segment* cur = head;
        while (cur->next->next != nullptr)
        {
            cur = cur->next;
        }
        delete cur->next;
        cur->next = nullptr;
        length--;
    }
    return 1;
}

// сохранение текущего состояния игры в файл
void saveGame()
{
    ofstream out("save.bin", ios::binary);
    if (!out)
    {
        cout << "не удалось создать файл сохранения" << endl;
        return;
    }
    // сохранение основных параметров игры
    out.write(reinterpret_cast<const char*>(&score), sizeof(score));
    out.write(reinterpret_cast<const char*>(&direction), sizeof(direction));
    out.write(reinterpret_cast<const char*>(&foodX), sizeof(foodX));
    out.write(reinterpret_cast<const char*>(&foodY), sizeof(foodY));
    out.write(reinterpret_cast<const char*>(&length), sizeof(length));

    // сохранение всех частей змеи
    Segment* cur = head;
    while (cur != nullptr)
    {
        out.write(reinterpret_cast<const char*>(&cur->x), sizeof(cur->x));
        out.write(reinterpret_cast<const char*>(&cur->y), sizeof(cur->y));
        cur = cur->next;
    }
    out.close();
    cout << "игра сохранена" << endl;
}

// загрузка состояния игры из бинарного файла
int loadGame()
{
    ifstream in("save.bin", ios::binary);
    if (!in)
    {
        return 0;
    }

    int savedLength;

    freeSnake();
    // чтение параметров игры
    in.read(reinterpret_cast<char*>(&score), sizeof(score));
    in.read(reinterpret_cast<char*>(&direction), sizeof(direction));
    in.read(reinterpret_cast<char*>(&foodX), sizeof(foodX));
    in.read(reinterpret_cast<char*>(&foodY), sizeof(foodY));
    in.read(reinterpret_cast<char*>(&savedLength), sizeof(savedLength));

    // восстановление змеи 
    for (int i = 0; i < savedLength; i++)
    {
        int x;
        int y;
        in.read(reinterpret_cast<char*>(&x), sizeof(x));
        in.read(reinterpret_cast<char*>(&y), sizeof(y));
        addToEnd(x, y);
    }
    in.close();
    return 1;
}

// добавление нового рекорда в файл
void addRecord(int points)
{
    Record rec;
    cout << "введите имя: ";
    cin.getline(rec.name, 30);
    rec.score = points;

    ofstream out("records.bin", ios::binary | ios::app);
    if (!out)
    {
        cout << "не удалось открыть файл рекордов" << endl;
        return;
    }
    out.write(reinterpret_cast<const char*>(&rec), sizeof(Record));
    out.close();
}

// отображение рекордов
void showRecords()
{
    ifstream in("records.bin", ios::binary);
    if (!in)
    {
        cout << "рекордов пока нет" << endl;
        return;
    }

    // количество записей в файле
    in.seekg(0, ios::end);
    int size = static_cast<int>(in.tellg());
    int count = size / sizeof(Record);
    in.seekg(0, ios::beg);

    // чтение всех рекордов в массив
    Record* records = new Record[count];
    in.read(reinterpret_cast<char*>(records), count * sizeof(Record));
    in.close();

    cout << "таблица рекордов" << endl;
    for (int i = 0; i < count; i++)
    {
        cout << i + 1 << ". " << records[i].name << " - " << records[i].score << endl;
    }

    delete[] records;
}

// основной игровой цикл
void playGame()
{
    int running = 1;

    while (running == 1)
    {
        draw();

        // обработка нажатий клавиш
        if (_kbhit())
        {
            char key = _getch();
            if ((key == 'w' || key == 'W') && direction != 2)
            {
                direction = 1;
            }
            else if ((key == 's' || key == 'S') && direction != 1)
            {
                direction = 2;
            }
            else if ((key == 'a' || key == 'A') && direction != 4)
            {
                direction = 3;
            }
            else if ((key == 'd' || key == 'D') && direction != 3)
            {
                direction = 4;
            }
            else if (key == 'e' || key == 'E')
            {
                saveGame();
                Sleep(1000);
            }
        }

        // выполнение хода змеи
        if (moveSnake() == 0)
        {
            running = 0;
        }

        Sleep(150);  // задержка для управления скоростью игры
    }

    cout << "игра окончена! ваш счёт: " << score << endl;
    addRecord(score);
    cout << "нажмите на любую клавишу, чтобы продолжить" << endl;
    _getch();
}

int main()
{
    setlocale(LC_ALL, "Russian");
    srand(time(NULL));

    int choice = 0;
    while (choice != 4)
    {
        system("cls");
        cout << "1. новая игра" << endl;
        cout << "2. загрузить игру" << endl;
        cout << "3. таблица рекордов" << endl;
        cout << "4. выход" << endl;
        cout << "выбор: ";
        cin >> choice;
        cin.ignore();

        if (choice == 1)
        {
            startNewGame();
            playGame();
        }
        else if (choice == 2)
        {
            if (loadGame() == 1)
            {
                playGame();
            }
            else
            {
                cout << "сохранение не найдено или повреждено" << endl;
                cout << "нажмите на любую клавишу, чтобы продолжить" << endl;
                _getch();
            }
        }
        else if (choice == 3)
        {
            showRecords();
            cout << "нажмите любую клавишу, чтобы продолжить" << endl;
            _getch();
        }
    }

    freeSnake();
    return 0;
}