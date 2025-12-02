#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include "Util.h"

#include <vector>
#include <cmath>

struct Vertex {
    float x, y;
    float r, g, b;
};

int endProgram(std::string message) {
    std::cout << message << std::endl;
    glfwTerminate();
    return -1;
}

int main()
{
    // Inicijalizacija GLFW i postavljanje na verziju 3 sa programabilnim pajplajnom
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Formiranje prozora za prikaz sa datim dimenzijama i naslovom
    GLFWwindow* window = glfwCreateWindow(1800, 1300, "Vezba 1", NULL, NULL);
    if (window == NULL) return endProgram("Prozor nije uspeo da se kreira.");
    glfwMakeContextCurrent(window);

    // Inicijalizacija GLEW
    if (glewInit() != GLEW_OK) return endProgram("GLEW nije uspeo da se inicijalizuje.");

    unsigned int basicShader = createShader("basic.vert", "basic.frag");
    int uOffsetLocation = glGetUniformLocation(basicShader, "uOffset"); //handler preko koga menjam uOffset

    std::vector<Vertex> vertices;

    // Kriva putanja pomocu sinusa
    const int NUM_TRACK_POINTS = 200; //vise tacaka da putanja bude glatka
    const float NUM_HILLS = 5.0f;

    // Parametri vagona
    const int WAGON_SEGMENTS = 4;      // 4 mala kvadrata
    const int WAGON_VERTEX_COUNT_PER_SEGMENT = 4;
    const float WAGON_SEGMENT_SIZE = 0.15f;   // sirina = visina (kvadrat)
    const float WAGON_Y_BOTTOM = -0.9f;
    const float WAGON_Y_TOP = WAGON_Y_BOTTOM + WAGON_SEGMENT_SIZE;
    const float WAGON_X_START = -0.3f; // pocetak prvog kvadrata po x-osi
    const float WAGON_GAP = 0.02f;           // razmak između kvadrata

    // Centri segmenata
    std::vector<float> segmentCenterX(WAGON_SEGMENTS);
    float segmentCenterY = (WAGON_Y_BOTTOM + WAGON_Y_TOP) / 2.0f;

    for (int i = 0; i < NUM_TRACK_POINTS; ++i) {
        float t = i / float(NUM_TRACK_POINTS - 1);   // t ide od 0 do 1

        // x ide od -0.9 do 0.9 (vodoravno preko ekrana)
        float x = -0.9f + t * 1.8f;

        // sinus sa ~3 brda (3 * PI → ~1.5 periode → 3 uzbrdice/nizbrdice)
        float hills = sinf(t * NUM_HILLS * 3.14159f);

        // y bazna visina -0.6 i amplituda 0.3 (podesivo po želji)
        float y = -0.6f + 0.3f * hills;

        // siva boja šina
        vertices.push_back({ x, y, 0.7f, 0.7f, 0.7f });
    }

    const int TRACK_VERTEX_COUNT = NUM_TRACK_POINTS;

    std::vector<float> trackS(TRACK_VERTEX_COUNT);
    trackS[0] = 0.0f;
    for (int i = 1; i < TRACK_VERTEX_COUNT; ++i) {
        float dx = vertices[i].x - vertices[i - 1].x;
        float dy = vertices[i].y - vertices[i - 1].y;
        float dist = std::sqrt(dx * dx + dy * dy);
        trackS[i] = trackS[i - 1] + dist;
    }
    float trackTotalLength = trackS[TRACK_VERTEX_COUNT - 1];

    auto getPointOnTrack = [&](float s, float& outX, float& outY) {
        while (s < 0.0f)        s += trackTotalLength;
        while (s > trackTotalLength) s -= trackTotalLength;

        int i = 0;
        while (i < TRACK_VERTEX_COUNT - 1 && trackS[i + 1] < s) {
            ++i;
        }

        float segLen = trackS[i + 1] - trackS[i];
        float tLocal = (segLen > 0.0f) ? (s - trackS[i]) / segLen : 0.0f;

        float x0 = vertices[i].x;
        float y0 = vertices[i].y;
        float x1 = vertices[i + 1].x;
        float y1 = vertices[i + 1].y;

        outX = x0 + tLocal * (x1 - x0);
        outY = y0 + tLocal * (y1 - y0);
        };

    int WAGON_START_INDEX = TRACK_VERTEX_COUNT;

    // Dodajemo segmente vagona jedan iza drugog
    for (int i = 0; i < WAGON_SEGMENTS; ++i) {
        float x0 = WAGON_X_START + i * (WAGON_SEGMENT_SIZE + WAGON_GAP);     // levo
        float x1 = x0 + WAGON_SEGMENT_SIZE;                     // desno

        //boja vagona
        float r = 0.2f, g = 0.4f, b = 0.9f;

        // zapamtimo centar ovog segmenta
        segmentCenterX[i] = (x0 + x1) / 2.0f;

        // 4 verteksa za jedan kvadrat (TRIANGLE_FAN)
        vertices.push_back({ x0, WAGON_Y_BOTTOM, r, g, b }); // dole levo
        vertices.push_back({ x1, WAGON_Y_BOTTOM, r, g, b }); // dole desno
        vertices.push_back({ x1, WAGON_Y_TOP,    r, g, b }); // gore desno
        vertices.push_back({ x0, WAGON_Y_TOP,    r, g, b }); // gore levo
    }

    const int WAGON_TOTAL_VERTEX_COUNT = WAGON_SEGMENTS * WAGON_VERTEX_COUNT_PER_SEGMENT;

    // Inicijalizacija VAO i VBO, tipičnih struktura za čuvanje podataka o verteksima
    unsigned int VAO;
    unsigned int VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(
        0,
        2,
        GL_FLOAT,
        GL_FALSE,
        sizeof(Vertex),
        (void*)0
    );
    glEnableVertexAttribArray(0);

    // layout(location = 1) → vec3 inCol
    glVertexAttribPointer(
        1,
        3,
        GL_FLOAT,
        GL_FALSE,
        sizeof(Vertex),
        (void*)(2 * sizeof(float))
    );
    glEnableVertexAttribArray(1);


    glClearColor(0.3f, 0.1f, 0.6f, 1.0f); // Postavljanje boje pozadine

    double lastTime = glfwGetTime();

    const float SPEED = 0.5f;   // brzina kretanja

    // Razmak izmedju segmenata po stazi
    const float SEGMENT_SPACING = WAGON_SEGMENT_SIZE + WAGON_GAP;

    // Pocetna pozicija glave voza
    float sHead = (WAGON_SEGMENTS - 1) * SEGMENT_SPACING;

    // Voz na pocetku miruje
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
            sHead += SPEED * (float)deltaTime;

            float maxHead = trackTotalLength;

            if (sHead >= maxHead) {
                sHead = maxHead;
                isRunning = false;
            }
        }


        glClear(GL_COLOR_BUFFER_BIT); // Bojenje pozadine

        glUseProgram(basicShader); // Podešavanje da se crta koristeći dati šejder
        glBindVertexArray(VAO); // Podešavanje da se crta koristeći date vertekse


        // 1)SINE
        glUniform2f(uOffsetLocation, 0.0f, 0.0f);
        glDrawArrays(GL_LINE_STRIP, 0, TRACK_VERTEX_COUNT);

        // 2)VAGON – svaki segment na svojoj tacki putanje
        for (int i = 0; i < WAGON_SEGMENTS; ++i) {

            float sSeg = sHead - i * SEGMENT_SPACING;

            float pathXSeg, pathYSeg;
            getPointOnTrack(sSeg, pathXSeg, pathYSeg);

            float offsetXSeg = pathXSeg - segmentCenterX[i];
            float offsetYSeg = pathYSeg - segmentCenterY;

            glUniform2f(uOffsetLocation, offsetXSeg, offsetYSeg);

            int startIndex = WAGON_START_INDEX + i * WAGON_VERTEX_COUNT_PER_SEGMENT;
            glDrawArrays(GL_TRIANGLE_FAN, startIndex, WAGON_VERTEX_COUNT_PER_SEGMENT);
        }


        glfwSwapBuffers(window); // Zamena bafera - prednji i zadnji bafer se menjaju kao štafeta; dok jedan procesuje, drugi se prikazuje.
        glfwPollEvents(); // Sinhronizacija pristiglih događaja
    }

    glfwTerminate();
    return 0;
}