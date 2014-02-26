#include <string>
#include <windows.h>
#include <curl/curl.h>
#include <stdarg.h>
#include <stdio.h>
#include "png.h"

//#include "shitty_system_tray.h"


#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

// Windows Header Files:
#include <windows.h>
#include <Windowsx.h>
#include <commctrl.h>
#include <shellapi.h>
#include <Shlwapi.h>

// C RunTime Header Files
#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

#include <process.h>

#include <string.h>

const char* mimes[] = {
    "3dm",          "x-world/x-3dmf",
    "3dmf",         "x-world/x-3dmf",
    "a",            "application/octet-stream",
    "aab",          "application/x-authorware-bin",
    "aam",          "application/x-authorware-map",
    "aas",          "application/x-authorware-seg",
    "abc",          "text/vnd.abc",
    "acgi",         "text/html",
    "afl",          "video/animaflex",
    "ai",           "application/postscript",
    "aif",          "audio/aiff",
    "aif",          "audio/x-aiff",
    "aifc",         "audio/aiff",
    "aifc",         "audio/x-aiff",
    "aiff",         "audio/aiff",
    "aiff",         "audio/x-aiff",
    "aim",          "application/x-aim",
    "aip",          "text/x-audiosoft-intra",
    "ani",          "application/x-navi-animation",
    "aos",          "application/x-nokia-9000-communicator-add-on-software",
    "aps",          "application/mime",
    "arc",          "application/octet-stream",
    "arj",          "application/arj",
    "arj",          "application/octet-stream",
    "art",          "image/x-jg",
    "asf",          "video/x-ms-asf",
    "asm",          "text/x-asm",
    "asp",          "text/asp",
    "asx",          "application/x-mplayer2",
    "asx",          "video/x-ms-asf",
    "asx",          "video/x-ms-asf-plugin",
    "au",           "audio/basic",
    "au",           "audio/x-au",
    "avi",          "application/x-troff-msvideo",
    "avi",          "video/avi",
    "avi",          "video/msvideo",
    "avi",          "video/x-msvideo",
    "avs",          "video/avs-video",
    "bcpio",        "application/x-bcpio",
    "bin",          "application/mac-binary",
    "bin",          "application/macbinary",
    "bin",          "application/octet-stream",
    "bin",          "application/x-binary",
    "bin",          "application/x-macbinary",
    "bm",           "image/bmp",
    "bmp",          "image/bmp",
    "bmp",          "image/x-windows-bmp",
    "boo",          "application/book",
    "book",         "application/book",
    "boz",          "application/x-bzip2",
    "bsh",          "application/x-bsh",
    "bz",           "application/x-bzip",
    "bz2",          "application/x-bzip2",
    "c",            "text/plain",
    "c",            "text/x-c",
    "c++",          "text/plain",
    "cat",          "application/vnd.ms-pki.seccat",
    "cc",           "text/plain",
    "cc",           "text/x-c",
    "ccad",         "application/clariscad",
    "cco",          "application/x-cocoa",
    "cdf",          "application/cdf",
    "cdf",          "application/x-cdf",
    "cdf",          "application/x-netcdf",
    "cer",          "application/pkix-cert",
    "cer",          "application/x-x509-ca-cert",
    "cha",          "application/x-chat",
    "chat",         "application/x-chat",
    "class",        "application/java",
    "class",        "application/java-byte-code",
    "class",        "application/x-java-class",
    "com",          "application/octet-stream",
    "com",          "text/plain",
    "conf",         "text/plain",
    "cpio",         "application/x-cpio",
    "cpp",          "text/x-c",
    "cpt",          "application/mac-compactpro",
    "cpt",          "application/x-compactpro",
    "cpt",          "application/x-cpt",
    "crl",          "application/pkcs-crl",
    "crl",          "application/pkix-crl",
    "crt",          "application/pkix-cert",
    "crt",          "application/x-x509-ca-cert",
    "crt",          "application/x-x509-user-cert",
    "csh",          "application/x-csh",
    "csh",          "text/x-script.csh",
    "css",          "application/x-pointplus",
    "css",          "text/css",
    "cxx",          "text/plain",
    "dcr",          "application/x-director",
    "deepv",        "application/x-deepv",
    "def",          "text/plain",
    "der",          "application/x-x509-ca-cert",
    "dif",          "video/x-dv",
    "dir",          "application/x-director",
    "dl",           "video/dl",
    "dl",           "video/x-dl",
    "doc",          "application/msword",
    "dot",          "application/msword",
    "dp",           "application/commonground",
    "drw",          "application/drafting",
    "dump",         "application/octet-stream",
    "dv",           "video/x-dv",
    "dvi",          "application/x-dvi",
    "dwf",          "drawing/x-dwf (old)",
    "dwf",          "model/vnd.dwf",
    "dwg",          "application/acad",
    "dwg",          "image/vnd.dwg",
    "dwg",          "image/x-dwg",
    "dxf",          "application/dxf",
    "dxf",          "image/vnd.dwg",
    "dxf",          "image/x-dwg",
    "dxr",          "application/x-director",
    "el",           "text/x-script.elisp",
    "elc",          "application/x-bytecode.elisp (compiled elisp)",
    "elc",          "application/x-elc",
    "env",          "application/x-envoy",
    "eps",          "application/postscript",
    "es",           "application/x-esrehber",
    "etx",          "text/x-setext",
    "evy",          "application/envoy",
    "evy",          "application/x-envoy",
    "exe",          "application/octet-stream",
    "f",            "text/plain",
    "f",            "text/x-fortran",
    "f77",          "text/x-fortran",
    "f90",          "text/plain",
    "f90",          "text/x-fortran",
    "fdf",          "application/vnd.fdf",
    "fif",          "application/fractals",
    "fif",          "image/fif",
    "fli",          "video/fli",
    "fli",          "video/x-fli",
    "flo",          "image/florian",
    "flx",          "text/vnd.fmi.flexstor",
    "fmf",          "video/x-atomic3d-feature",
    "for",          "text/plain",
    "for",          "text/x-fortran",
    "fpx",          "image/vnd.fpx",
    "fpx",          "image/vnd.net-fpx",
    "frl",          "application/freeloader",
    "funk",         "audio/make",
    "g",            "text/plain",
    "g3",           "image/g3fax",
    "gif",          "image/gif",
    "gl",           "video/gl",
    "gl",           "video/x-gl",
    "gsd",          "audio/x-gsm",
    "gsm",          "audio/x-gsm",
    "gsp",          "application/x-gsp",
    "gss",          "application/x-gss",
    "gtar",         "application/x-gtar",
    "gz",           "application/x-compressed",
    "gz",           "application/x-gzip",
    "gzip",         "application/x-gzip",
    "gzip",         "multipart/x-gzip",
    "h",            "text/plain",
    "h",            "text/x-h",
    "hdf",          "application/x-hdf",
    "help",         "application/x-helpfile",
    "hgl",          "application/vnd.hp-hpgl",
    "hh",           "text/plain",
    "hh",           "text/x-h",
    "hlb",          "text/x-script",
    "hlp",          "application/hlp",
    "hlp",          "application/x-helpfile",
    "hlp",          "application/x-winhelp",
    "hpg",          "application/vnd.hp-hpgl",
    "hpgl",         "application/vnd.hp-hpgl",
    "hqx",          "application/binhex",
    "hqx",          "application/binhex4",
    "hqx",          "application/mac-binhex",
    "hqx",          "application/mac-binhex40",
    "hqx",          "application/x-binhex40",
    "hqx",          "application/x-mac-binhex40",
    "hta",          "application/hta",
    "htc",          "text/x-component",
    "htm",          "text/html",
    "html",         "text/html",
    "htmls",        "text/html",
    "htt",          "text/webviewhtml",
    "htx",          "text/html",
    "ice",          "x-conference/x-cooltalk",
    "ico",          "image/x-icon",
    "idc",          "text/plain",
    "ief",          "image/ief",
    "iefs",         "image/ief",
    "iges",         "application/iges",
    "iges",         "model/iges",
    "igs",          "application/iges",
    "igs",          "model/iges",
    "ima",          "application/x-ima",
    "imap",         "application/x-httpd-imap",
    "inf",          "application/inf",
    "ins",          "application/x-internett-signup",
    "ip",           "application/x-ip2",
    "isu",          "video/x-isvideo",
    "it",           "audio/it",
    "iv",           "application/x-inventor",
    "ivr",          "i-world/i-vrml",
    "ivy",          "application/x-livescreen",
    "jam",          "audio/x-jam",
    "jav",          "text/plain",
    "jav",          "text/x-java-source",
    "java",         "text/plain",
    "java",         "text/x-java-source",
    "jcm",          "application/x-java-commerce",
    "jfif",         "image/jpeg",
    "jfif",         "image/pjpeg",
    "jfif-tbnl",    "image/jpeg",
    "jpe",          "image/jpeg",
    "jpe",          "image/pjpeg",
    "jpeg",         "image/jpeg",
    "jpeg",         "image/pjpeg",
    "jpg",          "image/jpeg",
    "jpg",          "image/pjpeg",
    "jps",          "image/x-jps",
    "js",           "application/x-javascript",
    "jut",          "image/jutvision",
    "kar",          "audio/midi",
    "kar",          "music/x-karaoke",
    "ksh",          "application/x-ksh",
    "ksh",          "text/x-script.ksh",
    "la",           "audio/nspaudio",
    "la",           "audio/x-nspaudio",
    "lam",          "audio/x-liveaudio",
    "latex",        "application/x-latex",
    "lha",          "application/lha",
    "lha",          "application/octet-stream",
    "lha",          "application/x-lha",
    "lhx",          "application/octet-stream",
    "list",         "text/plain",
    "lma",          "audio/nspaudio",
    "lma",          "audio/x-nspaudio",
    "log",          "text/plain",
    "lsp",          "application/x-lisp",
    "lsp",          "text/x-script.lisp",
    "lst",          "text/plain",
    "lsx",          "text/x-la-asf",
    "ltx",          "application/x-latex",
    "lzh",          "application/octet-stream",
    "lzh",          "application/x-lzh",
    "lzx",          "application/lzx",
    "lzx",          "application/octet-stream",
    "lzx",          "application/x-lzx",
    "m",            "text/plain",
    "m",            "text/x-m",
    "m1v",          "video/mpeg",
    "m2a",          "audio/mpeg",
    "m2v",          "video/mpeg",
    "m3u",          "audio/x-mpequrl",
    "man",          "application/x-troff-man",
    "map",          "application/x-navimap",
    "mar",          "text/plain",
    "mbd",          "application/mbedlet",
    "mc$",          "application/x-magic-cap-package-1.0",
    "mcd",          "application/mcad",
    "mcd",          "application/x-mathcad",
    "mcf",          "image/vasa",
    "mcf",          "text/mcf",
    "mcp",          "application/netmc",
    "me",           "application/x-troff-me",
    "mht",          "message/rfc822",
    "mhtml",        "message/rfc822",
    "mid",          "application/x-midi",
    "mid",          "audio/midi",
    "mid",          "audio/x-mid",
    "mid",          "audio/x-midi",
    "mid",          "music/crescendo",
    "mid",          "x-music/x-midi",
    "midi",         "application/x-midi",
    "midi",         "audio/midi",
    "midi",         "audio/x-mid",
    "midi",         "audio/x-midi",
    "midi",         "music/crescendo",
    "midi",         "x-music/x-midi",
    "mif",          "application/x-frame",
    "mif",          "application/x-mif",
    "mime",         "message/rfc822",
    "mime",         "www/mime",
    "mjf",          "audio/x-vnd.audioexplosion.mjuicemediafile",
    "mjpg",         "video/x-motion-jpeg",
    "mm",           "application/base64",
    "mm",           "application/x-meme",
    "mme",          "application/base64",
    "mod",          "audio/mod",
    "mod",          "audio/x-mod",
    "moov",         "video/quicktime",
    "mov",          "video/quicktime",
    "movie",        "video/x-sgi-movie",
    "mp2",          "audio/mpeg",
    "mp2",          "audio/x-mpeg",
    "mp2",          "video/mpeg",
    "mp2",          "video/x-mpeg",
    "mp2",          "video/x-mpeq2a",
    "mp3",          "audio/mpeg3",
    "mp3",          "audio/x-mpeg-3",
    "mp3",          "video/mpeg",
    "mp3",          "video/x-mpeg",
    "mpa",          "audio/mpeg",
    "mpa",          "video/mpeg",
    "mpc",          "application/x-project",
    "mpe",          "video/mpeg",
    "mpeg",         "video/mpeg",
    "mpg",          "audio/mpeg",
    "mpg",          "video/mpeg",
    "mpga",         "audio/mpeg",
    "mpp",          "application/vnd.ms-project",
    "mpt",          "application/x-project",
    "mpv",          "application/x-project",
    "mpx",          "application/x-project",
    "mrc",          "application/marc",
    "ms",           "application/x-troff-ms",
    "mv",           "video/x-sgi-movie",
    "my",           "audio/make",
    "mzz",          "application/x-vnd.audioexplosion.mzz",
    "nap",          "image/naplps",
    "naplps",       "image/naplps",
    "nc",           "application/x-netcdf",
    "ncm",          "application/vnd.nokia.configuration-message",
    "nif",          "image/x-niff",
    "niff",         "image/x-niff",
    "nix",          "application/x-mix-transfer",
    "nsc",          "application/x-conference",
    "nvd",          "application/x-navidoc",
    "o",            "application/octet-stream",
    "oda",          "application/oda",
    "omc",          "application/x-omc",
    "omcd",         "application/x-omcdatamaker",
    "omcr",         "application/x-omcregerator",
    "p",            "text/x-pascal",
    "p10",          "application/pkcs10",
    "p10",          "application/x-pkcs10",
    "p12",          "application/pkcs-12",
    "p12",          "application/x-pkcs12",
    "p7a",          "application/x-pkcs7-signature",
    "p7c",          "application/pkcs7-mime",
    "p7c",          "application/x-pkcs7-mime",
    "p7m",          "application/pkcs7-mime",
    "p7m",          "application/x-pkcs7-mime",
    "p7r",          "application/x-pkcs7-certreqresp",
    "p7s",          "application/pkcs7-signature",
    "part",         "application/pro_eng",
    "pas",          "text/pascal",
    "pbm",          "image/x-portable-bitmap",
    "pcl",          "application/vnd.hp-pcl",
    "pcl",          "application/x-pcl",
    "pct",          "image/x-pict",
    "pcx",          "image/x-pcx",
    "pdb",          "chemical/x-pdb",
    "pdf",          "application/pdf",
    "pfunk",        "audio/make",
    "pfunk",        "audio/make.my.funk",
    "pgm",          "image/x-portable-graymap",
    "pgm",          "image/x-portable-greymap",
    "pic",          "image/pict",
    "pict",         "image/pict",
    "pkg",          "application/x-newton-compatible-pkg",
    "pko",          "application/vnd.ms-pki.pko",
    "pl",           "text/plain",
    "pl",           "text/x-script.perl",
    "plx",          "application/x-pixclscript",
    "pm",           "image/x-xpixmap",
    "pm",           "text/x-script.perl-module",
    "pm4",          "application/x-pagemaker",
    "pm5",          "application/x-pagemaker",
    "png",          "image/png",
    "pnm",          "application/x-portable-anymap",
    "pnm",          "image/x-portable-anymap",
    "pot",          "application/mspowerpoint",
    "pot",          "application/vnd.ms-powerpoint",
    "pov",          "model/x-pov",
    "ppa",          "application/vnd.ms-powerpoint",
    "ppm",          "image/x-portable-pixmap",
    "pps",          "application/mspowerpoint",
    "pps",          "application/vnd.ms-powerpoint",
    "ppt",          "application/mspowerpoint",
    "ppt",          "application/powerpoint",
    "ppt",          "application/vnd.ms-powerpoint",
    "ppt",          "application/x-mspowerpoint",
    "ppz",          "application/mspowerpoint",
    "pre",          "application/x-freelance",
    "prt",          "application/pro_eng",
    "ps",           "application/postscript",
    "psd",          "application/octet-stream",
    "pvu",          "paleovu/x-pv",
    "pwz",          "application/vnd.ms-powerpoint",
    "py",           "text/x-script.phyton",
    "pyc",          "applicaiton/x-bytecode.python",
    "qcp",          "audio/vnd.qcelp",
    "qd3",          "x-world/x-3dmf",
    "qd3d",         "x-world/x-3dmf",
    "qif",          "image/x-quicktime",
    "qt",           "video/quicktime",
    "qtc",          "video/x-qtc",
    "qti",          "image/x-quicktime",
    "qtif",         "image/x-quicktime",
    "ra",           "audio/x-pn-realaudio",
    "ra",           "audio/x-pn-realaudio-plugin",
    "ra",           "audio/x-realaudio",
    "ram",          "audio/x-pn-realaudio",
    "ras",          "application/x-cmu-raster",
    "ras",          "image/cmu-raster",
    "ras",          "image/x-cmu-raster",
    "rast",         "image/cmu-raster",
    "rexx",         "text/x-script.rexx",
    "rf",           "image/vnd.rn-realflash",
    "rgb",          "image/x-rgb",
    "rm",           "application/vnd.rn-realmedia",
    "rm",           "audio/x-pn-realaudio",
    "rmi",          "audio/mid",
    "rmm",          "audio/x-pn-realaudio",
    "rmp",          "audio/x-pn-realaudio",
    "rmp",          "audio/x-pn-realaudio-plugin",
    "rng",          "application/ringing-tones",
    "rng",          "application/vnd.nokia.ringing-tone",
    "rnx",          "application/vnd.rn-realplayer",
    "roff",         "application/x-troff",
    "rp",           "image/vnd.rn-realpix",
    "rpm",          "audio/x-pn-realaudio-plugin",
    "rt",           "text/richtext",
    "rt",           "text/vnd.rn-realtext",
    "rtf",          "application/rtf",
    "rtf",          "application/x-rtf",
    "rtf",          "text/richtext",
    "rtx",          "application/rtf",
    "rtx",          "text/richtext",
    "rv",           "video/vnd.rn-realvideo",
    "s",            "text/x-asm",
    "s3m",          "audio/s3m",
    "saveme",       "application/octet-stream",
    "sbk",          "application/x-tbook",
    "scm",          "application/x-lotusscreencam",
    "scm",          "text/x-script.guile",
    "scm",          "text/x-script.scheme",
    "scm",          "video/x-scm",
    "sdml",         "text/plain",
    "sdp",          "application/sdp",
    "sdp",          "application/x-sdp",
    "sdr",          "application/sounder",
    "sea",          "application/sea",
    "sea",          "application/x-sea",
    "set",          "application/set",
    "sgm",          "text/sgml",
    "sgm",          "text/x-sgml",
    "sgml",         "text/sgml",
    "sgml",         "text/x-sgml",
    "sh",           "application/x-bsh",
    "sh",           "application/x-sh",
    "sh",           "application/x-shar",
    "sh",           "text/x-script.sh",
    "shar",         "application/x-bsh",
    "shar",         "application/x-shar",
    "shtml",        "text/html",
    "shtml",        "text/x-server-parsed-html",
    "sid",          "audio/x-psid",
    "sit",          "application/x-sit",
    "sit",          "application/x-stuffit",
    "skd",          "application/x-koan",
    "skm",          "application/x-koan",
    "skp",          "application/x-koan",
    "skt",          "application/x-koan",
    "sl",           "application/x-seelogo",
    "smi",          "application/smil",
    "smil",         "application/smil",
    "snd",          "audio/basic",
    "snd",          "audio/x-adpcm",
    "sol",          "application/solids",
    "spc",          "application/x-pkcs7-certificates",
    "spc",          "text/x-speech",
    "spl",          "application/futuresplash",
    "spr",          "application/x-sprite",
    "sprite",       "application/x-sprite",
    "src",          "application/x-wais-source",
    "ssi",          "text/x-server-parsed-html",
    "ssm",          "application/streamingmedia",
    "sst",          "application/vnd.ms-pki.certstore",
    "step",         "application/step",
    "stl",          "application/sla",
    "stl",          "application/vnd.ms-pki.stl",
    "stl",          "application/x-navistyle",
    "stp",          "application/step",
    "sv4cpio",      "application/x-sv4cpio",
    "sv4crc",       "application/x-sv4crc",
    "svf",          "image/vnd.dwg",
    "svf",          "image/x-dwg",
    "svr",          "application/x-world",
    "svr",          "x-world/x-svr",
    "swf",          "application/x-shockwave-flash",
    "t",            "application/x-troff",
    "talk",         "text/x-speech",
    "tar",          "application/x-tar",
    "tbk",          "application/toolbook",
    "tbk",          "application/x-tbook",
    "tcl",          "application/x-tcl",
    "tcl",          "text/x-script.tcl",
    "tcsh",         "text/x-script.tcsh",
    "tex",          "application/x-tex",
    "texi",         "application/x-texinfo",
    "texinfo",      "application/x-texinfo",
    "text",         "application/plain",
    "text",         "text/plain",
    "tgz",          "application/gnutar",
    "tgz",          "application/x-compressed",
    "tif",          "image/tiff",
    "tif",          "image/x-tiff",
    "tiff",         "image/tiff",
    "tiff",         "image/x-tiff",
    "tr",           "application/x-troff",
    "tsi",          "audio/tsp-audio",
    "tsp",          "application/dsptype",
    "tsp",          "audio/tsplayer",
    "tsv",          "text/tab-separated-values",
    "turbot",       "image/florian",
    "txt",          "text/plain",
    "uil",          "text/x-uil",
    "uni",          "text/uri-list",
    "unis",         "text/uri-list",
    "unv",          "application/i-deas",
    "uri",          "text/uri-list",
    "uris",         "text/uri-list",
    "ustar",        "application/x-ustar",
    "ustar",        "multipart/x-ustar",
    "uu",           "application/octet-stream",
    "uu",           "text/x-uuencode",
    "uue",          "text/x-uuencode",
    "vcd",          "application/x-cdlink",
    "vcs",          "text/x-vcalendar",
    "vda",          "application/vda",
    "vdo",          "video/vdo",
    "vew",          "application/groupwise",
    "viv",          "video/vivo",
    "viv",          "video/vnd.vivo",
    "vivo",         "video/vivo",
    "vivo",         "video/vnd.vivo",
    "vmd",          "application/vocaltec-media-desc",
    "vmf",          "application/vocaltec-media-file",
    "voc",          "audio/voc",
    "voc",          "audio/x-voc",
    "vos",          "video/vosaic",
    "vox",          "audio/voxware",
    "vqe",          "audio/x-twinvq-plugin",
    "vqf",          "audio/x-twinvq",
    "vql",          "audio/x-twinvq-plugin",
    "vrml",         "application/x-vrml",
    "vrml",         "model/vrml",
    "vrml",         "x-world/x-vrml",
    "vrt",          "x-world/x-vrt",
    "vsd",          "application/x-visio",
    "vst",          "application/x-visio",
    "vsw",          "application/x-visio",
    "w60",          "application/wordperfect6.0",
    "w61",          "application/wordperfect6.1",
    "w6w",          "application/msword",
    "wav",          "audio/wav",
    "wav",          "audio/x-wav",
    "wb1",          "application/x-qpro",
    "wbmp",         "image/vnd.wap.wbmp",
    "web",          "application/vnd.xara",
    "wiz",          "application/msword",
    "wk1",          "application/x-123",
    "wmf",          "windows/metafile",
    "wml",          "text/vnd.wap.wml",
    "wmlc",         "application/vnd.wap.wmlc",
    "wmls",         "text/vnd.wap.wmlscript",
    "wmlsc",        "application/vnd.wap.wmlscriptc",
    "word",         "application/msword",
    "wp",           "application/wordperfect",
    "wp5",          "application/wordperfect",
    "wp5",          "application/wordperfect6.0",
    "wp6",          "application/wordperfect",
    "wpd",          "application/wordperfect",
    "wpd",          "application/x-wpwin",
    "wq1",          "application/x-lotus",
    "wri",          "application/mswrite",
    "wri",          "application/x-wri",
    "wrl",          "application/x-world",
    "wrl",          "model/vrml",
    "wrl",          "x-world/x-vrml",
    "wrz",          "model/vrml",
    "wrz",          "x-world/x-vrml",
    "wsc",          "text/scriplet",
    "wsrc",         "application/x-wais-source",
    "wtk",          "application/x-wintalk",
    "xbm",          "image/x-xbitmap",
    "xbm",          "image/x-xbm",
    "xbm",          "image/xbm",
    "xdr",          "video/x-amt-demorun",
    "xgz",          "xgl/drawing",
    "xif",          "image/vnd.xiff",
    "xl",           "application/excel",
    "xla",          "application/excel",
    "xla",          "application/x-excel",
    "xla",          "application/x-msexcel",
    "xlb",          "application/excel",
    "xlb",          "application/vnd.ms-excel",
    "xlb",          "application/x-excel",
    "xlc",          "application/excel",
    "xlc",          "application/vnd.ms-excel",
    "xlc",          "application/x-excel",
    "xld",          "application/excel",
    "xld",          "application/x-excel",
    "xlk",          "application/excel",
    "xlk",          "application/x-excel",
    "xll",          "application/excel",
    "xll",          "application/vnd.ms-excel",
    "xll",          "application/x-excel",
    "xlm",          "application/excel",
    "xlm",          "application/vnd.ms-excel",
    "xlm",          "application/x-excel",
    "xls",          "application/excel",
    "xls",          "application/vnd.ms-excel",
    "xls",          "application/x-excel",
    "xls",          "application/x-msexcel",
    "xlt",          "application/excel",
    "xlt",          "application/x-excel",
    "xlv",          "application/excel",
    "xlv",          "application/x-excel",
    "xlw",          "application/excel",
    "xlw",          "application/vnd.ms-excel",
    "xlw",          "application/x-excel",
    "xlw",          "application/x-msexcel",
    "xm",           "audio/xm",
    "xml",          "application/xml",
    "xml",          "text/xml",
    "xmz",          "xgl/movie",
    "xpix",         "application/x-vnd.ls-xpix",
    "xpm",          "image/x-xpixmap",
    "xpm",          "image/xpm",
    "x-png",        "image/png",
    "xsr",          "video/x-amt-showrun",
    "xwd",          "image/x-xwd",
    "xwd",          "image/x-xwindowdump",
    "xyz",          "chemical/x-pdb",
    "z",            "application/x-compress",
    "z",            "application/x-compressed",
    "zip",          "application/x-compressed",
    "zip",          "application/x-zip-compressed",
    "zip",          "application/zip",
    "zip",          "multipart/x-zip",
    "zoo",          "application/octet-stream",
    "zsh",          "text/x-script.zsh",
    0,              0
};

