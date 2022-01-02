#ifndef __SNAKE_H__
#define __SNAKE_H__

#include <vector>
#include <cstdlib>
#include <ctime>

#include "winsys.h"
#include "cpoint.h"
#include "screen.h"

class CSnake : public CFramedWindow
{
  enum state_t
  {
    PAUSED,
    RUNNING,
    LOOSE,
    WIN
  };

  struct Part : public CPoint
  {
    char c;
    Part(int _x, int _y, char _c) : CPoint(_x, _y), c(_c) {}
  };

  int tick_n = 0;
  int speed = 40;
  state_t state = PAUSED;
  bool help = true;
  int score = 0;
  CPoint velocity = CPoint(1, 0);
  vector<Part> parts;
  Part food = Part(1, 1, 'O');
  int direction = KEY_RIGHT;

  void reset()
  {
    tick_n = 0;
    speed = 40;
    score = 0;
    parts.clear();
    int x = 5, y = 2;
    parts.push_back(Part(x, y, '*'));
    parts.push_back(Part(x - 1, y, '+'));
    parts.push_back(Part(x - 2, y, '+'));
    velocity = CPoint(1, 0);
    direction = KEY_RIGHT;
    generateFood();
    state = RUNNING;
    help = false;
  }

  void drawPaused()
  {
    int x = geom.topleft.x + geom.size.x / 2 - 3, y = geom.topleft.y + geom.size.y / 2;
    gotoyx(y, x);
    printl("PAUSED");
  }

  void drawHelp()
  {
    int x = geom.topleft.x, y = geom.topleft.y;
    gotoyx(y + 2, x + 3);
    printl("h - toggle help information");
    gotoyx(y + 3, x + 3);
    printl("p - toggle play/pause mode");
    gotoyx(y + 4, x + 3);
    printl("r - restart game");
    gotoyx(y + 5, x + 3);
    printl("q - quit");
    gotoyx(y + 6, x + 3);
    printl("arrows - move snake (in play mode) or");
    gotoyx(y + 7, x + 12);
    printl("move window (in pause mode)");
  }

  void draw()
  {
    for (int y = 1; y < geom.size.y - 1; y++)
    {
      for (int x = 1; x < geom.size.x - 1; x++)
      {
        gotoyx(geom.topleft.y + y, geom.topleft.x + x);
        printc(' ');
      }
    }

    for (int i = parts.size() - 1; i >= 0; i--)
    {
      gotoyx(geom.topleft.y + parts[i].y, geom.topleft.x + parts[i].x);
      printc(parts[i].c);
    }

    gotoyx(geom.topleft.y + food.y, geom.topleft.x + food.x);
    printc(food.c);
  }

  void togglePaused()
  {
    if (state == PAUSED)
      state = RUNNING;
    else
      state = PAUSED;
  }

  void drawLoose()
  {
    int x = geom.topleft.x + geom.size.x / 2 - 4;
    int y = geom.topleft.y + geom.size.y / 2;
    gotoyx(y, x);
    printl("YOU LOOSE");
  }

  void drawWin()
  {
    int x = geom.topleft.x + geom.size.x / 2 - 3;
    int y = geom.topleft.y + geom.size.y / 2;
    gotoyx(y, x);
    printl("YOU WIN");
  }

  void tick()
  {
    tick_n = (tick_n + 1) % speed;
    if (tick_n == 0)
      move();
  }

  void move()
  {
    for (int i = parts.size() - 1; i > 0; i--)
    {
      parts[i].x = parts[i - 1].x;
      parts[i].y = parts[i - 1].y;
    }

    parts[0] += velocity;

    checkHead();

    if (parts[0].x == food.x && parts[0].y == food.y)
    {
      score += 1;

      if (score == (geom.size.x - 2) * (geom.size.y - 2))
      {
        state = WIN;
        return;
      }

      if (speed > 5)
        speed -= 5;

      parts.push_back(Part(parts[0].x, parts[0].y, '+'));
      move();

      generateFood();
    }

    for (size_t i = 1; i < parts.size(); i++)
    {
      if (parts[0].x == parts[i].x && parts[0].y == parts[i].y)
      {
        state = LOOSE;
        break;
      }
    }
  }

  void checkHead()
  {
    if (parts[0].x == geom.size.x - 1)
      parts[0].x = 1;
    if (parts[0].x == 0)
      parts[0].x = geom.size.x - 2;

    if (parts[0].y == geom.size.y - 1)
      parts[0].y = 1;
    if (parts[0].y == 0)
      parts[0].y = geom.size.y - 2;
  }

  void generateFood()
  {
    bool ok = true;
    do
    {
      food.x = rand() % (geom.size.x - 2) + 1;
      food.y = rand() % (geom.size.y - 2) + 1;

      ok = true;
      for (auto &part : parts)
      {
        if (part.x == food.x && part.y == food.y)
        {
          ok = false;
          break;
        }
      }
    } while (!ok);
  }

public:
  CSnake(CRect r) : CFramedWindow(r, ' ')
  {
    int x = 5, y = 2;
    parts.push_back(Part(x, y, '*'));
    parts.push_back(Part(x - 1, y, '+'));
    parts.push_back(Part(x - 2, y, '+'));
  }

  bool handleEvent(int key)
  {
    if (tolower(key) == 'p' && state != LOOSE && state != WIN && !help)
    {
      togglePaused();
      return true;
    }

    if (tolower(key) == 'h')
    {
      help = !help;
      if (help && state != LOOSE && state != WIN)
        state = PAUSED;
      return true;
    }

    if (tolower(key) == 'r')
    {
      reset();
      return true;
    }

    if (state == RUNNING)
    {
      if (key == direction)
      {
        move();
        return true;
      }

      if (key == KEY_UP && direction != KEY_DOWN)
      {
        direction = KEY_UP;
        velocity = CPoint(0, -1);
        move();
      }

      else if (key == KEY_DOWN && direction != KEY_UP)
      {
        direction = KEY_DOWN;
        velocity = CPoint(0, 1);
        move();
      }

      else if (key == KEY_RIGHT && direction != KEY_LEFT)
      {
        direction = KEY_RIGHT;
        velocity = CPoint(1, 0);
        move();
      }

      else if (key == KEY_LEFT && direction != KEY_RIGHT)
      {
        direction = KEY_LEFT;
        velocity = CPoint(-1, 0);
        move();
      }

      else
        tick();

      return true;
    }

    return CFramedWindow::handleEvent(key);
  }

  void paint()
  {
    CFramedWindow::paint();

    gotoyx(geom.topleft.y - 1, geom.topleft.x);
    printl("| SCORE: %d |", score);

    if (help)
      drawHelp();
    else
    {
      draw();
      if (state == PAUSED)
        drawPaused();
      if (state == LOOSE)
        drawLoose();
      if (state == WIN)
        drawWin();
    }
  }
};

#endif
