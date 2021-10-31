#include "openglwindow.hpp"

#include <imgui.h>

#include "abcg.hpp"

void OpenGLWindow::handleEvent(SDL_Event &event) {
  // Keyboard events LeftBar
  if (event.type == SDL_KEYDOWN) {
    if (event.key.keysym.sym == SDLK_w)
      m_gameData.m_inputLeft.set(static_cast<size_t>(Input::Up));
    if (event.key.keysym.sym == SDLK_s)
      m_gameData.m_inputLeft.set(static_cast<size_t>(Input::Down));
  }
  if (event.type == SDL_KEYUP) {
    if (event.key.keysym.sym == SDLK_w)
      m_gameData.m_inputLeft.reset(static_cast<size_t>(Input::Up));
    if (event.key.keysym.sym == SDLK_s)
      m_gameData.m_inputLeft.reset(static_cast<size_t>(Input::Down));
  }
  
  // Keyboard events RightBar
  if (event.type == SDL_KEYDOWN) {
    if (event.key.keysym.sym == SDLK_UP)
      m_gameData.m_inputRight.set(static_cast<size_t>(Input::Up));
    if (event.key.keysym.sym == SDLK_DOWN)
      m_gameData.m_inputRight.set(static_cast<size_t>(Input::Down));

  }
  if (event.type == SDL_KEYUP) {
    if (event.key.keysym.sym == SDLK_UP)
      m_gameData.m_inputRight.reset(static_cast<size_t>(Input::Up));
    if (event.key.keysym.sym == SDLK_DOWN)
      m_gameData.m_inputRight.reset(static_cast<size_t>(Input::Down));
  }
  
}

void OpenGLWindow::initializeGL() {
  // Load a new font
  ImGuiIO &io{ImGui::GetIO()};
  const auto filename{getAssetsPath() + "Inconsolata-Medium.ttf"};
  m_font = io.Fonts->AddFontFromFileTTF(filename.c_str(), 30.0f);
  if (m_font == nullptr) {
    throw abcg::Exception{abcg::Exception::Runtime("Cannot load font file")};
  }

  
  // Create program to render the stars
  /*
  m_starsProgram = createProgramFromFile(getAssetsPath() + "stars.vert",
                                         getAssetsPath() + "stars.frag"); */

  // Create program to render the other objects
  m_objectsProgram = createProgramFromFile(getAssetsPath() + "objects.vert",
                                           getAssetsPath() + "objects.frag");

  abcg::glClearColor(0, 0, 0, 1);

#if !defined(__EMSCRIPTEN__)
  abcg::glEnable(GL_PROGRAM_POINT_SIZE);
#endif

  // Start pseudo-random number generator
  m_randomEngine.seed(
      std::chrono::steady_clock::now().time_since_epoch().count());

  restart();
}

void OpenGLWindow::restart() {
  m_gameData.m_state = State::Playing;

  m_barLeft.initializeGL(m_objectsProgram);
  m_barRight.initializeGL(m_objectsProgram);
  m_ball.initializeGL(m_objectsProgram);
  m_scenary.initializeGL(m_objectsProgram);
  // m_starLayers.initializeGL(m_starsProgram, 25);
  // m_ship.initializeGL(m_objectsProgram);
  // m_asteroids.initializeGL(m_objectsProgram, 3);
  // m_bullets.initializeGL(m_objectsProgram);
}

void OpenGLWindow::update() {
  const float deltaTime{static_cast<float>(getDeltaTime())};

  // Wait 5 seconds before restarting
  if (m_gameData.m_state != State::Playing &&
      m_restartWaitTimer.elapsed() > 5) {
    restart();
    return;
  }
  m_barRight.update(m_gameData, deltaTime);
  m_barLeft.update(m_gameData, deltaTime);
  m_ball.update(m_barLeft, m_barRight, m_gameData, deltaTime);
  m_scenary.update(m_gameData);
  // m_ship.update(m_gameData, deltaTime);
  // m_starLayers.update(m_ship, deltaTime);
  // m_asteroids.update(m_ship, deltaTime);
  // m_bullets.update(m_ship, m_gameData, deltaTime);


  if (m_gameData.m_state == State::Playing) {
    checkCollisions();
    checkWinCondition();
  }

}

void OpenGLWindow::paintGL() {
  update();

  abcg::glClear(GL_COLOR_BUFFER_BIT);
  abcg::glViewport(0, 0, m_viewportWidth, m_viewportHeight);

  /*
  m_starLayers.paintGL();
  m_asteroids.paintGL();
  m_bullets.paintGL();
  m_ship.paintGL(m_gameData);
  */
  m_barLeft.paintGL(m_gameData);
  m_barRight.paintGL(m_gameData);
  m_scenary.paintGL(m_gameData);
  m_ball.paintGL();
}

void OpenGLWindow::paintUI() {
  abcg::OpenGLWindow::paintUI();

  {
    const auto size{ImVec2(300, 85)};
    const auto position{ImVec2((m_viewportWidth - size.x) / 2.0f,
                               (m_viewportHeight - size.y) / 2.0f)};
    ImGui::SetNextWindowPos(position);
    ImGui::SetNextWindowSize(size);
    ImGuiWindowFlags flags{ImGuiWindowFlags_NoBackground |
                           ImGuiWindowFlags_NoTitleBar |
                           ImGuiWindowFlags_NoInputs};
    ImGui::Begin(" ", nullptr, flags);
    ImGui::PushFont(m_font);

    if (m_gameData.m_state == State::WinPlayer1) {
      ImGui::Text("*Player 1 Win!*");
    } else if (m_gameData.m_state == State::WinPlayer2) {
      ImGui::Text("*Player 2 Win!*");
    }

    ImGui::PopFont();
    ImGui::End();
  }
}

