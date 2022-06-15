#include <vector>
#include <SFML/Graphics.hpp>

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
};

Engine::Engine():
    video(1920, 1080),
    window(this->video, "Game"),
    view(sf::FloatRect(0, 0, 1920, 1080)),
    deltaTime(0.f)
{
    window.setView(view);
    srand(time(NULL));
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

class Board : public ActorSprite {
    ~Board();
public:
    Board();
    void execute() override;
};

Board::Board() : ActorSprite("./Assets/Sprites/BlackTile.png") {}

Board::~Board() {}

void Board::execute() {
    ActorSprite::draw();
}

short int level[64];

int main()
{
    Engine::instance;
    std::vector<Actor*> actors;

    //Board board[64];

    Board* as = new Board();
    //Board* b = new Board("./Assets/Sprites/Pieces.png", 3 * 128, 0, 128, 128);
    actors.push_back(as);
    //actors.push_back(b);

    while (Engine::instance().window.isOpen()) {
        sf::Event event = Engine::instance().next();

        if (event.type == sf::Event::Closed)
            Engine::instance().window.close();

        for (Actor* a : actors)
            a->execute();

        Engine::instance().window.display();
    }
    
    actors.clear();
}