#include <u.h>
#include <libc.h>

#define expect(x) if(!(x)) return -1

int
parsetm(Tm *tm, char *timestr)
{
	int dir = 1, tzhr, tzmin = 0;
	char *cp;
	tm->year = strtol(timestr, &cp, 10) - 1900;
	expect(*cp == '-');
	tm->mon = strtol(cp+1, &cp, 10) - 1;
	expect(*cp == '-');
	tm->mday = strtol(cp+1, &cp, 10);
	expect(*cp == 'T');
	tm->hour = strtol(cp+1, &cp, 10);
	expect(*cp == ':');
	tm->min = strtol(cp+1, &cp, 10);
	expect(*cp == ':');
	tm->sec = strtol(cp+1, &cp, 10);
	if(*cp == '.') {
		(void)strtol(cp+1, &cp, 10);
	}
	strcpy(tm->zone, "");
	switch(*cp) {
		case 'Z':
			strcpy(tm->zone, "GMT");
		case '\0':
			tm->tzoff = 0;
			break;

		case '-':
			dir = -1;
		case '+': 
			tzhr = strtol(cp+1, &cp, 10);
			if(*cp == ':') {
				tzmin = strtol(cp+1, &cp, 10);
			}
			tm->tzoff = dir*(tzhr*3600+tzmin*60);
			strcpy(tm->zone, "GMT");
			break;

		default:
			return -1;
	}
	return 0;
}

long
parsedur(char *cp)
{
	long duration = 0, val;
	expect(*cp == 'P');
	while(*(cp+1)) {
		val = strtol(cp+1, &cp, 10);
		switch(*cp) {
			case 'Y':
				duration += val*60*60*24*365;
				break;
			case 'D':
				duration += val*60*60*24;
				break;
			case 'T':
				break;
			case 'H':
				duration += val*60*60;
				break;
			case 'M':
				duration += val*60;
				break;
			case 'S':
				duration += val;
				break;
			default:
				fprint(2, "unknown duration suffix %c\n", *cp);
				exits("duration");
		}
	}
	return duration;
}


long
putdur(long duration, char suffix, long val)
{
	if(duration >= val) {
		print("%d%c", duration / val, suffix);
		return duration % val;
	}
	return duration;
}

void
usage() {
	fprint(2, "usage: delta [-n] (START END|DELTA)\n");
	exits("usage");
}

int nflg;

void
main(int argc, char **argv)
{
	long duration = 0;

	ARGBEGIN {
	case 'n':	nflg = 1; break;
	default:	usage();
	} ARGEND

	if(argc == 2) {
		Tm start, end;
		if(parsetm(&start, *argv) != 0) {
			fprint(2, "%s: invalid time: %s\n", argv0, *argv);
			exits("invalid");
		}
		++argv;
		if(parsetm(&end, *argv) != 0) {
			fprint(2, "%s: invalid time: %s\n", argv0, *argv);
			exits("invalid");
		}
		if(*start.zone && !*end.zone) {
			strcpy(end.zone, start.zone);
			end.tzoff = start.tzoff;
		} else if(!*start.zone && end.zone) {
			strcpy(start.zone, end.zone);
			start.tzoff = end.tzoff;
		}
		duration = labs((tm2sec(&end)-end.tzoff) - (tm2sec(&start)-start.tzoff));
	} else if(argc == 1) {
		duration = parsedur(*argv);
	} else {
		usage();
	}
	if(nflg) {
		print("%ld\n", duration);
	} else {
		print("P");
		duration = labs(duration);
		duration = putdur(duration, 'Y', 60*60*24*365);
		duration = putdur(duration, 'D', 60*60*24);
		print("T");
		duration = putdur(duration, 'H', 60*60);
		duration = putdur(duration, 'M', 60);
		duration = putdur(duration, 'S', 1);
		print("\n");
	}
	exits(nil);
}
