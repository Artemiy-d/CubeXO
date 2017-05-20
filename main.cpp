#include <iostream>
#include <vector>
#include <iostream>

#include <assert.h>

using FieldType = char;

constexpr FieldType fieldX = 'x';
constexpr FieldType fieldO = 'o';
constexpr FieldType fieldEmpty = '-';

struct Vector
{
    int x, y;

    Vector operator - () const
    {
        return {-x, -y};
    }

    bool operator == (const Vector& v) const
    {
        return x == v.x || y == v.y;
    }

    bool operator != (const Vector& v) const
    {
        return x != v.x || y != v.y;
    }

    Vector operator + (const Vector& v) const
    {
        return {x + v.x, y + v.y};
    }

    Vector operator * (int val) const
    {
        return {x * val, y * val};
    }

    Vector& operator += (const Vector& v)
    {
        x += v.x;
        y += v.y;

        return *this;
    }

    bool isEmpty() const
    {
        return x == 0 && y == 0;
    }
};

template <typename Type>
class Cube
{
public:

    template <typename Cube>
    class CubeIterator
    {
    public:

        CubeIterator(Cube* cube) :
            _cube(cube),
            _number(-1),
            _point({0, 0}),
            _step({0, 0})
        {

        }

        CubeIterator(Cube* cube, int number, int x, int y, const Vector& step) :
            _cube(cube),
            _number(number),
            _point({x * 2 + 1 - cube->_size, y * 2 + 1 - cube->_size}),
            _step(step * 2)
        {

        }

        template <typename OtherCube>
        CubeIterator(const CubeIterator<OtherCube>& other) :
            _cube(other._cube),
            _number(other._number),
            _point(other._point),
            _step(other._step)
        {

        }

        CubeIterator& operator = (std::nullptr_t)
        {
            _number = -1;
            return *this;
        }

        typename std::conditional<std::is_const<Cube>::value, Type, Type&>::type operator * () const
        {
            return _cube->_cubeMap[getIndex()];
        }

        CubeIterator& operator ++ ()
        {
            return (*this) += _step;
        }

        bool operator == (const CubeIterator& other) const
        {
            return _number == other._number &&
                   _point == other._point;
        }

        bool operator != (const CubeIterator& other) const
        {
            return _number != other._number ||
                   _point != other._point;
        }

        bool operator == (std::nullptr_t) const
        {
            return _number == -1;
        }

        int getIndex() const
        {
            return (_point.y * _cube->_size + _cube->_size2 + _point.x - 1) / 2 + _number * _cube->_size2;
        }

    private:

        CubeIterator& operator += (const Vector& step)
        {
            _point += step;
            const Vector d({ _cube->getSideOffset(_point.x), _cube->getSideOffset(_point.y) });

            assert(std::abs(d.x) <= 1);
            assert(std::abs(d.y) <= 1);

            if (d.x != 0 && d.y != 0)
                return (*this) = nullptr;

            if (d.isEmpty())
                return *this;

            auto number = d.x + d.y * 2;
            if (number < 0)
                number = 3 - number;

            _number = (_number + number) % 6;

            const auto a = d.x + std::abs(d.y);
            const auto b = std::abs(d.x) - d.y;

            auto transformVector = [a, b](Vector& point)
            {
                const auto newX = point.y * a;
                point.y = point.x * b;
                point.x = newX;
            };

            _point += d * (-_cube->_size * 2);
            transformVector(_point);
            transformVector(_step);

            return *this;
        }

    private:

        Cube* _cube;
        int _number;
        Vector _point;
        Vector _step;

        friend class CubeIterator<Cube const>;
    };

    Cube(int size, const Type& v)
    {
        reset(size, v);
    }

    int getSize() const
    {
        return _size;
    }

    void reset(int size, const Type& v)
    {
        assert(size > 0);

        _size = size;
        _size2 = size * size;
        _cubeMap.resize(_size2 * 6, v);
    }

    CubeIterator<Cube const> getIterator(int number, int x, int y, const Vector& step) const
    {
        return CubeIterator<Cube const>(this, number, x, y, step);
    }

    CubeIterator<Cube> getIterator(int number, int x, int y, const Vector& step)
    {
        return CubeIterator<Cube>(this, number, x, y, step);
    }

    CubeIterator<Cube const> getNullIterator() const
    {
        return CubeIterator<Cube const>(this);
    }

    CubeIterator<Cube> getNullIterator()
    {
        return CubeIterator<Cube>(this);
    }

private:

    int getSideOffset(int point) const
    {
        return  point >= _size ? point - _size :
                point <= -_size ? point + _size :
                              0;
    }

private:

    int _size;
    int _size2;
    std::vector<Type> _cubeMap;
};

class Game
{
public:
    using Cube = Cube<FieldType>;

    Game() :
        _cube(4, fieldEmpty),
        _winPoint(&_cube)
    {

    }

    FieldType getWinner() const
    {
        return _winPoint == nullptr ? fieldEmpty : *_winPoint;
    }

