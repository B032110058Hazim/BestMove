#include <vector>
#include <string>
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>

#define GAMELENGTH 30.f

int score = 0;

namespace Utilities {
    sf::Vector2f linearInterpolate(sf::Vector2f a, sf::Vector2f b, float t) {
        return sf::Vector2f(a + sf::Vector2f(t * (b - a).x, t * (b - a).y));
    }

    sf::Vector2f Normalize(sf::Vector2f v) {
        float l = std::sqrt(std::pow(v.x, 2) + std::pow(v.y, 2));

        return l == 0 ? sf::Vector2f() : sf::Vector2f(
            v.x /
            l,
            v.y /
            l);
    }

    float distanceBetween(sf::Vector2f a, sf::Vector2f b) {
        return sqrt(pow(b.x - a.x, 2) + pow(b.y - a.y, 2) * 1.f);
    }

    float radiansToDegrees(float f) {
        return f * (180.0f / 3.141592653589793238463f);
    }

    float DegreesToRadians(float f) {
        return f * (3.141592653589793238463f / 180.0f);
    }

}

//Engine is a singleton
class Engine {
    static Engine* engine;

    sf::VideoMode video;
    sf::Clock clock;

    Engine();

public:
    sf::RenderWindow window;
    sf::View view;
    sf::Event event;

    float deltaTime;

    Engine(Engine& other) = delete;
    void operator=(const Engine&) = delete;
    static Engine& instance();
    void render(sf::Sprite& sprite);
    void render(sf::Text& text);
    void render(sf::Vertex* v, int l);
    sf::Event& next();
    sf::Vector2f getMousePosition();
};

Engine::Engine():
    video(1920, 1080),
    window(this->video, "Best Move"),
    view(sf::FloatRect(0, 0, 1920, 1080)),
    deltaTime(0.f)
{
    srand(time(NULL));
    window.setView(view);
    window.setMouseCursorVisible(false);
    window.setKeyRepeatEnabled(false);
}

Engine& Engine::instance() {
    static Engine* engine = new Engine();
    return *engine;
}

void Engine::render(sf::Sprite& sprite)
{
    window.draw(sprite);
}

void Engine::render(sf::Text& text)
{
    window.draw(text);
}

sf::Event& Engine::next()
{
    deltaTime = clock.restart().asSeconds();
    window.pollEvent(event);
    window.clear();
    
    return event;
}

sf::Vector2f Engine::getMousePosition() {
    return window.mapPixelToCoords(sf::Mouse::getPosition(window));
}

class Actor {
protected:
    Actor();
    virtual ~Actor() {};

    Engine& engine;

public:
    virtual bool pass() = 0;
    virtual void execute() = 0;
};

Actor::Actor() :
    engine(Engine::instance())
{}

class ActorSprite : public Actor {
protected:
    sf::Texture texture;
    sf::Sprite sprite;

    ActorSprite(std::string fp);
    ActorSprite(std::string fp, int x, int y, int w, int h);
    ActorSprite(std::string fp, sf::IntRect ir);
    virtual ~ActorSprite();
    void draw();
    virtual bool pass() override;
};

ActorSprite::ActorSprite(std::string fp) {
    if (texture.loadFromFile(fp))
    {
        sprite.setTexture(texture);
    }
}

ActorSprite::ActorSprite(std::string fp, int x, int y, int w, int h) {
    if (texture.loadFromFile(fp, sf::IntRect(x, y, w, h)))
    {
        sprite.setTexture(texture);
    }
}

ActorSprite::ActorSprite(std::string fp, sf::IntRect ir) {
    if (texture.loadFromFile(fp, ir))
    {
        sprite.setTexture(texture);
    }
}

ActorSprite::~ActorSprite() {}

void ActorSprite::draw() {
    engine.render(sprite);
}

bool ActorSprite::pass() {
    return true;
}

class ActorText : public Actor {
protected:
    sf::Font font;
    sf::Text text;

    ActorText(std::string fp);
    ActorText(std::string fp, std::string s);
    virtual ~ActorText();
    void draw();
    virtual bool pass() override;
};

void ActorText::draw() {
    engine.render(text);
}

