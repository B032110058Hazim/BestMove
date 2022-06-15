#include <vector>
#include <SFML/Graphics.hpp>

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
    sf::Event& next();
    sf::Vector2f getMousePosition();
};

Engine::Engine():
    video(1920, 1080),
    window(this->video, "Game"),
    //window(this->video, "Game", sf::Style::Fullscreen),
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

    text.setString("Hello world");
}

bool ActorText::pass() {
    return true;
}

class Cursor : public ActorSprite {
    ~Cursor();
public:
    sf::FloatRect cursorHitbox;

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

    sf::IntRect pieceRect(int i);
    Piece(int i);
    void execute() override;
    virtual bool pass() override;
};

sf::IntRect Piece::pieceRect(int i) {
    const int TILESIZE = 128;
    int x = 0, y = 0;

    x = i;

    if (i > 5) {
        x =- 5;
        y = TILESIZE;
    }

    return sf::IntRect(x, y, TILESIZE, TILESIZE);
}

Piece::Piece(int i) : ActorSprite("./Assets/Sprites/Pieces.png", pieceRect(i)) {}

Piece::~Piece() {}

void Piece::execute() {
    ActorSprite::draw();
}

bool Piece::pass() {
    return true;
}

class Board : public ActorSprite {
    int selection[2];
    ~Board();
public:
    Board();
    void addSelection();
    void execute() override;
    virtual bool pass() override;
};

Board::Board() : ActorSprite("./Assets/Sprites/Board.png"),
selection{ 0, 0 } {
    sprite.setPosition(engine.window.getSize().x / 2 - sprite.getLocalBounds().width / 2,
        engine.window.getSize().y / 2 - sprite.getLocalBounds().height / 2);
}

Board::~Board() {}

void Board::addSelection() {
    static int index = 0;

    if (index > 1)
        index = 0;

    for (int i = 1; i < 8; i++) {
        if ((i) * 128  + engine.window.getSize().x / 2 - sprite.getLocalBounds().width / 2 > engine.getMousePosition().x)
            break;
        selection[index] = i * 100;
    }

    for (int i = 1; i < 8; i++) {
        if ((i) * 128 + engine.window.getSize().y / 2 - sprite.getLocalBounds().height / 2 > engine.getMousePosition().y)
            break;
        selection[index]++;
    }

    selection[index] += 101;

    index++;
}

void Board::execute() {
    static float timer = 0.1f;

    if (timer > 0.1f && engine.event.type == sf::Event::MouseButtonPressed && sprite.getGlobalBounds().contains(engine.getMousePosition())) {
        addSelection();

        timer = 0.f;
    }
    else
        timer += engine.deltaTime;
    
    ActorSprite::draw();
}

bool Board::pass() {
    return true;
}

int arr[64] = {
    -4, -2, -3, -5, -6, -3, -2, -4,
    -1, -1, -1, -1, -1, -1, -1, -1,
     0,  0,  0,  0,  0,  0,  0,  0,
     0,  0,  0,  0,  0,  0,  0,  0,
     0,  0,  0,  0,  0,  0,  0,  0,
     0,  0,  0,  0,  0,  0,  0,  0,
     1,  1,  1,  1,  1,  1,  1,  1,
     4,  2,  3,  5,  6,  3,  2,  4
};

class Game {
    Engine& engine;
    std::vector<Actor*> actors;

    int kept;
    int timer;
    bool countdown;

public:
    Game();
    ~Game();
    
    void clearActors();
    void play();
};

Game::Game():
    engine(Engine::instance()),
    kept(0),
    timer(100),
    countdown(false) {

    Cursor* c = new Cursor();
    Board* as = new Board();
    Piece* b = new Piece(Piece::PIECE::WhitePawn);
    
    actors.insert(actors.end() - kept, as);
    actors.insert(actors.end() - kept, b);
    actors.insert(actors.end() - kept, c);

}

Game::~Game() {
    actors.clear();
}

void Game::clearActors() {
    kept = 0;

    for (Actor* a : actors) {
        if (a->pass()) {
            actors.erase(actors.begin() + kept);
            kept--;
        }
        kept++;
        printf("%i", kept);
    }
}

void Game::play() {

    while (Engine::instance().window.isOpen()) {
        sf::Event& event = Engine::instance().next();

        if(countdown)
            timer -= engine.deltaTime;

        if (timer <= 0) {
            clearActors();
            timer = 100;
        }

        kept = 0;

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

        Engine::instance().window.display();
    }
}

int main()
{
    Game g;
    g.play();
}



//kept = 0;
//
//for (Actor* a : actors) {
//  if (a->pass()) {
//      actors.erase(actors.begin() + kept);
//      kept--;
//  }
//kept++;
//printf("%i", kept);