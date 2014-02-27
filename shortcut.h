#ifndef __FROGLIES_SHORTCUT_H__
#define __FROGLIES_SHORTCUT_H__
namespace FrogLies {
    class Shortcut;
}
#include<vector>
#include<string>


#include "debug.h"


namespace FrogLies {
    class Shortcut {
        std::vector<std::string> keys;
        static std::vector<Shortcut*> Shortcuts;
    public:
        Shortcut( std::string val );
        void Set( std::string val );
        int IsHit();
        int IsValid( );
    };
    extern Shortcut ShortcutWin, ShortcutDesk, ShortcutCrop, ShortcutClip, ShortcutQuit, ShortcutStop;
}

#endif