    void start(int cubeSize, int winLength)
    {
        assert(winLength > 0);
        assert(winLength < cubeSize * 4);

        _winPoint = nullptr;
        _winLength = winLength;
        _cube.reset(cubeSize, fieldEmpty);
    }

    int step(int number, int x, int y, FieldType v)
    {
        assert(number >= 0 && number < 6);
        assert(x >= 0 && x < _cube.getSize());
        assert(y >= 0 && y < _cube.getSize());
        assert(getWinner() == fieldEmpty);

        auto& value = *_cube.getIterator(number, x, y, {0, 1});

        if (value != fieldEmpty)
            return 0;

        value = v;

        static const auto steps =
        {
            Vector({0, 1}),
            Vector({1, 0}),
            Vector({1, 1}),
            Vector({1, -1})
        };

        int result = 0;
        Vector bestStep({0, 0});

        for (const auto& step : steps)
        {
            const auto len = count(_cube.getIterator(number, x, y, step), v) +
                              count(_cube.getIterator(number, x, y, -step), v);

            if (len > result)
            {
                bestStep = step;
                result = len;
            }
        }

        if (++result >= _winLength)
            _winPoint = _cube.getIterator(number, x, y, bestStep);

        return result;
    }

    std::string toString(int number) const
    {
        const auto size = _cube.getSize();

        const auto lenX = size * 4 + 1;
        const auto lenY = size * 3;
        std::string result(lenY * lenX, ' ');

        auto index = [&result, lenX, lenY](int x, int y) -> char&
        {
            return result[x + (lenY - y - 1) * lenX];
        };

        for (int i = 0; i < lenY; ++i)
        {
            index(lenX - 1, i) = '\n';
        }

        auto fill = [&](const Vector& p, const Vector& a, int count)
        {
            const Vector b({a.y, -a.x});

            for (int i = 0; i < size; ++i)
            {
                auto c = a * i + p;
                auto it = _cube.getIterator(number, c.x, c.y, b);

                for (int j = 0; j < count; ++j)
                {
                    ++it;
                    c += b;

                    index((size + c.x + lenX - 1) % (lenX - 1), (size + c.y + lenY) % lenY) = *it;
                }
            }
        };

        fill({0, 0}, {0, 1}, lenX - 1);
        fill({0, 0}, {1, 0}, size);
        fill({size - 1, size - 1}, {-1, 0}, size);

        return result;
    }

    int getCubeSize() const
    {
        return _cube.getSize();
    }

private:

    static int count(const Cube::CubeIterator<Cube const>& start, FieldType v)
    {
        assert(*start == v);

        int result = -1;
        auto it = start;

        do
            ++result;
        while (++it != nullptr && it != start && *it == v);

        return result;
    }

private:
    Cube _cube;
    int _winLength = 4;
    Cube::CubeIterator<Cube const>  _winPoint;
};

std::string numberToString(int number)
{
    assert(number >= 0 && number < 6);
    std::string result(15, ' ');
    result[4] = result[9] = result[14] = '\n';
    static const int indices[] = { 6, 7, 1, 8, 5, 11};

    for (int i = 0; i < 6; ++i)
    {
        result[indices[i]] = '1' + (number + i) % 6;
    }

    return result;
}

int main(int, char *[])
{  
    std::cout << "\nGame started. Input 'h' for help\n";

    Game game;

    int n = 0;

    auto printCube = [&]()
    {
        std::cout << numberToString(n) << game.toString(n);
    };

    printCube();

    while (1)
    {
        auto c = getchar();

        switch (c)
        {
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
            if (n != c - '1')
            {
                n = c - '1';

                std::cout << "Switched to '" << static_cast<char>(c) << "'\n";
                printCube();
            }
            else
            {
                std::cout << "Actual center quard is '" << static_cast<char>(c) << "'\n";
            }
            break;

        case fieldX:
        case fieldO:
            {
                std::cout << "Input step (x, y): '" << static_cast<char>(c) << "'\n";

                int x, y;
                std::cin >> x >> y;
                if (x > 0 && x <= game.getCubeSize() &&
                    y > 0 && y <= game.getCubeSize())
                {
                    const int length = game.step(n, x - 1, y - 1, c);
                    if (length > 0)
                    {
                        printCube();
                        std::cout << "length: " << length << "\n";
                    }
                    else
                    {
                        std::cout << "Cell isn't empty!\n";
                    }
                }
                else
                {
                    std::cout << "invalid arguments\n";
                }

                break;
            }
        case 'n':
            {
                std::cout << "Input size\n";

                int size;
                std::cin >> size;
                if (size > 0)
                {
                    game.start(size, fieldEmpty);
                    n = 0;
                    printCube();
                }
                else
                {
                    std::cout << "invalid arguments\n";
                }

                break;
            }
        case 'h':
            std::cout << "n    - new game\n"
                         "1-6  - set center quad\n"
                         "x    - next step 'x'\n"
                         "o    - next step 'o'\n"
                         "e    - exit";
            break;
        case 'e':
            return 0;
        }
    }

    return 0;
}
