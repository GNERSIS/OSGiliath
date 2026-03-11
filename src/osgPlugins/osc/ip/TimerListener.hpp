/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * TimerListener class.
 * Provides: TimerExpired.
 */
#pragma once

class TimerListener
{
    public:

        virtual ~TimerListener()
        {
        }

        virtual void
        TimerExpired() = 0;
};
