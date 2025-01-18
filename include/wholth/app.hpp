#ifndef WHOLTH_APP_H_
#define WHOLTH_APP_H_

namespace wholth::app
{
    struct InitArgs
    {
        float window_height;
        float window_width;
    };

    void init(const InitArgs);
}

# endif // WHOLTH_APP_H_
