#include <SFML/Graphics.hpp>
#include <SFML/Graphics/Vertex.hpp>
#include <cstdint>
#include <iostream>
#include <random>
#include <vector>

const int WINDOW_WIDTH = 1080;
const int WINDOW_HEIGHT = 720;
const float IMAGE_MAX_DIMENSION = 256;
const int SCALING = 2;

struct Config {
    std::string inputPath = "./assets/target.png";
    std::string spritePath = "./assets/sprite.png";
    std::string outputPath = "output.png";
    int dnaLength = 500;
};

Config parseArguments(int argc, char* argv[]) {
    Config config;
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if ((arg == "-i" || arg == "--input") && i + 1 < argc) {
            config.inputPath = argv[++i];
        } else if ((arg == "-o" || arg == "--output") && i + 1 < argc) {
            config.outputPath = argv[++i];
        } else if ((arg == "-s" || arg == "--sprite") && i + 1 < argc) {
            config.spritePath = argv[++i];
        } else if ((arg == "-d" || arg == "--dna") && i + 1 < argc) {
            config.dnaLength = std::stoi(argv[++i]);
        } else if (arg == "-h" || arg == "--help") {
            std::cout << "Usage: " << argv[0] << " [options]\n"
                      << "Options:\n"
                      << "  -i, --input <path>   Path to source image (default: ./assets/target.png)\n"
                      << "  -s, --sprite <path>  Path to sprite image (default: input)\n"
                      << "  -o, --output <path>  Path to save result (default: output.png)\n"
                      << "  -d, --dna <number>   Number of shapes to draw (default: 500)\n";
            exit(0);
        }
    }
    return config;
}

std::mt19937 rng(std::random_device{}());
template <typename T>
T Rand(T min, T max) {
    if constexpr (std::is_integral_v<T>) {
        std::uniform_int_distribution<T> dist(min, max);
        return dist(rng);
    } else {
        std::uniform_real_distribution<T> dist(min, max);
        return dist(rng);
    }
}

using gene = sf::Sprite;

struct Individual {
    std::vector<gene> dna;
    long long error;
};

// Batching multiple sprites into 1 render with VertexArray
void fastRender(sf::RenderTarget& target, const std::vector<gene>& dna, const sf::Texture& texture) {
    static sf::VertexArray vertices(sf::PrimitiveType::Triangles, dna.size() * 6);

    sf::Vector2u texSize = texture.getSize();
    sf::Vector2f fTexSize((float)texSize.x, (float)texSize.y);

    for (size_t i = 0; i < dna.size(); ++i) {
        const sf::Sprite& sprite = dna[i];

        sf::Transform trans = sprite.getTransform();

        sf::FloatRect local = sprite.getLocalBounds();

        sf::Vector2f tl = trans.transformPoint({0.f, 0.f});
        sf::Vector2f tr = trans.transformPoint({local.size.x, 0.f});
        sf::Vector2f br = trans.transformPoint({local.size.x, local.size.y});
        sf::Vector2f bl = trans.transformPoint({0.f, local.size.y});

        sf::Color col = sprite.getColor();

        size_t vIdx = i * 6;

        vertices[vIdx + 0] = sf::Vertex({tl, col, {0.f, 0.f}});
        vertices[vIdx + 1] = sf::Vertex({tr, col, {fTexSize.x, 0.f}});
        vertices[vIdx + 2] = sf::Vertex({bl, col, {0.f, fTexSize.y}});
        vertices[vIdx + 3] = sf::Vertex({tr, col, {fTexSize.x, 0.f}});
        vertices[vIdx + 4] = sf::Vertex({br, col, {fTexSize.x, fTexSize.y}});
        vertices[vIdx + 5] = sf::Vertex({bl, col, {0.f, fTexSize.y}});
    }

    target.draw(vertices, sf::RenderStates(&texture));
}

float Clamp(float val, float min, float max) {
    if (val < min)
        return min;
    if (val > max)
        return max;
    return val;
}

