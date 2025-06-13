#ifndef HELP_H
# define HELP_H

# define HELP "Usage: ping [OPTION...] HOST ...\n\
Send ICMP ECHO_REQUEST packets to network hosts.\n\
\n\
 Options valid for all request types:\n\
\n\
  -c, --count=NUMBER         stop after sending NUMBER packets\n\
  -i, --interval=NUMBER      wait NUMBER seconds between sending each packet\n\
      --ttl=N                specify N as time-to-live\n\
  -v, --verbose              verbose output\n\
  -w, --timeout=N            stop after N seconds\n\
  -f, --flood                flood ping\n\
\n\
 Options valid for --echo requests:\n\
\n\
  -q, --quiet                quiet output\n\
\n\
  -?, --help                 give this help list\n\
      --usage                give a short usage message\n\
\n\
Mandatory or optional arguments to long options are also mandatory or optional\n\
for any corresponding short options.\n\
\n\
Report bugs to <psegura-@student.42madrid.com>.\
"

#define USAGE "Usage: ping [--verbose] [--help] [--usage]\n\
            [-c NUMBER] [--count=NUMBER] \n\
            [-i NUMBER] [--interval=NUMBER]\n\
            [-w NUMBER] [--timeout=NUMBER]\n\
            [-f]        [--flood]\n\
            [--ttl=N]\n\
            [--quiet]\n\
            HOST ...\
"

#endif