void CheckKeys();

std::string GetMimeFromExt( std::string ext ) {
    for( int i = 0; mimes[i] != 0; i += 2 ) {
        if( ext == mimes[i] )
            return mimes[i+1];
    }
    return "application/octet-stream";
}

class WHFF {
    static int HasInit;
    std::string Owner;
    std::string LastUpload;
    static size_t callback( char *ptr, size_t size, size_t nmemb, void *userdata);
public:
    WHFF( std::string owner );
    void SetOwner( std::string owner );
    int Upload( std::string name, const void* data, size_t datalen, std::string mimetype = "application/octet-stream", std::string password = "" );
    int Upload( std::string fname, std::string password = "" );
    std::string GetLastUpload();
};
int WHFF::HasInit = 0;

WHFF::WHFF( std::string owner ) {
    if( !HasInit ) {
        HasInit = 1;
        curl_global_init(CURL_GLOBAL_WIN32);
    }
    SetOwner( owner );
}

void WHFF::SetOwner( std::string owner ) {
    Owner = owner;
}

size_t WHFF::callback( char *ptr, size_t size, size_t nmemb, void *userdata ) {
    printf("HAH %d|%s|", size * nmemb, ptr);
    char* tmp = (char*) malloc( size*nmemb + 1 );
    memcpy( tmp, ptr, size*nmemb );
    tmp[size*nmemb] = 0;
    std::string value = tmp;
    if( value.substr(0, 5) != "Error" )
        value = "http://fiel.tk/?i=" + value;
    WHFF* self = (WHFF*) userdata;
    self->LastUpload = value;
    return size * nmemb;
}