ActorText::~ActorText() {}

ActorText::ActorText(std::string fp) {
    if (font.loadFromFile(fp))
    {
        text.setFont(font);
    }
}

ActorText::ActorText(std::string fp, std::string s) {
    if (font.loadFromFile(fp))
    {
        text.setFont(font);
    }

    text.setString(s);
}

bool ActorText::pass() {
    return false;
}

class Score : public ActorText {
    ~Score();
public:

    Score();
    void execute() override;
    virtual bool pass() override;
};

Score::Score() : ActorText("./Assets/Fonts/FredokaOne-Regular.ttf", std::to_string(score)) {
    text.setCharacterSize(64);
}

Score::~Score() {};

void Score::execute() {
    text.setString(std::to_string(score));
    
    ActorText::draw();
}

bool Score::pass() {
    return false;
}

class Title : public ActorSprite {
    ~Title();

public:
    int value = 0;

    Title();
    void execute() override;
    virtual bool pass() override;
};

Title::Title() : ActorSprite("./Assets/GUI/Title.png") {
    sprite.setPosition(engine.window.getSize().x / 2 - sprite.getLocalBounds().width / 2,
        engine.window.getSize().y / 2 - sprite.getLocalBounds().height / 2 - 333);
}

Title::~Title() {}

void Title::execute() {
    ActorSprite::draw();
}

bool Title::pass() {
    return true;
}

class PlayButton : public ActorSprite {
    ~PlayButton();

public:
    int value = 0;

    PlayButton();
    void execute() override;
    virtual bool pass() override;
};

PlayButton::PlayButton() : ActorSprite("./Assets/GUI/Play.png") {
    sprite.setPosition(engine.window.getSize().x / 2 - sprite.getLocalBounds().width / 2,
        engine.window.getSize().y / 2 - sprite.getLocalBounds().height / 2);
}

PlayButton::~PlayButton() {}

void PlayButton::execute() {
    static float timer = 0.3f;

    if (timer > 0.3f && engine.event.type == sf::Event::MouseButtonPressed && sprite.getGlobalBounds().contains(engine.getMousePosition())) {
        value = 1;
        timer = 0.3;
    }
    else
        timer += engine.deltaTime;

    ActorSprite::draw();
}

bool PlayButton::pass() {
    return true;
}


class Cursor : public ActorSprite {
    ~Cursor();
public:
    Cursor();
    void execute() override;
    virtual bool pass() override;
};

Cursor::Cursor() : ActorSprite("./Assets/GUI/Pick.png") {
}

Cursor::~Cursor() {}

void Cursor::execute() {
    sprite.setPosition(engine.getMousePosition());
    ActorSprite::draw();
}

bool Cursor::pass() {
    return false;
}

class Board : public ActorSprite {
    friend class Piece;

    ~Board();

public:
    sf::RectangleShape line[2];
    int selection[2];
    Board();
    void addSelection();
    void execute() override;
    virtual bool pass() override;
};

Board::Board() : ActorSprite("./Assets/Sprites/Board.png"),
line{sf::RectangleShape(sf::Vector2f(128.f, 128.f)), sf::RectangleShape(sf::Vector2f(128.f, 128.f))},
selection{ 0, 0 } {
    sprite.setPosition(engine.window.getSize().x / 2 - sprite.getLocalBounds().width / 2,
        engine.window.getSize().y / 2 - sprite.getLocalBounds().height / 2);
    line[0].setFillColor(sf::Color::Transparent);
    line[1].setFillColor(sf::Color::Transparent);
}

Board::~Board() {}

void Board::addSelection() {
    static int index = 0;

    if (index > 1){
        index = 0;
    }

    for (int i = 1; i <= 8; i++) {
        line[index].setPosition((i - 1) * 128 + sprite.getPosition().x, 0);
        if ((i) * 128 + engine.window.getSize().x / 2 - sprite.getLocalBounds().width / 2 > engine.getMousePosition().x)
            break;
        selection[index] = i * 100;
    }

    for (int i = 1; i <= 8; i++) {
        line[index].setPosition(line[index].getPosition().x, (i - 1) * 128 + sprite.getPosition().y);
        if ((i) * 128 + engine.window.getSize().y / 2 - sprite.getLocalBounds().height / 2 > engine.getMousePosition().y)
            break;
        selection[index]++;
    }

    selection[index] += 101;

    index++;
}

