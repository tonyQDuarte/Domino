/* app_run liga as animações: a distribuição está em andamento no primeiro quadro. */
#include "raylib.h"

#include "app.h"
#include "harness.h"

static bool seen, animate, dealing;
static int flying;

static void state(const Controller *c, void *user)
{
    (void)user;
    View v;
    ctl_view(c, &v);
    seen = true;
    animate = c->animate;
    dealing = c->anim == ANIM_DEAL;
    flying = v.flying_count;
}

static int app_animations_on(void)
{
    AppOptions opt = {0};
    opt.seed = 1;
    opt.max_frames = 2;
    opt.on_state = state;
    CHECK(app_run(&opt) == 0);
    CHECK(seen);
    CHECK(animate);
    CHECK(dealing);
    CHECK(flying >= 1);
    return 0;
}

int main(int argc, char **argv)
{
    static const TestCase cases[] = {
        {"app_animations_on", app_animations_on},
    };
    SetTraceLogLevel(LOG_WARNING);
    return RUN_TESTS(cases);
}