std::string WHFF::GetLastUpload() {
    return LastUpload;
}

void* FillTemplate( unsigned int& bpos, const char** Template, ... ) {
    va_list args;
    va_start( args, Template );
    unsigned int bsize = 1000;
    char* buffer = (char*) malloc( bsize );
    bpos = 0;

    for( int i = 0; Template[i] != 0; i += 2 ) {
        switch( Template[i][0] ) {
        case 'R':
        case 'r': {
            unsigned int rawlen = strlen( Template[i+1] );
            while( bpos + rawlen >= bsize ) {
                bsize *= 2;
                buffer = (char*) realloc( buffer, bsize );
            }
            bpos += sprintf( buffer + bpos, "%s", Template[i+1] );
        }
        break;
        case 'S':
        case 's': {
            char* arg = va_arg( args, char* );
            unsigned int rawlen = strlen( arg );
            while( bpos + rawlen >= bsize ) {
                bsize *= 2;
                buffer = (char*) realloc( buffer, bsize );
            }
            bpos += sprintf( buffer + bpos, "%s", arg );
        }
        break;
        case 'D':
        case 'd': {
            void* arg = va_arg( args, void* );
            unsigned int rawlen = va_arg( args, unsigned int );
            while( bpos + rawlen >= bsize ) {
                bsize *= 2;
                buffer = (char*) realloc( buffer, bsize );
            }
            memcpy( buffer + bpos, arg, rawlen );
            bpos += rawlen;
        }
        break;
        }
    }
    return buffer;
}