void Board::execute() {
    static float timer = 0.3f;

    if (timer > 0.3f && engine.event.type == sf::Event::MouseButtonPressed && sprite.getGlobalBounds().contains(engine.getMousePosition())) {
        addSelection();

        timer = 0.f;
    }
    else
        timer += engine.deltaTime;

    ActorSprite::draw();
    Engine::instance().window.draw(line[0]);
    Engine::instance().window.draw(line[1]);
}

bool Board::pass() {
    return false;
}   

class Piece : public ActorSprite {
    ~Piece();
public:
    enum PIECE{
        WhitePawn,
        WhiteKnight,
        WhiteBishop,
        WhiteRook,
        WhiteQueen,
        WhiteKing,
        BlackPawn,
        BlackKnight,
        BlackBishop,
        BlackRook,
        BlackQueen,
        BlackKing
    };
    Board& board;

    sf::IntRect pieceRect(int i);
    Piece(Board& b, int i, int x, int y);
    void execute() override;
    virtual bool pass() override;
};

sf::IntRect Piece::pieceRect(int i) {
    const int TILESIZE = 128;
    int x = 0, y = 0;

    x = i;

    if (i > 5) {
        x -= 6;
        y = TILESIZE;
    }

    return sf::IntRect(x * TILESIZE, y, TILESIZE, TILESIZE);
}


Piece::Piece(Board& b, int i, int x, int y) : ActorSprite("./Assets/Sprites/Pieces.png", pieceRect(i)),
    board(b) {
    const int TILESIZE = 128;

    sprite.setPosition(sf::Vector2f((x - 1) * TILESIZE, (y - 1) * TILESIZE) + board.sprite.getPosition());
}

Piece::~Piece() {}

void Piece::execute() {
    ActorSprite::draw();
}

bool Piece::pass() {
    return true;
}

class Background : public ActorSprite {
    ~Background();
public:
    Background();
    void execute() override;
    virtual bool pass() override;
};

Background::Background() : ActorSprite("./Assets/Background/Background.jpg") {}

Background::~Background() {}

void Background::execute() {
    ActorSprite::draw();
}

bool Background::pass() {
    return false;
}

class Game {
    Engine& engine;
    std::vector<Actor*> actors;
    sf::RectangleShape bar;
    Board* board;
    PlayButton* playButton;
    sf::Music music;

    int kept;
    float timer;
    bool countdown;
    int level;
    int correct[2];

public:
    Game();
    ~Game();
    
    //void loadPieces(Board* b, int arr[64]);
    void insertActor(Actor* a);
    void clearActors();
    void play();
};

Game::Game() :
    engine(Engine::instance()),
    kept(0),
    timer(0),
    countdown(true),
    bar(sf::Vector2f(1920, 24)),
    board(new Board),
    playButton(new PlayButton),
    level(-1),
    correct() {
    bar.setFillColor(sf::Color::Red);
    bar.setPosition(0, 1056);
    if (music.openFromFile("./Assets/Audio/the-final-game.wav"))
        music.play();
}

Game::~Game() {
    actors.clear();
}

void Game::insertActor(Actor* a) {
    actors.push_back(a);
}

void Game::clearActors() {
    actors.clear();
}

