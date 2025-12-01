#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include "Util.h"

#include <vector>

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

    std::vector<Vertex> vertices;

    // Kriva putanja pomocu sinusa
    const int NUM_TRACK_POINTS = 200; //vise tacaka da putanja bude glatka
    const float NUM_HILLS = 5.0f;

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
    int WAGON_START_INDEX = TRACK_VERTEX_COUNT;

    // VAGON
    vertices.push_back({ -0.4f, -0.9f, 0.2f, 0.4f, 0.9f }); // dole levo
    vertices.push_back({ 0.0f, -0.9f, 0.2f, 0.4f, 0.9f }); // dole desno
    vertices.push_back({ 0.0f, -0.75f, 0.2f, 0.4f, 0.9f }); // gore desno
    vertices.push_back({ -0.4f, -0.75f, 0.2f, 0.4f, 0.9f }); // gore levo

    const int WAGON_VERTEX_COUNT = 4;

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

    while (!glfwWindowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT); // Bojenje pozadine

        glUseProgram(basicShader); // Podešavanje da se crta koristeći dati šejder
        glBindVertexArray(VAO); // Podešavanje da se crta koristeći date vertekse

        // SINE
        glDrawArrays(GL_LINE_STRIP, 0, TRACK_VERTEX_COUNT);

        // VAGON
        //glDrawArrays(GL_TRIANGLE_FAN, WAGON_START_INDEX, WAGON_VERTEX_COUNT);


        glfwSwapBuffers(window); // Zamena bafera - prednji i zadnji bafer se menjaju kao štafeta; dok jedan procesuje, drugi se prikazuje.
        glfwPollEvents(); // Sinhronizacija pristiglih događaja
    }

    glfwTerminate();
    return 0;
}