int WHFF::Upload( std::string name, const void* data, size_t datalen, std::string mimetype, std::string password ) {
    for( unsigned int i = 0; i < name.length(); ++i ) {
        if( name[i] == '\"' || name[i] < ' ' )
            name[i] = ' ';
    }

    const char * posttemplate[] = {
        "raw",  "-----------------------------28251299466151\r\n",
        "raw",  "Content-Disposition: form-data; name=\"file\"; filename=\"",
        "str",  "",
        "raw",  "\"\r\n",
        "raw",  "Content-Type: ",
        "str",  "",
        "raw",  "\r\n\r\n",
        "dat",  "",
        "raw",  "\r\n-----------------------------28251299466151\r\n",
        "raw",  "Content-Disposition: form-data; name=\"password\"\r\n\r\n",
        "dat",  "",
        "raw",  "\r\n-----------------------------28251299466151\r\n",
        "raw",  "Content-Disposition: form-data; name=\"owner\"\r\n\r\n",
        "dat",  "",
        "raw",  "\r\n-----------------------------28251299466151--\r\n",
        0, 0
    };
    unsigned int length;
    void *postobject = FillTemplate(   length, posttemplate,
                                       name.c_str(),
                                       mimetype.c_str(),
                                       data, datalen,
                                       password.c_str(), password.length(),
                                       Owner.c_str(), Owner.length() );

    printf("%s\n", (char*) postobject );

    CURL *curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
    const char *theUrl = "http://frogbox.es/whff/upload.php?raw";
    curl_easy_setopt(curl, CURLOPT_URL, theUrl );
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postobject);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, length);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WHFF::callback );
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, this );



    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: multipart/form-data; boundary=---------------------------28251299466151" );
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);


    curl_easy_perform(curl);
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    free( postobject );
    return 1;
}