void Game::play() {
    while (Engine::instance().window.isOpen()) {
        sf::Event& event = Engine::instance().next();
        static float wait = 1.f;

        if (board->selection[0] > 0) {
            board->line[0].setFillColor(sf::Color::Yellow);
        }

        if (board->selection[1] > 0) {
            board->line[1].setFillColor(sf::Color::Green);
        }

        if (board->selection[0] > 0 && board->selection[1] > 0){
            if (board->selection[0] == correct[0] && board->selection[1] == correct[1]) {
                score += (int)timer;
                timer = 0;
                wait += engine.deltaTime;
            }
        }

        if (level == 0 && playButton->value == 1) {
            timer = 0;
        }

        if (timer <= 0 && wait >= 1.f) {
            board->line[0].setFillColor(sf::Color::Transparent);
            board->line[1].setFillColor(sf::Color::Transparent);
            clearActors();

            level++;

            if (level != 0) {
                timer = GAMELENGTH;
                wait = 0.f;

                board->selection[0] = 0;
                board->selection[1] = 0;
            }

            switch (level) {
            case 0:
                timer = INFINITY;
                wait = 1.f;
                Game::insertActor(new Background);
                Game::insertActor(new Title);
                Game::insertActor(playButton);
                Game::insertActor(new Cursor);
                break;
            case 1:
                correct[0] = 702; correct[1] = 701;
                Game::insertActor(new Background);
                Game::insertActor(board);
                Game::insertActor(new Piece(*board, Piece::PIECE::BlackKing, 2, 1));
                Game::insertActor(new Piece(*board, Piece::PIECE::WhiteRook, 7, 2));
                Game::insertActor(new Piece(*board, Piece::PIECE::WhiteKing, 2, 3));
                Game::insertActor(new Score);
                Game::insertActor(new Cursor);
                break;
            case 2:
                correct[0] = 604; correct[1] = 804;
                Game::insertActor(new Background);
                Game::insertActor(board);
                Game::insertActor(new Piece(*board, Piece::PIECE::WhiteBishop, 6, 2));
                Game::insertActor(new Piece(*board, Piece::PIECE::BlackRook, 3, 3));
                Game::insertActor(new Piece(*board, Piece::PIECE::WhiteRook, 6, 4));
                Game::insertActor(new Piece(*board, Piece::PIECE::BlackPawn, 8, 4));
                Game::insertActor(new Piece(*board, Piece::PIECE::BlackPawn, 7, 5));
                Game::insertActor(new Piece(*board, Piece::PIECE::BlackKing, 8, 5));
                Game::insertActor(new Piece(*board, Piece::PIECE::WhiteKing, 7, 7));
                Game::insertActor(new Score);
                Game::insertActor(new Cursor);
                break;
            case 3:
                correct[0] = 605; correct[1] = 403;
                Game::insertActor(new Background);
                Game::insertActor(board);
                Game::insertActor(new Piece(*board, Piece::PIECE::BlackBishop, 5, 1));
                Game::insertActor(new Piece(*board, Piece::PIECE::BlackKing, 6, 1));
                Game::insertActor(new Piece(*board, Piece::PIECE::BlackPawn, 1, 2));
                Game::insertActor(new Piece(*board, Piece::PIECE::BlackPawn, 2, 2));
                Game::insertActor(new Piece(*board, Piece::PIECE::BlackPawn, 3, 2));
                Game::insertActor(new Piece(*board, Piece::PIECE::BlackPawn, 7, 2));
                Game::insertActor(new Piece(*board, Piece::PIECE::BlackKnight, 3, 3));
                Game::insertActor(new Piece(*board, Piece::PIECE::BlackPawn, 4, 3));
                Game::insertActor(new Piece(*board, Piece::PIECE::BlackPawn, 8, 3));
                Game::insertActor(new Piece(*board, Piece::PIECE::BlackBishop, 3, 4));
                Game::insertActor(new Piece(*board, Piece::PIECE::WhiteBishop, 3, 5));
                Game::insertActor(new Piece(*board, Piece::PIECE::WhiteBishop, 6, 5));
                Game::insertActor(new Piece(*board, Piece::PIECE::BlackKnight, 7, 5));
                Game::insertActor(new Piece(*board, Piece::PIECE::BlackQueen, 8, 5));
                Game::insertActor(new Piece(*board, Piece::PIECE::WhiteKnight, 6, 3));
                Game::insertActor(new Piece(*board, Piece::PIECE::WhitePawn, 1, 7));
                Game::insertActor(new Piece(*board, Piece::PIECE::WhitePawn, 2, 7));
                Game::insertActor(new Piece(*board, Piece::PIECE::WhitePawn, 7, 7));
                Game::insertActor(new Piece(*board, Piece::PIECE::WhitePawn, 8, 7));
                Game::insertActor(new Piece(*board, Piece::PIECE::WhiteRook, 5, 8));
                Game::insertActor(new Piece(*board, Piece::PIECE::WhiteRook, 6, 8));
                Game::insertActor(new Piece(*board, Piece::PIECE::WhiteKing, 8, 8));
                Game::insertActor(new Score);
                Game::insertActor(new Cursor);
                break;
            case 4:
                correct[0] = 204; correct[1] = 203;
                Game::insertActor(new Background);
                Game::insertActor(board);
                Game::insertActor(new Piece(*board, Piece::PIECE::BlackKing, 3, 1));
                Game::insertActor(new Piece(*board, Piece::PIECE::WhitePawn, 3, 2));
                Game::insertActor(new Piece(*board, Piece::PIECE::BlackPawn, 1, 3));
                Game::insertActor(new Piece(*board, Piece::PIECE::WhiteKing, 3, 3));
                Game::insertActor(new Piece(*board, Piece::PIECE::WhitePawn, 2, 4));
                Game::insertActor(new Score);
                Game::insertActor(new Cursor);
                break;
            case 5:
                correct[0] = 203; correct[1] = 202;
                Game::insertActor(new Background);
                Game::insertActor(board);
                Game::insertActor(new Piece(*board, Piece::PIECE::BlackKing, 3, 1));
                Game::insertActor(new Piece(*board, Piece::PIECE::WhitePawn, 3, 2));
                Game::insertActor(new Piece(*board, Piece::PIECE::BlackPawn, 1, 4));
                Game::insertActor(new Piece(*board, Piece::PIECE::WhiteKing, 3, 3));
                Game::insertActor(new Piece(*board, Piece::PIECE::WhitePawn, 2, 3));
                Game::insertActor(new Score);
                Game::insertActor(new Cursor);
                break;
            default:
            Engine::instance().window.close();
                break;
            }
        }

        switch (event.type)
        {
        case sf::Event::Closed:
            Engine::instance().window.close();
            break;
        default:
            break;
        }

        for (Actor* a : actors)
            a->execute();

        if (countdown) {
            timer -= engine.deltaTime;
            Engine::instance().window.draw(bar);
        }

        bar.setScale(sf::Vector2f((float)(timer / GAMELENGTH), 1.f));
        Engine::instance().window.display();
    }
}

