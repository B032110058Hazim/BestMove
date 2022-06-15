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

    float deltaTime;

    Engine();

public:
    sf::RenderWindow window;
    sf::View view;
    sf::Event event;

    Engine(Engine& other) = delete;
    void operator=(const Engine&) = delete;
    static Engine& instance();
    void render(sf::Sprite& sprite);
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
    window.setView(view);
    srand(time(NULL));
    window.setMouseCursorVisible(false);
}

Engine& Engine::instance() {
    static Engine* engine = new Engine();
    return *engine;
}

void Engine::render(sf::Sprite& sprite)
{
    window.draw(sprite);
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
    virtual ~ActorSprite();
    void draw();
};

void ActorSprite::draw() {
    engine.render(sprite);
}

ActorSprite::~ActorSprite() {}

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

class Cursor : public ActorSprite {
    ~Cursor();
public:
    sf::FloatRect cursorHitbox;

    Cursor();
    void execute() override;
};

Cursor::Cursor() : ActorSprite("./Assets/GUI/Pick.png") {
}

Cursor::~Cursor() {}

void Cursor::execute() {
    cursorHitbox = sprite.getGlobalBounds();
    sprite.setPosition(engine.getMousePosition());
    ActorSprite::draw();
}

class Board : public ActorSprite {
    Cursor& cursor;
    int selection[2];
    ~Board();
public:
    Board(Cursor& c);
    void addSelection();
    void execute() override;
};

Board::Board(Cursor& c) : ActorSprite("./Assets/Sprites/Board.png"),
cursor(c),
selection{ 0, 0 } {
    sprite.setPosition(engine.window.getSize().x / 2 - sprite.getLocalBounds().width / 2,
        engine.window.getSize().y / 2 - sprite.getLocalBounds().height / 2);
}

Board::~Board() {}

void Board::addSelection() {
    sf::FloatRect intersection;

    sprite.getGlobalBounds().intersects(cursor.cursorHitbox, intersection);

    for (int i = 0; i < 8; i++) {
        if ((i + 1) * 128 > intersection.left)
            break;
        selection[0] = i;
    }

    for (int i = 0; i < 8; i++) {
        if ((i + 1) * 128 > intersection.top)
            break;
        selection[1] = i;
    }

    printf("%f, %f\n", intersection.left, intersection.top);
}

void Board::execute() {
    if (engine.event.type == sf::Event::MouseButtonReleased && sprite.getGlobalBounds().contains(engine.getMousePosition()))
        addSelection();
    ActorSprite::draw();
}

short int level[64];

int main()
{
    std::vector<Actor*> actors;

    Cursor* c = new Cursor();
    Board* as = new Board(*c);
    //Board* b = new Board("./Assets/Sprites/Pieces.png", 3 * 128, 0, 128, 128);
    actors.push_back(as);
    actors.push_back(c);
    //actors.push_back(b);

    while (Engine::instance().window.isOpen()) {
        sf::Event event = Engine::instance().next();

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
    
    actors.clear();
}