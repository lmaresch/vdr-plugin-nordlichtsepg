/*
 * See the README file for copyright information and how to reach the author.
 */

#include <string>
#include <locale.h>
#include <langinfo.h>
#include <math.h>
#include <vdr/menu.h>
#include <vdr/status.h>
#include "mywhatsonitem.h"

// --- Icons ------------------------------------------------------------------
bool Icons::IsUTF8 = false;

void Icons::InitCharSet()
{
    // Taken from VDR's vdr.c
    char *CodeSet = NULL;
    if (setlocale(LC_CTYPE, ""))
        CodeSet = nl_langinfo(CODESET);
    else
    {
        char *LangEnv = getenv("LANG"); // last resort in case locale stuff isn't installed
        if (LangEnv)
        {
            CodeSet = strchr(LangEnv, '.');
            if (CodeSet)
                CodeSet++; // skip the dot
        }
    }

    if (CodeSet && strcasestr(CodeSet, "UTF-8") != 0)
        IsUTF8 = true;
}

// --- myWhatsOnItem ----------------------------------------------------------
myWhatsOnItem::myWhatsOnItem(const cEvent *Event, const cChannel *Channel, bool Next)
{
    event = Event;
    channel = Channel;
    next = Next;
    timer = NULL;

    Set();
}

void myWhatsOnItem::Set()
{
    int i;
    char *buffer = NULL;
    const char *m = " ";

    // look for timers
    LOCK_TIMERS_READ;
    for (const cTimer *ti = Timers->First(); ti; ti = Timers->Next(ti))
    {
        if (ti->Matches(t) && (ti->Channel() == channel))
        {
            timer = ti;
            m = timer->Recording() ? Icons::Recording() : Icons::AlarmClock();
        }
    }

    if (event)
    {
        // calculate progress bar
        int progress = (int)roundf((float)(time(NULL) - event->StartTime()) / (float)(event->Duration()) * 10.0);

        if (progress < 0)
            progress = 0;
        else if (progress > 9)
            progress = 9;

        std::string ProgressBar;
        ProgressBar += Icons::ProgressStart();
        for (i = 0; i < 10; i++)
        {
            if (i < progress)
                ProgressBar += Icons::ProgressFilled();
            else
                ProgressBar += Icons::ProgressEmpty();
        }
        ProgressBar += Icons::ProgressEnd();

        if (showchannelnumbers)
            asprintf(&buffer, "%s\t%d\t%-10s\t %s\t%s", m, channel->Number(), channel->ShortName(true), (!(event->RunningStatus() == 4) && next) ? *event->GetTimeString() : ProgressBar.c_str(), event->Title());
        else
            asprintf(&buffer, "%s\t%-10s\t %s\t%s", m, channel->ShortName(true), (!(event->RunningStatus() == 4) && next) ? *event->GetTimeString() : ProgressBar.c_str(), event->Title());
    }
    else
    {
        if (showchannelnumbers)
            asprintf(&buffer, "%s\t%d\t%-10s\t \t(%s)", m, channel->Number(), channel->ShortName(true), tr("no info"));
        else
            asprintf(&buffer, "%s\t%-10s\t \t(%s)", m, channel->ShortName(true), tr("no info"));
    }

    SetText(buffer, false);
}
