#pragma once

#include <src/cli/manager.h>

class ScreenController {
  private:
       virtual void m_init() {}
       virtual void m_cleanup() {}
       virtual void m_draw() = 0;
       virtual void m_on_resize() {
               this->cleanup();
               this->init();
               this->draw();
       }

       /* TODO: return whether input was processed and should not propagate until top level controller
        */
       virtual void m_on_key(int key) = 0;

  protected:
       WindowManager *wmanager;

  public:
       ScreenController(WindowManager *wmanager) : wmanager(wmanager) {}
       virtual ~ScreenController() {}

       void init() { this->m_init(); }
       void cleanup() { this->m_cleanup(); }
       void draw() { this->m_draw(); }

       void on_resize() { this->m_on_resize(); }

       void on_key(int key) { this->m_on_key(key); }
};