void* read_file_to_buffer( std::string fname, size_t& length ) {
    FILE* f = fopen( fname.c_str(), "rb" );
    if( f == 0 ) return 0;
    length = 1000;
    unsigned char* ret = (unsigned char*) malloc( length );
    size_t i = 0;

    while( true ) {
        size_t hasread = fread( ret + i, 1, length - i, f);
        i += hasread;
        if( feof( f ) ) {
            length = i;
            break;
        }
        length *= 2;
        ret = (unsigned char*) realloc( ret, length );
    }
    return ret;
}

std::string basename( std::string in ) {
    int pos = 0;
    for( int i = 0; in[i]!= 0; ++i ) {
        if( in[i] == '/' || in[i] == '\\' )
            pos = i+1;
    }
    std::string ret = in.c_str()+pos;
    return ret;
}

std::string extension( std::string in ) {
    int pos = 0;
    for( int i = 0; in[i]!= 0; ++i ) {
        if( in[i] == '.' )
            pos = i+1;
    }
    std::string ret = in.c_str()+pos;
    return ret;
}

int WHFF::Upload( std::string fname, std::string password ) {
    void* buffer;
    size_t len;
    buffer = read_file_to_buffer( fname, len );
    Upload( basename( fname ), buffer, len, GetMimeFromExt( extension( fname ) ), password );
    return 1;
}

