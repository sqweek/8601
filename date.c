#include <u.h>
#include <libc.h>


void
usage() {
	fprint(2, "usage: 8601/date [-z zone]\n");
	exits("usage");
}

void
main(int argc, char *argv[])
{
	ulong now;
	Tm *t;
	char *zone = nil;

	ARGBEGIN{
	case 'z':	zone = EARGF(usage()); break;
	default:	usage();
	}ARGEND

	now = time(0);

	if(zone) {
		long offset = 0;
		if(strcmp(zone, "Z") == 0) {
			offset = 0;
		} else {
			char *cp = zone;
			int dir = 1;
			if(*cp == '-') {
				dir = -1;
				++cp;
			} else if(*cp == '+') {
				++cp;
			} else {
				fprint(2, "invalid timezone %s\n", zone);
				exits("timezone");
			}
			offset += 60*60*strtol(cp, &cp, 10);
			if(*cp == ':') {
				++cp;
			} else {
				fprint(2, "invalid timezone %s\n", zone);
				exits("timezone");
			}
			offset += 60*strtol(cp, &cp, 10);
			offset *= dir;
		}
		t = gmtime(now+offset);
		print("%04d-%02d-%02dT%02d:%02d:%02d%s\n", t->year+1900, t->mon+1, t->mday, t->hour, t->min, t->sec, zone);
	} else {
		t = localtime(now);
		print("%04d-%02d-%02dT%02d:%02d:%02d%+03d:%02d\n", t->year+1900, t->mon+1, t->mday, t->hour, t->min, t->sec, t->tzoff/3600, abs(t->tzoff)%3600);
	}

	exits(0);
}
