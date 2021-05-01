#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <learnopengl/filesystem.h>
#include <learnopengl/shader_m.h>
#include <learnopengl/camera.h>
#include <learnopengl/model.h>

#include <iostream>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);
unsigned int loadTexture(const char *path);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// lighting
glm::vec3 lightPos(1.2f, 2.0f, 2.0f);

int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);

    // shaders
    // ------------------------------------
    Shader pyramidShader("resources/shaders/pyramid.vs", "resources/shaders/pyramid.fs");
    Shader lightCubeShader("resources/shaders/light_cube.vs", "resources/shaders/light_cube.fs");


    // pyramid vertices
    float vertices[] = {
            // positions      normals    texture coords
            -0.5,-0.5,-0.5,  0,0.5,-1,      0,0,
            0.5,-0.5,-0.5,   0,0.5,-1,      1,0,
            0,0.5,0,         0,0.5,-1,      0.5,1,

            0.5,-0.5,-0.5,   1,0.5,0,       1,0,
            0.5,-0.5,0.5,    1,0.5,0,       0,0,
            0,0.5,0,         1,0.5,0,       0.5,1,

            0.5,-0.5,0.5,    0,0.5,1,       0,0,
            -0.5,-0.5,0.5,   0,0.5,1,       1,0,
            0,0.5,0,         0,0.5,1,       0.5,1,

            -0.5,-0.5,0.5,   -1,0.5,0,      1,0,
            -0.5,-0.5,-0.5,  -1,0.5,0,      0,0,
            0,0.5,0,         -1,0.5,0,      0.5,1,

            -0.5,-0.5,-0.5,   0,1,0,        0,0,
            0.5,-0.5,-0.5,    0,1,0,        1,0,
            0.5,-0.5,0.5,     0,1,0,        0,0,

            -0.5,-0.5,-0.5,   0,1,0,        0,0,
            0.5,-0.5,0.5,    0,1,0,         0,0,
            -0.5,-0.5,0.5,   0,1,0,         1,0,
    };

    //configure the pyramid's VAO and VBO
    unsigned int pyramidVBO, pyramidVAO;
    glGenVertexArrays(1, &pyramidVAO);
    glGenBuffers(1, &pyramidVBO);

    glBindBuffer(GL_ARRAY_BUFFER, pyramidVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindVertexArray(pyramidVAO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);


    // configure the lightCube's VAO,VBO and EBO
    float lightCube_vertices[] = {
            // front
            -0.5f, -0.5f,  0.5f,
            0.5f, -0.5f,  0.5f,
            0.5f,  0.5f,  0.5f,
            -0.5f,  0.5f,  0.5f,
            // back
            -0.5f, -0.5f, -0.5f,
            0.5f, -0.5f, -0.5f,
            0.5f,  0.5f, -0.5f,
            -0.5f,  0.5f, -0.5f
    };

    int lightCube_indices[] = {
            // front
            0, 1, 2,
            2, 3, 0,
            // right
            1, 5, 6,
            6, 2, 1,
            // back
            7, 6, 5,
            5, 4, 7,
            // left
            4, 0, 3,
            3, 7, 4,
            // bottom
            4, 5, 1,
            1, 0, 4,
            // top
            3, 2, 6,
            6, 7, 3
    };

    unsigned int lightCubeVAO, lightCubeVBO, lightCubeEBO;
    glGenVertexArrays(1, &lightCubeVAO);
    glGenBuffers(1, &lightCubeVBO);
    glGenBuffers(1, &lightCubeEBO);
    glBindVertexArray(lightCubeVAO);

    glBindBuffer(GL_ARRAY_BUFFER, lightCubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(lightCube_vertices), lightCube_vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, lightCubeEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(lightCube_indices), lightCube_indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);

    // plane
    float planeVertices[] = {
            // positions           // normals         // texture Coords
            5.0f, -0.5f,  5.0f,     0.0f, 1.0f, 0.0f,   2.0f, 0.0f,
            -5.0f, -0.5f,  5.0f,    0.0f, 1.0f, 0.0f,   0.0f, 0.0f,
            -5.0f, -0.5f, -5.0f,    0.0f, 1.0f, 0.0f,   0.0f, 2.0f,

            5.0f, -0.5f,  5.0f,     0.0f, 1.0f, 0.0f,   2.0f, 0.0f,
            -5.0f, -0.5f, -5.0f,    0.0f, 1.0f, 0.0f,   0.0f, 2.0f,
            5.0f, -0.5f, -5.0f,     0.0f, 1.0f, 0.0f,   2.0f, 2.0f
    };

    unsigned int planeVAO, planeVBO;
    glGenVertexArrays(1, &planeVAO);
    glGenBuffers(1, &planeVBO);
    glBindVertexArray(planeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), &planeVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glBindVertexArray(0);

    // load textures
    unsigned int floorTexture = loadTexture(FileSystem::getPath("resources/textures/sand.jpg").c_str());
    unsigned int diffuseMap = loadTexture(FileSystem::getPath("resources/textures/brickwall.jpg").c_str());
    unsigned int specularMap = loadTexture(FileSystem::getPath("resources/textures/brickwall.jpg").c_str());

    // shader configuration
    // --------------------
    pyramidShader.use();
    pyramidShader.setInt("material.diffuse", 1);
    pyramidShader.setInt("material.specular", 1);

    Model anubis(FileSystem::getPath("resources/objects/anubis/Anubis_baseMesh.OBJ"));


    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // per-frame time logic
        // --------------------
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        // -----
        processInput(window);

        // render
        // ------
        //glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClearColor(0.75f, 0.52f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


        pyramidShader.use();
        pyramidShader.setVec3("light.position", lightPos);
        pyramidShader.setVec3("viewPos", camera.Position);


        pyramidShader.setVec3("dirLight.direction", -0.2f, 2.0f, -0.3f);
        pyramidShader.setVec3("dirLight.ambient", 0.3f, 0.24f, 0.14f);
        pyramidShader.setVec3("dirLight.diffuse", 0.7f, 0.42f, 0.26f);
        pyramidShader.setVec3("dirLight.specular", 0.5f, 0.5f, 0.5f);
        // point light 1
        pyramidShader.setVec3("pointLight.position", lightPos);
        pyramidShader.setVec3("pointLight.ambient", 1.0 * 0.1,  0.6 * 0.1,  0.0* 0.1);
        pyramidShader.setVec3("pointLight.diffuse", glm::vec3(1.0f, 0.6f, 0.0f));
        pyramidShader.setVec3("pointLight.specular",  glm::vec3(1.0f, 0.6f, 0.0f));
        pyramidShader.setFloat("pointLight.constant", 1.0f);
        pyramidShader.setFloat("pointLight.linear", 0.09);
        pyramidShader.setFloat("pointLight.quadratic", 0.032);


        // light properties
        pyramidShader.setVec3("light.ambient", 0.2f, 0.2f, 0.2f);
        pyramidShader.setVec3("light.diffuse", 0.5f, 0.5f, 0.5f);
        pyramidShader.setVec3("light.specular", 1.0f, 0.85f, 0.0f);

        // material properties
        pyramidShader.setFloat("material.shininess", 64.0f);


        lightPos.x = 3.0f * cos(glfwGetTime());
        lightPos.z = 3.0f * sin(glfwGetTime());

        // view/projection transformations
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        pyramidShader.setMat4("projection", projection);
        pyramidShader.setMat4("view", view);

        // world transformation
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::scale(model, glm::vec3(2.0f));
        model = glm::translate(model,glm::vec3(0.0f,0.25f,0.0f));
        pyramidShader.setMat4("model", model);

        // bind diffuse map
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, diffuseMap);
        // bind specular map
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, specularMap);
        // render the pyramid
        glBindVertexArray(pyramidVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);


        // draw the lightCube
        lightCubeShader.use();
        lightCubeShader.setMat4("projection", projection);
        lightCubeShader.setMat4("view", view);
        model = glm::mat4(1.0f);
        model = glm::translate(model, lightPos);
        model = glm::scale(model, glm::vec3(0.2f)); // a smaller cube
        lightCubeShader.setMat4("model", model);
        glBindVertexArray(lightCubeVAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);


        // draw plane
        pyramidShader.use();
        glBindVertexArray(planeVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, floorTexture);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, floorTexture);
        model = glm::mat4(1.0f);
        pyramidShader.setMat4("model", model);
        pyramidShader.setMat4("view",view);
        pyramidShader.setMat4("projection",projection);
        glDrawArrays(GL_TRIANGLES, 0, 6);


        //anubis

        pyramidShader.setVec3("pointLight.ambient", 1.0 * 0.1,  0.6 * 0.1,  0.0* 0.1);
        pyramidShader.setVec3("pointLight.diffuse", glm::vec3(1.0f, 0.0f, 0.0f));
        pyramidShader.setVec3("pointLight.specular",  glm::vec3(1.0f, 0.0f, 1.0f));

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(4.0f, 2.25f, -4.0f));
        model = glm::scale(model, glm::vec3(0.3));
        model = glm::rotate(model,glm::radians(-45.0f),glm::vec3(0.0f,1.0f,0.0f));
        pyramidShader.setMat4("model", model);
        anubis.Draw(pyramidShader);


        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDeleteVertexArrays(1, &pyramidVAO);
    glDeleteVertexArrays(1, &lightCubeVAO);
    glDeleteVertexArrays(1, &planeVAO);
    glDeleteBuffers(1, &planeVBO);
    glDeleteBuffers(1, &pyramidVBO);
    glDeleteBuffers(1, &lightCubeVBO);
    glDeleteBuffers(1, &lightCubeEBO);

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(yoffset);
}

// utility function for loading a 2D texture from file
// ---------------------------------------------------
unsigned int loadTexture(char const * path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}