int main()
{
    Game g;
    g.play();
}

/*

void Game::insertActor(Actor* a) {
    actors.insert(actors.end() - kept, a);
}

void Game::clearActors() {
    int i = -1;
    kept = 0;

    for (Actor* a : actors) {
        i++;
        if (!a->pass()) {
            kept++;
            continue;
        }

        actors.erase(actors.begin() + i);
    }

    printf("%d\n", kept);
}{
        {605, 403},
        {605, 502},
        {805, 804}
    } 

Board* board = new Board();

Actor* Level[5][5] = {
{
    new Background(),
    board,
    new Cursor()
},
{
    new Piece(*board, Piece::PIECE::BlackBishop, 1, 1),
    new Piece(*board, Piece::PIECE::WhiteKing, 2, 1)
}
};

int arr[64] = {
     10, 8,  9, 11, 12,  9,  8, 10,
     7,  7,  7,  7,  7,  7,  7,  7,
     0,  0,  0,  0,  0,  0,  0,  0,
     0,  0,  0,  0,  0,  0,  0,  0,
     0,  0,  0,  0,  0,  0,  0,  0,
     0,  0,  0,  0,  0,  0,  0,  0,
     1,  1,  1,  1,  1,  1,  1,  1,
     4,  2,  3,  5,  6,  3,  2,  4
};

void Game::insertActors(Actor* a[][5]) {
    for (int i = 0; i <= 5; i++)
        if(a[level][i] != nullptr)
            actors.insert(actors.end() - kept, a[level][i]);
}


void Game::loadPieces(Board* b, int arr[64]) {
    int y = 1, x = 1;

    for (int i = 0; i < 64; i++) {
        if (x == 8) {
            y++;
            x = 1;
        }

        if (arr[i] != 0) {
            Piece* p = new Piece(*b, arr[i], x, y);
            actors.insert(actors.end() - kept, p);
        }

        x++;
    }
}*/