#include "scenary.hpp"

#include <glm/gtx/fast_trigonometry.hpp>
#include <glm/gtx/rotate_vector.hpp>

void Scenary::initializeGL(GLuint program) {
  terminateGL();

  m_program = program;
  m_colorLoc = abcg::glGetUniformLocation(m_program, "color");
  m_rotationLoc = abcg::glGetUniformLocation(m_program, "rotation");
  m_scaleLoc = abcg::glGetUniformLocation(m_program, "scale");
  m_translationLoc = abcg::glGetUniformLocation(m_program, "translation");

  m_rotation = 0.0f;
  m_translation = glm::vec2{0, 0};
  m_velocity = glm::vec2(0);

  // clang-format off
  std::array<glm::vec2, 24> positions{
      // Baixo
      glm::vec2{-15.5f, -15.5f}, // Inferior Esquerdo 0
      glm::vec2{+15.5f, -15.5f}, // Inferior Direito 1
      glm::vec2{+15.5f, -14.5f}, // Superior Direito 2 
      glm::vec2{-15.5f, -14.5f}, // Superior Esquerdo 3
      // Cima
      glm::vec2{-15.5f, +15.5f}, // Superior Esquerdo 0
      glm::vec2{+15.5f, +15.5f}, // Superior Direito 1
      glm::vec2{+15.5f, +14.5f}, // Inferior Direito 2 
      glm::vec2{-15.5f, +14.5f}, // Inferior Esquerdo 3
      /*
      glm::vec2{-03.5f, -12.5f}, glm::vec2{+03.5f, -12.5f},
      glm::vec2{+09.5f, -07.5f}, glm::vec2{+15.5f, -12.5f},
      glm::vec2{+15.5f, +02.5f}, glm::vec2{+02.5f, +12.5f},

      
      // Cannon left
      glm::vec2{-12.5f, +10.5f}, glm::vec2{-12.5f, +04.0f},
      glm::vec2{-09.5f, +04.0f}, glm::vec2{-09.5f, +10.5f},

      // Cannon right
      glm::vec2{+09.5f, +10.5f}, glm::vec2{+09.5f, +04.0f},
      glm::vec2{+12.5f, +04.0f}, glm::vec2{+12.5f, +10.5f},
      
      // Thruster trail (left)
      glm::vec2{-12.0f, -07.5f}, 
      glm::vec2{-09.5f, -18.0f}, 
      glm::vec2{-07.0f, -07.5f},

      // Thruster trail (right)
      glm::vec2{+07.0f, -07.5f}, 
      glm::vec2{+09.5f, -18.0f}, 
      glm::vec2{+12.0f, -07.5f},
      */
      };
      

  // Normalize
  for (auto &position : positions) {
    position /= glm::vec2{15.5f, 15.5f};
  }
/* vertices da barra
3-----2
|     |
0-----1 

7-----6
|     |
4-----5 
*/
  const std::array indices{0, 1, 3,
                           1, 2, 3,
                           4, 5, 6,
                           4, 6, 7,

                           /*
                           0, 3, 4,
                           0, 4, 5,
                           9, 0, 5,
                           9, 5, 6,
                           9, 6, 8,
                           8, 6, 7,
                           */
                           /*
                           // Cannons
                           10, 11, 12,
                           10, 12, 13,
                           14, 15, 16,
                           14, 16, 17,
                           // Thruster trails
                           18, 19, 20,
                           21, 22, 23
                           */
                           };
  // clang-format on

  // Generate VBO
  abcg::glGenBuffers(1, &m_vbo);
  abcg::glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
  abcg::glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions.data(),
                     GL_STATIC_DRAW);
  abcg::glBindBuffer(GL_ARRAY_BUFFER, 0);

  // Generate EBO
  abcg::glGenBuffers(1, &m_ebo);
  abcg::glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
  abcg::glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices.data(),
                     GL_STATIC_DRAW);
  abcg::glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

  // Get location of attributes in the program
  GLint positionAttribute{abcg::glGetAttribLocation(m_program, "inPosition")};

  // Create VAO
  abcg::glGenVertexArrays(1, &m_vao);

  // Bind vertex attributes to current VAO
  abcg::glBindVertexArray(m_vao);

  abcg::glEnableVertexAttribArray(positionAttribute);
  abcg::glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
  abcg::glVertexAttribPointer(positionAttribute, 2, GL_FLOAT, GL_FALSE, 0,
                              nullptr);
  abcg::glBindBuffer(GL_ARRAY_BUFFER, 0);

  abcg::glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);

  // End of binding to current VAO
  abcg::glBindVertexArray(0);
}

void Scenary::paintGL(const GameData &gameData) {
  if (gameData.m_state != State::Playing) return;

  abcg::glUseProgram(m_program);

  abcg::glBindVertexArray(m_vao);

  abcg::glUniform1f(m_scaleLoc, m_scale);
  abcg::glUniform1f(m_rotationLoc, m_rotation);
  abcg::glUniform2fv(m_translationLoc, 1, &m_translation.x);
  
  
  // Restart thruster blink timer every 100 ms
  /*
  if (m_trailBlinkTimer.elapsed() > 100.0 / 1000.0) m_trailBlinkTimer.restart();

  if (gameData.m_input[static_cast<size_t>(Input::Up)]) {
    // Show thruster trail during 50 ms
    if (m_trailBlinkTimer.elapsed() < 50.0 / 1000.0) {
      abcg::glEnable(GL_BLEND);
      abcg::glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

      // 50% transparent
      abcg::glUniform4f(m_colorLoc, 1, 1, 1, 0.5f);

      abcg::glDrawElements(GL_TRIANGLES, 14 * 3, GL_UNSIGNED_INT, nullptr);

      abcg::glDisable(GL_BLEND);
    }
  }
  */

  abcg::glUniform4fv(m_colorLoc, 1, &m_color.r);
  abcg::glDrawElements(GL_TRIANGLES, 4 * 3, GL_UNSIGNED_INT, nullptr);

  abcg::glBindVertexArray(0);

  abcg::glUseProgram(0);
}

void Scenary::terminateGL() {
  abcg::glDeleteBuffers(1, &m_vbo);
  abcg::glDeleteBuffers(1, &m_ebo);
  abcg::glDeleteVertexArrays(1, &m_vao);
}


void Scenary::update(const GameData &gameData, float deltaTime) {
  // gambiarra para build
  if (gameData.m_state == State::Playing){
  m_velocity += 0; // glm::vec2{0.0f, 300.0f} * deltaTime;
  m_translation += 0;}
  /*
  if (gameData.m_inputLeft[static_cast<size_t>(Input::Up)] && gameData.m_state == State::Playing) {
    m_translation += m_velocity * deltaTime;

  }else if (gameData.m_inputLeft[static_cast<size_t>(Input::Down)] && gameData.m_state == State::Playing) {
    m_translation -= m_velocity * deltaTime;
  }
  */
}