void SetClipboard( std::string text ) {
    if(OpenClipboard(NULL)) {
        HGLOBAL clipbuffer;
        char *buffer;
        EmptyClipboard();
        clipbuffer = GlobalAlloc(GMEM_DDESHARE, text.length()+1);
        buffer = (char*)GlobalLock(clipbuffer);
        strcpy(buffer, text.c_str());
        GlobalUnlock(clipbuffer);
        SetClipboardData(CF_TEXT,clipbuffer);
        CloseClipboard();
    }
}

class Bitmap {
    int Width, Height;
    unsigned char* Data;
    void* pngdata;
    size_t pnglen;
    size_t pngpos;
    int* references;
public:
    Bitmap() {
        Data = NULL;
        Width = Height = 0;
        references = new int;
        *references = 1;
        pngdata = 0;
        pnglen = pngpos = 0;
    }
    Bitmap( const Bitmap& other ) {
        Data = other.Data;
        Width = other.Width;
        Height = other.Height;
        references = other.references;
        (*references)++;
        pngdata = 0;
        pnglen = pngpos = 0;
    }

    ~Bitmap() {
        (*references)--;
        if( *references <= 0 ) {
            if( Data != NULL )
                free( Data );
            delete references;
        }
        if( pngdata != NULL )
            free( pngdata );
    }
    void Write( int width, int height, void* data ) {
        Data = (unsigned char*) malloc( width * height * 4 );
        memcpy( Data, data, width * height * 4 );
        Width = width;
        Height = height;
    }
    void* Read() {
        return Data;
    }
    int W() {
        return Width;
    }
    int H() {
        return Height;
    }

    void Crop( int x, int y, int w, int h ) {
        if( x < 0 ) {
            w += x;
            x = 0;
        }
        if( y < 0 ) {
            h += y;
            y = 0;
        }
        if( x >= Width ) {
            x = Width-1;
        }
        if( y >= Height ) {
            y = Height-1;
        }
        if( x + w > Width ) {
            w = Width - x;
        }
        if( y + h > Height ) {
            h = Height - y;
        }

        if( Data == 0 ) return;
        unsigned char* newdata = (unsigned char*) malloc( w * h * 4 );
        for( int i = 0; i < h; ++i ) {
            memcpy( newdata + i * w * 4, Data + (y+i)*Width*4 + x*4, w*4);
        }
        free( Data );
        Data = newdata;
        Width = w;
        Height = h;
    }
    static Bitmap *self;
    static void pngwrite(png_structp png_ptr,png_bytep data, png_uint_32 length) {
        //Bitmap* self = (Bitmap*) png_get_io_ptr(png_ptr);
        int changed = 0;
        while( self->pngpos + length > self->pnglen ) {
            self->pnglen *= 2;
            changed = 1;
        }
        if( changed ) {
            self->pngdata = realloc( self->pngdata, self->pnglen );
        }
        memcpy(self->pngdata+self->pngpos, data, length);
        self->pngpos += length;
    }


    static void pngflush(png_structp png_ptr) {

    }

    size_t PNGLen() {
        return pngpos;
    }

