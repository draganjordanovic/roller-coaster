#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include "Util.h"

#include <vector>
#include <cmath>

struct Vertex {
    float x, y;
    float u, v;
    float r, g, b;
};

// KONSTANTE ZA STAZU
constexpr int   NUM_TRACK_POINTS = 200;
constexpr float NUM_HILLS = 5.0f;

// KONSTANTE ZA VAGON
constexpr int   WAGON_SEGMENTS = 4;
constexpr int   WAGON_VERTEX_COUNT_PER_SEGMENT = 4;
constexpr float WAGON_SEGMENT_SIZE = 0.1f;
constexpr float WAGON_Y_BOTTOM = -0.9f;
constexpr float WAGON_Y_TOP = WAGON_Y_BOTTOM + WAGON_SEGMENT_SIZE;
// Početni x za prvi segment (pre pomeranja po stazi).
constexpr float WAGON_X_START = -0.3f;
constexpr float WAGON_GAP = 0.02f;

int endProgram(std::string message) {
    std::cout << message << std::endl;
    glfwTerminate();
    return -1;
}

void preprocessTexture(unsigned& texture, const char* filepath) {
    texture = loadImageToTexture(filepath); // Učitavanje teksture
    glBindTexture(GL_TEXTURE_2D, texture);  // Vezujemo se za teksturu kako bismo je podesili

    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

// buildTrack
// Popunjava:
// - vertices: tačke staze
// - trackS:   kumulativne duzine duz staze
// - trackTotalLength: ukupna duzina staze
void buildTrack(std::vector<Vertex>& vertices,
    std::vector<float>& trackS,
    float& trackTotalLength)
{
    vertices.clear();
    vertices.reserve(NUM_TRACK_POINTS);

    for (int i = 0; i < NUM_TRACK_POINTS; ++i) {
        float t = i / float(NUM_TRACK_POINTS - 1);

        float x = -0.9f + t * 1.8f;        // x se linijski menja od -0.9 do 0.9
        float bigHill = std::sin(3.14159f * (t + 0.1f));                 // 1 veliko brdo
        float midHill = 0.6f * std::sin(3.0f * 3.14159f * (t - 0.15f));  // 3 srednja
        float smallWiggle = 0.3f * std::sin(8.0f * 3.14159f * t);            // sitne neravnine

        float hills = bigHill + midHill + smallWiggle;
        float yBase = -0.45f;
        float yAmp = 0.42f;

        float y = yBase + yAmp * hills;
        vertices.push_back({ x, y, 0.0f, 0.0f, 0.7f, 0.7f, 0.7f });
    }

    int trackVertexCount = NUM_TRACK_POINTS;

    // trackS[i] zna razdaljinu od pocetka do verteksa i.
    trackS.resize(trackVertexCount);
    trackS[0] = 0.0f;
    for (int i = 1; i < trackVertexCount; ++i) {
        float dx = vertices[i].x - vertices[i - 1].x;
        float dy = vertices[i].y - vertices[i - 1].y;
        float dist = std::sqrt(dx * dx + dy * dy);
        trackS[i] = trackS[i - 1] + dist;
    }
    // ukupna dužina staze je kumulativna dužina do poslednjeg verteksa
    trackTotalLength = trackS[trackVertexCount - 1];
}

// buildTrain
void buildTrain(std::vector<Vertex>& vertices,
    std::vector<float>& segmentCenterX,
    float& segmentCenterY,
    int& wagonStartIndex)
{
    segmentCenterX.assign(WAGON_SEGMENTS, 0.0f);
    segmentCenterY = WAGON_Y_BOTTOM;
    wagonStartIndex = static_cast<int>(vertices.size());

    // Dodajem segmente vagona jedan iza drugog
    for (int i = 0; i < WAGON_SEGMENTS; ++i) {        
        float x0 = WAGON_X_START + i * (WAGON_SEGMENT_SIZE + WAGON_GAP);    // Levi x        
        float x1 = x0 + WAGON_SEGMENT_SIZE; // Desni x
        float r = 0.2f, g = 0.4f, b = 0.9f; //boja vagona
        segmentCenterX[i] = (x0 + x1) / 2.0f;
        // Četiri verteksa kvadrata
        vertices.push_back({ x0, WAGON_Y_BOTTOM, 0.0f, 0.0f, r, g, b }); // dole levo
        vertices.push_back({ x1, WAGON_Y_BOTTOM, 1.0f, 0.0f, r, g, b }); // dole desno
        vertices.push_back({ x1, WAGON_Y_TOP, 1.0f, 1.0f, r, g, b }); // gore desno
        vertices.push_back({ x0, WAGON_Y_TOP, 0.0f, 1.0f, r, g, b }); // gore levo
    }
}

// getPointOnTrack
// Za datu duzinu s (udaljenost duz staze od početka) vraća tačku (x,y) na sinama
void getPointOnTrack(float s,
    float& outX,
    float& outY,
    const std::vector<Vertex>& vertices,
    const std::vector<float>& trackS,
    float trackTotalLength)
{
    const int TRACK_VERTEX_COUNT = static_cast<int>(trackS.size());

    while (s < 0.0f)             s += trackTotalLength;
    while (s > trackTotalLength) s -= trackTotalLength;

    //trazim indeks segmenta u kome se nalazi dužina s.
    int i = 0;
    while (i < TRACK_VERTEX_COUNT - 1 && trackS[i + 1] < s) {
        ++i;
    }

    // duzina segmenta [i, i+1].
    float segLen = trackS[i + 1] - trackS[i];
    // razdaljina od pocetka segmenta do s
    float tLocal = (segLen > 0.0f) ? (s - trackS[i]) / segLen : 0.0f;

    // Koordinate krajeva segmenta
    float x0 = vertices[i].x;
    float y0 = vertices[i].y;
    float x1 = vertices[i + 1].x;
    float y1 = vertices[i + 1].y;

    // pomeri se tLocal procenata unutar segmenta
    outX = x0 + tLocal * (x1 - x0);
    outY = y0 + tLocal * (y1 - y0);
}

int main()
{

    // Inicijalizacija GLFW
    glfwInit();

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(1800, 1300, "Rolerkoster", NULL, NULL);
    if (window == NULL) return endProgram("Prozor nije uspeo da se kreira.");
    glfwMakeContextCurrent(window);

    if (glewInit() != GLEW_OK) return endProgram("GLEW nije uspeo da se inicijalizuje.");

    unsigned int basicShader = createShader("basic.vert", "basic.frag");

    int uOffsetLocation = glGetUniformLocation(basicShader, "uOffset");

    int uUseTextureLocation = glGetUniformLocation(basicShader, "useTexture");
    int uTexLocation = glGetUniformLocation(basicShader, "uTex");

    unsigned int wagonTexture = 0;
    preprocessTexture(wagonTexture, "res/car.png");


    glUseProgram(basicShader);
    glUniform1i(uTexLocation, 0);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    //Priprema staze
    std::vector<Vertex> vertices;
    std::vector<float> trackS;
    float trackTotalLength = 0.0f;

    buildTrack(vertices, trackS, trackTotalLength);
    int TRACK_VERTEX_COUNT = static_cast<int>(trackS.size());

    //Priprema voza
    std::vector<float> segmentCenterX(WAGON_SEGMENTS);
    float segmentCenterY = 0.0f;
    int WAGON_START_INDEX = 0;
    buildTrain(vertices, segmentCenterX, segmentCenterY, WAGON_START_INDEX);
    const int WAGON_TOTAL_VERTEX_COUNT = WAGON_SEGMENTS * WAGON_VERTEX_COUNT_PER_SEGMENT;
    (void)WAGON_TOTAL_VERTEX_COUNT;

    unsigned int VAO;
    unsigned int VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER,
        vertices.size() * sizeof(Vertex),
        vertices.data(),
        GL_STATIC_DRAW);

    glVertexAttribPointer(
        0,
        2,
        GL_FLOAT,
        GL_FALSE,
        sizeof(Vertex),
        (void*)0
    );
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(
        1,
        2,
        GL_FLOAT,
        GL_FALSE,
        sizeof(Vertex),
        (void*)(2 * sizeof(float))
    );
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(
        2,
        3,
        GL_FLOAT,
        GL_FALSE,
        sizeof(Vertex),
        (void*)(4 * sizeof(float))
    );
    glEnableVertexAttribArray(2);

    glClearColor(0.3f, 0.1f, 0.6f, 1.0f);

    // animacione promenljive

    double lastTime = glfwGetTime();
    const float SPEED = 0.5f;
    const float SEGMENT_SPACING = WAGON_SEGMENT_SIZE + WAGON_GAP;

    float sHead = (WAGON_SEGMENTS - 1) * SEGMENT_SPACING;
    bool isRunning = false;

    while (!glfwWindowShouldClose(window))
    {
        double currentTime = glfwGetTime();
        double deltaTime = currentTime - lastTime;
        lastTime = currentTime;

        if (!isRunning && glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS) {
            isRunning = true;
        }

        if (isRunning) {
            sHead += SPEED * static_cast<float>(deltaTime);
            float maxHead = trackTotalLength;

            if (sHead >= maxHead) {
                sHead = maxHead;
                isRunning = false;
            }
        }

        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(basicShader);
        glBindVertexArray(VAO);

        glUniform1i(uUseTextureLocation, GL_FALSE);
        glUniform2f(uOffsetLocation, 0.0f, 0.0f);
        glDrawArrays(GL_LINE_STRIP, 0, TRACK_VERTEX_COUNT);


        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, wagonTexture);
        glUniform1i(uUseTextureLocation, GL_TRUE);
        for (int i = 0; i < WAGON_SEGMENTS; ++i) {
            float sSeg = sHead - i * SEGMENT_SPACING;

            float pathXSeg, pathYSeg;
            getPointOnTrack(sSeg, pathXSeg, pathYSeg,
                vertices, trackS, trackTotalLength);

            //offset: koliko da pomerimo ovaj segment
            float offsetXSeg = pathXSeg - segmentCenterX[i];
            float offsetYSeg = pathYSeg - segmentCenterY;

            glUniform2f(uOffsetLocation, offsetXSeg, offsetYSeg);
            int startIndex = WAGON_START_INDEX + i * WAGON_VERTEX_COUNT_PER_SEGMENT;

            glDrawArrays(GL_TRIANGLE_FAN, startIndex, WAGON_VERTEX_COUNT_PER_SEGMENT);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