uint8_t ClampColor(int val) {
    if (val < 0)
        return 0;
    if (val > 255)
        return 255;
    return (uint8_t)val;
}

void mutate(gene& gene, float canvasWidth, float canvasHeight) {
    // move
    float moveX = canvasWidth * Rand<float>(-0.05f, 0.05f);
    float moveY = canvasHeight * Rand<float>(-0.05f, 0.05f);
    gene.move({moveX, moveY});

    sf::Vector2f pos = gene.getPosition();
    pos.x = Clamp(pos.x, 0.0f, canvasWidth);
    pos.y = Clamp(pos.y, 0.0f, canvasHeight);
    gene.setPosition(pos);  // wrap position

    // rotate
    sf::Angle angle = sf::degrees(Rand<float>(0.0f, 360.0f));
    gene.rotate(angle);

    // scale
    float currentScalex = gene.getScale().x;
    float scaleDeltax = Rand<float>(-0.5f, 0.5f);
    float newScalex = currentScalex + scaleDeltax;
    if (newScalex < 0.1f)
        newScalex = 0.1f;
    if (newScalex > 5.0f)
        newScalex = 5.0f;

    float currentScaley = gene.getScale().y;
    float scaleDeltay = Rand<float>(-0.5f, 0.5f);
    float newScaley = currentScaley + scaleDeltay;
    if (newScaley < 0.1f)
        newScaley = 0.1f;
    if (newScaley > 5.0f)
        newScaley = 5.0f;

    gene.setScale({newScalex, newScaley});

    // color
    sf::Color c = gene.getColor();
    c.r = ClampColor((int)c.r + Rand<int>(-50, 50));
    c.g = ClampColor((int)c.g + Rand<int>(-50, 50));
    c.b = ClampColor((int)c.b + Rand<int>(-50, 50));
    c.a = ClampColor((int)c.a + Rand<int>(-50, 50));
    if (c.a > 220)
        c.a = 220;
    gene.setColor(c);
}

long long evaluate(const Individual& ind, const uint8_t* targetPixels, sf::RenderTexture& canvas, sf::Texture& geneTexture) {
    canvas.clear();
    fastRender(canvas, ind.dna, geneTexture);
    canvas.display();

    sf::Image geneImage = canvas.getTexture().copyToImage();
    const uint8_t* genePixels = geneImage.getPixelsPtr();

    long long totalError = 0;
    auto [width, height] = canvas.getSize();
    size_t totalBytes = width * height * 4;  // RGBA

    for (size_t i = 0; i < totalBytes; i += 4) {
        int diffR = abs(targetPixels[i] - genePixels[i]);
        int diffG = abs(targetPixels[i + 1] - genePixels[i + 1]);
        int diffB = abs(targetPixels[i + 2] - genePixels[i + 2]);

        totalError += (diffR + diffG + diffB);
        int diffA = abs(targetPixels[i + 3] - genePixels[i + 3]);
        totalError += diffA * diffA;
    }

    return totalError;
}

sf::Texture resizeTexture(sf::Texture src, float maxDimension) {
    auto [width, height] = src.getSize();
    float scaleFactor =
        (width > height) ? maxDimension / (float)width : maxDimension / (float)height;

    sf::Sprite spr(src);
    spr.scale({scaleFactor, scaleFactor});
    auto [sprWidth, sprHeight] = spr.getGlobalBounds().size;
    sf::ContextSettings settings;
    sf::RenderTexture render({(unsigned)sprWidth, (unsigned)sprHeight}, settings);
    render.draw(spr, sf::BlendNone);
    render.display();
    sf::Texture resizedTexture(render.getTexture());
    resizedTexture.setSmooth(true);

    return resizedTexture;
}