void OpenGLWindow::resizeGL(int width, int height) {
  m_viewportWidth = width;
  m_viewportHeight = height;

  abcg::glClear(GL_COLOR_BUFFER_BIT);
}

void OpenGLWindow::terminateGL() {
  // abcg::glDeleteProgram(m_starsProgram);
  abcg::glDeleteProgram(m_objectsProgram);

  /*
  m_asteroids.terminateGL();
  m_bullets.terminateGL();
  m_ship.terminateGL();
  m_starLayers.terminateGL();
  */
  m_barRight.terminateGL();
  m_barLeft.terminateGL();
  m_ball.terminateGL();
  m_scenary.terminateGL();
}


void OpenGLWindow::checkCollisions() {
  // Check collision between bar and ball
  const auto ballTranslation{m_ball.m_translation * glm::vec2{15.5f, 15.5f}};
  
  /*
  const auto distance{glm::distance(glm::vec2{-1, m_barLeft.m_translation.y}, ballTranslation)};

  if (distance < m_barLeft.m_scale * 0.07f + m_ball.m_scale) {
    m_ball.direction = !m_ball.direction;
    // m_gameData.m_state = State::GameOver;
    // m_restartWaitTimer.restart();
  }
  */

  if(ballTranslation.x <= -14.5f && (ballTranslation.y >= ((m_barLeft.m_translation.y * 15.5f) - 3.5f) && ballTranslation.y <= ((m_barLeft.m_translation.y * 15.5f) + 3.5f))){
    m_ball.direction = !m_ball.direction;
    m_ball.m_velocity = glm::vec2{1.0f, ((ballTranslation.y / 15.5f) - m_barLeft.m_translation.y)} * m_ball.m_ballSpeed;
  }

  if(ballTranslation.x >= +14.5f && (ballTranslation.y >= ((m_barRight.m_translation.y * 15.5f) - 3.5f) && ballTranslation.y <= ((m_barRight.m_translation.y * 15.5f) + 3.5f))){
    m_ball.direction = !m_ball.direction;
    m_ball.m_velocity = glm::vec2{1.0f, (m_barRight.m_translation.y - (ballTranslation.y / 15.5f))} * m_ball.m_ballSpeed;
  }

  // Check collision between ball and scenary
  if(ballTranslation.y <= -14.5f && (ballTranslation.x >= ((m_scenary.m_translation.x * 15.5f) - 3.5f) && ballTranslation.x <= ((m_scenary.m_translation.x * 15.5f) + 3.5f))){
    m_ball.direction = !m_ball.direction;
    m_ball.m_velocity = glm::vec2{1.0f, ((ballTranslation.x / 15.5f) - m_scenary.m_translation.x)} * m_ball.m_ballSpeed;
  }

  if(ballTranslation.y >= +14.5f && (ballTranslation.x >= ((m_scenary.m_translation.y * 15.5f) - 3.5f) && ballTranslation.x <= ((m_scenary.m_translation.x * 15.5f) + 3.5f))){
    m_ball.direction = !m_ball.direction;
    m_ball.m_velocity = glm::vec2{1.0f, (m_scenary.m_translation.x - (ballTranslation.x / 15.5f))} * m_ball.m_ballSpeed;
  }
  /*
  // Check collision between ship and asteroids
  for (const auto &asteroid : m_asteroids.m_asteroids) {
    const auto asteroidTranslation{asteroid.m_translation};
    const auto distance{
        glm::distance(m_ship.m_translation, asteroidTranslation)};

    if (distance < m_ship.m_scale * 0.9f + asteroid.m_scale * 0.85f) {
      m_gameData.m_state = State::GameOver;
      m_restartWaitTimer.restart();
    }
  }

  // Check collision between bullets and asteroids
  for (auto &bullet : m_bullets.m_bullets) {
    if (bullet.m_dead) continue;

    for (auto &asteroid : m_asteroids.m_asteroids) {
      for (const auto i : {-2, 0, 2}) {
        for (const auto j : {-2, 0, 2}) {
          const auto asteroidTranslation{asteroid.m_translation +
                                         glm::vec2(i, j)};
          const auto distance{
              glm::distance(bullet.m_translation, asteroidTranslation)};

          if (distance < m_bullets.m_scale + asteroid.m_scale * 0.85f) {
            asteroid.m_hit = true;
            bullet.m_dead = true;
          }
        }
      }
    }

    // Break asteroids marked as hit
    for (const auto &asteroid : m_asteroids.m_asteroids) {
      if (asteroid.m_hit && asteroid.m_scale > 0.10f) {
        std::uniform_real_distribution<float> m_randomDist{-1.0f, 1.0f};
        std::generate_n(std::back_inserter(m_asteroids.m_asteroids), 3, [&]() {
          const glm::vec2 offset{m_randomDist(m_randomEngine),
                                 m_randomDist(m_randomEngine)};
          return m_asteroids.createAsteroid(
              asteroid.m_translation + offset * asteroid.m_scale * 0.5f,
              asteroid.m_scale * 0.5f);
        });
      }
    }

    m_asteroids.m_asteroids.remove_if(
        [](const Asteroids::Asteroid &a) { return a.m_hit; });
  }
  */
}


void OpenGLWindow::checkWinCondition() {
  const auto ballTranslation{m_ball.m_translation * glm::vec2{15.5f, 15.5f}};
  
    if (ballTranslation.x > +16.5f) {
      m_gameData.m_state = State::WinPlayer1;
      m_restartWaitTimer.restart();
    }
    if (ballTranslation.x < -16.5f) {
      m_gameData.m_state = State::WinPlayer2;
      m_restartWaitTimer.restart();
    }

}

/*
void OpenGLWindow::transformPosition(auto position) {

  position /= 15.5f;

  return position;
}*/