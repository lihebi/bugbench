/*
 * Generated automatically from paths.h.in by the
 * configure script.
 */
/* paths.h - included in man-config.c */
/*
 * Define the absolute path to the configuration file and programs used.
 * (If no configuration file is found then the preset values are used.)
 */
#ifndef CONFIG_FILE
#define CONFIG_FILE "/usr/share/misc/man.conf"
#endif

static struct paths {
    char *name;
    char *path;			/* path plus command options - never NULL */
} paths[] = {
    { "MANBIN",	"" },		/* value unused */
    { "APROPOS",  "/usr/bin/apropos" },
    { "WHATIS",   "/usr/bin/whatis" },
    { "TROFF",	"/usr/bin/groff -Tps -mandoc" },
    { "NROFF",	"/usr/bin/groff -Tlatin1 -mandoc" },
    { "EQN",	"/usr/bin/geqn -Tps" },
    { "NEQN",	"/usr/bin/geqn -Tlatin1" },
    { "TBL",	"/usr/bin/gtbl" },
    { "COL",	"" },
    { "REFER",	"/usr/bin/grefer" },
    { "PIC",	"/usr/bin/gpic" },
    { "VGRIND",	"" },
    { "GRAP",	"" },
    { "PAGER",	"/usr/bin/less -is" },
    { "CMP",	"/usr/bin/cmp -s" },
    { "CAT",	"/bin/cat" },
    { "COMPRESS",	"/bin/gzip" },
    { "COMPRESS_EXT", ".gz" }, /* not a path, just a string variable */
    { "DECOMPRESS",	"/usr/bin/gunzip -c" },
    { "MANSECT",  "1:8:2:3:4:5:6:7:9:tcl:n:l:p:o"}           /* idem */
};