int main(int argc, char* argv[]) {
    Config config = parseArguments(argc, argv);
    std::cout << "Running with:\n"
              << "  Input: " << config.inputPath << "\n"
              << "  Sprite: " << config.spritePath << "\n"
              << "  Output: " << config.outputPath << "\n"
              << "  DNA Length: " << config.dnaLength << "\n";

    sf::RenderWindow window(sf::VideoMode({WINDOW_WIDTH, WINDOW_HEIGHT}), "Genetic Photo Recreation");

    sf::Texture sourceTex(config.inputPath);
    sf::Texture resizedSrcTex = resizeTexture(sourceTex, IMAGE_MAX_DIMENSION);

    sf::Texture geneTex(config.spritePath);
    sf::Texture resizedGeneTex = resizeTexture(geneTex, IMAGE_MAX_DIMENSION / 5);

    sf::Image targetImage = resizedSrcTex.copyToImage();
    const uint8_t* targetPixelPtr = targetImage.getPixelsPtr();

    sf::RenderTexture canvas({resizedSrcTex.getSize().x, resizedSrcTex.getSize().y});
    canvas.setSmooth(true);
    auto [canvasWidth, canvasHeight] = canvas.getSize();
    canvas.clear();

    Individual parent;
    for (int i = 0; i < config.dnaLength; i++) {
        gene gene(resizedGeneTex);
        gene.setOrigin({gene.getLocalBounds().size.x / 2.0f, gene.getLocalBounds().size.y / 2.0f});
        gene.setPosition({canvasWidth / 2.0f, canvasHeight / 2.0f});
        gene.setPosition({(float)Rand<int>(0, canvasWidth), (float)Rand<int>(0, canvasHeight)});
        gene.setRotation(sf::degrees(Rand<float>(0, 360)));
        parent.dna.push_back(gene);
        canvas.draw(gene);
    }
    parent.error = evaluate(parent, targetPixelPtr, canvas, geneTex);
    canvas.display();

    sf::Sprite sourceSpr(resizedSrcTex);
    sourceSpr.setOrigin(
        {sourceSpr.getLocalBounds().size.x / 2.0f, sourceSpr.getLocalBounds().size.y / 2.0f});
    sourceSpr.setPosition({WINDOW_WIDTH / 4.0f, WINDOW_HEIGHT / 2.0f});
    sourceSpr.setScale({SCALING, SCALING});

    sf::Sprite resultSpr(canvas.getTexture());
    resultSpr.setOrigin(
        {resultSpr.getLocalBounds().size.x / 2.0f, resultSpr.getLocalBounds().size.y / 2.0f});
    resultSpr.setPosition({3 * WINDOW_WIDTH / 4.0f, WINDOW_HEIGHT / 2.0f});
    resultSpr.setScale({SCALING, SCALING});

    sf::Font font("./assets/Roboto-Regular.ttf");

    sf::Text statsText(font);
    statsText.setCharacterSize(24);
    statsText.setFillColor(sf::Color::White);
    statsText.setPosition({10.f, 10.f});

    long long geneCnt = 0;
    while (window.isOpen()) {
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                if (canvas.getTexture().copyToImage().saveToFile(config.outputPath)) {
                    std::cout << "Successfully saved to: " << config.outputPath << std::endl;
                } else {
                    std::cerr << "Failed to save image" << std::endl;
                }
                window.close();
            }
        }
        sf::Clock computeClock;

        while (computeClock.getElapsedTime().asMilliseconds() < 16) {
            geneCnt++;
            Individual child = parent;

            int geneIndex = Rand<int>(0, child.dna.size() - 1);
            mutate(child.dna[geneIndex], (float)canvasWidth, (float)canvasHeight);

            child.error = evaluate(child, targetPixelPtr, canvas, geneTex);

            if (child.error < parent.error) {
                parent = child;
                // std::cout << "New Best: " << parent.error << "(generation: " << geneCnt << ")" << std::endl;
            }
        }

        canvas.clear();
        for (gene gene : parent.dna) {
            canvas.draw(gene);
        }
        canvas.display();

        std::string info = "Generation: " + std::to_string(geneCnt) +
                           "\nError: " + std::to_string(parent.error);
        statsText.setString(info);

        window.clear();
        window.draw(sourceSpr);
        window.draw(resultSpr);
        window.draw(statsText);
        window.display();
    }
}
