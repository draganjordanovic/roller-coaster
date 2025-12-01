#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include "Util.h"

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
    GLFWwindow* window = glfwCreateWindow(800, 800, "Vezba 1", NULL, NULL);
    if (window == NULL) return endProgram("Prozor nije uspeo da se kreira.");
    glfwMakeContextCurrent(window);

    // Inicijalizacija GLEW
    if (glewInit() != GLEW_OK) return endProgram("GLEW nije uspeo da se inicijalizuje.");

    unsigned int basicShader = createShader("basic.vert", "basic.frag");

    float vertices[] = {
         0.0
    };

    // Inicijalizacija VAO i VBO, tipičnih struktura za čuvanje podataka o verteksima
    unsigned int VAO;
    unsigned int VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);


    glClearColor(0.3f, 0.1f, 0.6f, 1.0f); // Postavljanje boje pozadine

    while (!glfwWindowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT); // Bojenje pozadine

        glUseProgram(basicShader); // Podešavanje da se crta koristeći dati šejder
        glBindVertexArray(VAO); // Podešavanje da se crta koristeći date vertekse


        glfwSwapBuffers(window); // Zamena bafera - prednji i zadnji bafer se menjaju kao štafeta; dok jedan procesuje, drugi se prikazuje.
        glfwPollEvents(); // Sinhronizacija pristiglih događaja
    }

    glfwTerminate();
    return 0;
}