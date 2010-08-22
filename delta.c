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

double
parsedur(char *cp)
{
	double duration = 0, val;
	expect(*cp == 'P');
	while(*(cp+1)) {
		val = strtod(cp+1, &cp);
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
	fprint(2, "usage: delta [-n] (START [+-] DELTA|START END|DELTA)\n");
	exits("usage");
}

int nflg;

void
main(int argc, char **argv)
{
	double duration = 0;

	ARGBEGIN {
	case 'n':	nflg = 1; break;
	default:	usage();
	} ARGEND

	if(argc == 3) {
		Tm start, *end;
		char zone[4];
		if(parsetm(&start, *argv) != 0) {
			fprint(2, "%s: invalid time: %s\n", argv0, *argv);
			exits("invalid");
		}
		++argv;
		switch(**argv) {
			case '+':	duration = 1; break;
			case '-':	duration = -1; break;
			default:	fprint(2, "%s: unknown operation: %s\n", argv0, *argv); exits("invalide");
		}
		++argv;
		duration *= parsedur(*argv);
		strcpy(zone, start.zone);
		strcpy(start.zone, "GMT"); /* so tm2sec doesn't assume local time */
		end = gmtime(tm2sec(&start) + duration);
		memcpy(end->zone, start.zone, 4);
		end->tzoff = start.tzoff;
		print("%04d-%02d-%02dT%02d:%02d:%02d%+03d:%02d\n", end->year+1900, end->mon+1, end->mday, end->hour, end->min, end->sec, end->tzoff/3600, abs(end->tzoff)%3600);
		exits(nil);
	} else if(argc == 2) {
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
		duration = abs((tm2sec(&end)-end.tzoff) - (tm2sec(&start)-start.tzoff));
	} else if(argc == 1) {
		duration = parsedur(*argv);
	} else {
		usage();
	}
	if(nflg) {
		print("%.11g\n", duration);
	} else {
		print("P");
		duration = abs(duration);
		duration = putdur((long)duration, 'Y', 60*60*24*365);
		duration = putdur((long)duration, 'D', 60*60*24);
		print("T");
		duration = putdur((long)duration, 'H', 60*60);
		duration = putdur((long)duration, 'M', 60);
		duration = putdur((long)duration, 'S', 1);
		print("\n");
	}
	exits(nil);
}