    void* ReadPNG() {
        self = this;
        if( pngdata )
            free( pngdata );
        pnglen = 1000;
        pngpos = 0;
        pngdata = malloc( pnglen );



        FILE *fp = fopen("t.png", "wb");
        png_structp png_ptr = NULL;
        png_infop info_ptr = NULL;
        size_t x, y;
        png_uint_32 bytes_per_row;
        png_byte **row_pointers = NULL;
        int pixel_size = 4;

        if (fp == NULL) return 0;

        /* Initialize the write struct. */
        png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
        if (png_ptr == NULL) {
            fclose(fp);
            return 0;
        }

        /* Initialize the info struct. */
        info_ptr = png_create_info_struct(png_ptr);
        if (info_ptr == NULL) {
            png_destroy_write_struct(&png_ptr, NULL);
            fclose(fp);
            return 0;
        }


        /* Set image attributes. */
        png_set_IHDR(png_ptr,
                     info_ptr,
                     Width,
                     Height,
                     8,
                     PNG_COLOR_TYPE_RGB_ALPHA,
                     PNG_INTERLACE_NONE,
                     PNG_COMPRESSION_TYPE_DEFAULT,
                     PNG_FILTER_TYPE_DEFAULT);
        /* Initialize rows of PNG. */
        bytes_per_row = Width * pixel_size;
        row_pointers = (png_byte**)png_malloc(png_ptr, Height * sizeof(png_byte *));
        for (y = 0; y < Height; ++y) {
            uint8_t *row = (uint8_t*)png_malloc(png_ptr, Width * sizeof(uint8_t) * pixel_size);
            row_pointers[y] = (png_byte *)row;
            for (x = 0; x < Width; ++x) {
                *row++ = Data[x*pixel_size+y*Width*pixel_size+2];
                *row++ = Data[x*pixel_size+y*Width*pixel_size+1];
                *row++ = Data[x*pixel_size+y*Width*pixel_size+0];

                //*row++ = Data[x*pixel_size+y*Width*pixel_size+3];
                *row++ = 0xFF;
            }
        }

        /* Actually write the image data. */
        png_set_write_fn (png_ptr, this, (png_rw_ptr)pngwrite, (png_flush_ptr)pngflush );
        //png_init_io(png_ptr, fp);
        png_set_rows(png_ptr, info_ptr, row_pointers);
        png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);


        /* Cleanup. */
        for (y = 0; y < Height; y++) {
            png_free(png_ptr, row_pointers[y]);
        }
        png_free(png_ptr, row_pointers);

        /* Finish writing. */
        png_destroy_write_struct(&png_ptr, &info_ptr);
        fclose(fp);
        return pngdata;
    }
};
Bitmap *Bitmap::self;

Bitmap GetWindow(HWND h) {
    RECT rect;
    RECT rect2;

    Bitmap ret;
    if( !GetClientRect(h, &rect) )
        return ret;
    if( !GetWindowRect(h, &rect2) )
        return ret;



    int width = rect2.right - rect2.left;
    int height = rect2.bottom - rect2.top;
    if( width <= 0 || height <= 0 ) return ret;
    int x = (int)(rect2.left);// + ((int)(rect.right-rect.left) - (int)(rect2.right-rect2.left))/2;
    int y = (int)(rect2.top);// + ((int)(rect.bottom-rect.top) - (int)(rect2.bottom-rect2.top)) - x;
    printf("%d\t%d\n", x, y );
    printf("%d\t%d\n", rect.top, rect2.top);
    if( h == GetDesktopWindow() ){
        width = GetSystemMetrics (SM_CXVIRTUALSCREEN);
        height = GetSystemMetrics (SM_CYVIRTUALSCREEN);
        x = GetSystemMetrics (SM_XVIRTUALSCREEN);
        y = GetSystemMetrics (SM_YVIRTUALSCREEN);
    }
    h = GetDesktopWindow();

    HDC hDC = GetDC(h);
    HDC hCaptureDC = CreateCompatibleDC(hDC);
    HBITMAP hCaptureBitmap =CreateCompatibleBitmap(hDC,
                            width, height);
    HGDIOBJ hOld = SelectObject(hCaptureDC,hCaptureBitmap);
    BitBlt(hCaptureDC,0,0,width,height,
           hDC,x,y,SRCCOPY|CAPTUREBLT);


    SelectObject(hCaptureDC, hOld);

    BITMAPINFOHEADER bmi = {0};
    bmi.biSize = sizeof(BITMAPINFOHEADER);
    bmi.biPlanes = 1;
    bmi.biBitCount = 32;
    bmi.biWidth = width;
    bmi.biHeight = -height;
    bmi.biCompression = BI_RGB;
    bmi.biSizeImage = 0;// 3 * ScreenX * ScreenY;


    BYTE* ScreenData = (BYTE*)malloc(4 * width * height );

    GetDIBits(hCaptureDC, hCaptureBitmap, 0, height, ScreenData, (BITMAPINFO*)&bmi, DIB_RGB_COLORS);

    ret.Write( width, height, ScreenData );


    // SaveCapturedBitmap(hCaptureBitmap); //Place holder - Put your code
    //here to save the captured image to disk
    ReleaseDC(h,hDC);
    DeleteDC(hCaptureDC);
    DeleteObject(hCaptureBitmap);

    return ret;
}

#include <map>
std::map<std::string,int> keyspressed;

int ReadKey( std::string key ){
    if( keyspressed[key] == 2 ){
        keyspressed[key] = 1;
        return 2;
    }
    if( keyspressed[key] == 3 ){
        keyspressed[key] = 0;
        return 2;
    }

    return keyspressed[key];
}

