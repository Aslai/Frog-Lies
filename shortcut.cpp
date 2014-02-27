#include "shortcut.h"
#include "froglies.h"

namespace FrogLies {
    std::vector<Shortcut*> Shortcut::Shortcuts;
    Shortcut    ShortcutWin ( "Ctrl Alt 2" ), ShortcutDesk( "Ctrl Alt 3" ),
                ShortcutCrop( "Ctrl Alt 4" ), ShortcutClip( "Ctrl Alt 5" ),
                ShortcutQuit( "Ctrl Alt Q" );

    Shortcut::Shortcut( std::string val ) {
        Set( val );
        Shortcuts.push_back( this );
    }
    int Shortcut::IsValid( ) {
        for( unsigned int i = 0; i < Shortcuts.size(); ++i ) {
                if( Shortcuts[i] == this ) {
                        return 1;
                    }
            }
        return 0;
    }
    void Shortcut::Set( std::string val ) {
        unsigned int pos = 0;
        keys.clear();
        for( unsigned int i = 0; i < val.size(); ++i ) {
                if( val[i] <= ' ' ) {
                        if( i == pos ) {
                                pos++;
                                continue;
                            }
                        keys.push_back( val.substr( pos, i - pos ) );
                        pos = i + 1;
                    }
            }
        keys.push_back( val.substr( pos ) );
    }
    int Shortcut::IsHit() {
        unsigned int s = 0;
        for( unsigned int i = 0; i < keys.size(); ++i ) {
                s += ReadKey( keys[i] );
            }
        if( s > keys.size() ) {
                return true;
            }
        return false;
    }
}