HHOOK	kbdhook;

 LRESULT CALLBACK handlekeys(int code, WPARAM wp, LPARAM lp){
    if(code == HC_ACTION && (wp == WM_SYSKEYUP || wp == WM_KEYUP)){
        char tmp[0xFF] = {0};
		std::string str;
		DWORD msg = 1;
		KBDLLHOOKSTRUCT st_hook = *((KBDLLHOOKSTRUCT*)lp);
		msg += (st_hook.scanCode << 16);
		msg += (st_hook.flags << 24);
		GetKeyNameText(msg, tmp, 0xFF);
		str = std::string(tmp);
		if( keyspressed[str] == 2 )
			keyspressed[str] = 3;
		else
			keyspressed[str] = 0;
        //fprintf(out, "%s\n", str.c_str());
    }
	else if (code == HC_ACTION && (wp == WM_SYSKEYDOWN || wp == WM_KEYDOWN)) {
		char tmp[0xFF] = {0};
		std::string str;
		DWORD msg = 1;
		KBDLLHOOKSTRUCT st_hook = *((KBDLLHOOKSTRUCT*)lp);
		msg += (st_hook.scanCode << 16);
		msg += (st_hook.flags << 24);
		GetKeyNameText(msg, tmp, 0xFF);
		str = std::string(tmp);
		if( keyspressed[str] == 0 )
            keyspressed[str] = 2;
        else
            keyspressed[str] = 1;
	}
	CheckKeys();

	return CallNextHookEx(kbdhook, code, wp, lp);
}


#define CLASSNAME	"winss"
#define WINDOWTITLE	"svchost"
#define ICON_MESSAGE (WM_USER + 1)

bool running;

LRESULT CALLBACK windowprocedure(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam){
    //printf("FISH!");

    switch (msg){
        case WM_CREATE:
            //printf("FISH!");
            break;
    case ICON_MESSAGE:
         switch(lparam){
             case WM_LBUTTONDBLCLK:
                     MessageBox(NULL, "Tray icon double clicked!", "clicked", MB_OK);
                     break;
             case WM_LBUTTONUP:
                     MessageBox(NULL, "Tray icon clicked!", "clicked", MB_OK);
                     break;
             default:
                    return DefWindowProc(hwnd, msg, wparam, lparam);
         };
         break;
    case WM_CLOSE: case WM_DESTROY:
        running = false;
        break;
    default:
        return DefWindowProc(hwnd, msg, wparam, lparam);
    };
    return 0;
}
HWND		hwnd;
void CheckKeys(){
	if (ReadKey("Ctrl") + ReadKey("Alt") + ReadKey("2") >= 4) {
            printf("hi\n");
            WHFF whff("");
            Bitmap mb = GetWindow(GetDesktopWindow());
            void* data = mb.ReadPNG();
            whff.Upload( "Screencap.png", data, mb.PNGLen(), GetMimeFromExt("png"));
            SetClipboard( whff.GetLastUpload() );
        }

        if (ReadKey("Ctrl") + ReadKey("Alt") + ReadKey("3") >= 4) {
                printf("hi\n");
            WHFF whff("");
            Bitmap mb = GetWindow(GetForegroundWindow());
            void* data = mb.ReadPNG();
            whff.Upload( "Screencap.png", data, mb.PNGLen(), GetMimeFromExt("png"));
            SetClipboard( whff.GetLastUpload() );
        }

        if (ReadKey("Ctrl") + ReadKey("Alt") + ReadKey("Q") >= 4) {
                printf("hi\n");
            PostMessage( hwnd, WM_CLOSE, 0, 0 );
        }
}

NOTIFYICONDATA nid;
HICON IconA;
HICON IconB;

void GUIThread( void* ){
	while( running ){
		Sleep(1000);
		nid.hIcon = IconA;
		Shell_NotifyIcon(NIM_MODIFY, &nid);
		Sleep(1000);
		nid.hIcon = IconB;
		Shell_NotifyIcon(NIM_MODIFY, &nid);
	}
}

int WINAPI WinMain(HINSTANCE thisinstance, HINSTANCE previnstance, LPSTR cmdline, int ncmdshow){

	HWND		fgwindow = GetForegroundWindow(); /* Current foreground window */
	MSG		    msg;
	WNDCLASSEX	windowclass;
	HINSTANCE	modulehandle;

    //HINSTANCE thisinstance = (HINSTANCE)GetModuleHandle(NULL);

	windowclass.hInstance = thisinstance;
	windowclass.lpszClassName = CLASSNAME;
	windowclass.lpfnWndProc = windowprocedure;
	windowclass.style = CS_DBLCLKS;
	windowclass.cbSize = sizeof(WNDCLASSEX);
	windowclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	windowclass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	windowclass.hCursor  = LoadCursor(NULL, IDC_ARROW);
	windowclass.lpszMenuName = NULL;
	windowclass.cbClsExtra = 0;
	windowclass.cbWndExtra = 0;
	windowclass.hbrBackground = (HBRUSH)COLOR_BACKGROUND;

	if (!(RegisterClassEx(&windowclass))){ return 1; }

	hwnd = CreateWindowEx(NULL, CLASSNAME, WINDOWTITLE, WS_OVERLAPPEDWINDOW,
                          CW_USEDEFAULT, CW_USEDEFAULT, 640, 480, HWND_DESKTOP, NULL,
                          thisinstance, NULL);

	if (!(hwnd)){ return 1; }

    ShowWindow(hwnd, SW_HIDE);
	UpdateWindow(hwnd);
	SetForegroundWindow(fgwindow); /* Give focus to the previous fg window */

    IconA = (HICON) LoadImage( thisinstance, "icona.ico", IMAGE_ICON, 32, 32, LR_LOADFROMFILE );
    IconB = (HICON) LoadImage( thisinstance, "iconb.ico", IMAGE_ICON, 32, 32, LR_LOADFROMFILE );


    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.uID = 100;
    nid.hWnd = hwnd;
    //nid.uVersion = NOTIFYICON_VERSION;
    nid.uCallbackMessage = ICON_MESSAGE;
    nid.hIcon = IconB; //= LoadIcon(NULL, IDI_APPLICATION);
    snprintf(nid.szTip, 64, "Icon");
    nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;

    Shell_NotifyIcon(NIM_ADD, &nid);

    modulehandle = GetModuleHandle(NULL);
	kbdhook = SetWindowsHookEx(WH_KEYBOARD_LL, (HOOKPROC)handlekeys, modulehandle, NULL);



    running = true;

    _beginthread( GUIThread, 1000, NULL );
    //GUIThread(0);

	while (running) {
		if (!GetMessage(&msg, NULL, 0, 0))
			break;
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

