#!/usr/bin/python

import argparse
import subprocess

def main():
  parser = argparse.ArgumentParser(
      description = "Run a variant of the dlx binary")
  binary = parser.add_mutually_exclusive_group()
  binary.add_argument("--sharp", help="Use the sharp heuristic", action="store_true")
  binary.add_argument("--skip", help="Use the skip heuristic", action="store_true")
  binary.add_argument("--dlx1", help="Use dlx1 (original)", action="store_true")
  binary.add_argument("--dlx2", help="Use dlx2 (variant with colors as characters)", action="store_true")
  binary.add_argument("--digits", help="Use dlx2 (variant with colors as digits)", action="store_true")
  binary.add_argument("--dlx3", help="Use dlx3 (variant with multiplicity)", action="store_true")
  binary.add_argument("--dlx5", help="Use dlx5 (variant with cost)", action="store_true")
  speed = parser.add_mutually_exclusive_group()
  speed.add_argument("--speed", help="Output progress in a given speed", nargs=1, type=int)
  speed.add_argument("--fast", help="Output progress fast", action="store_true")
  speed.add_argument("--faster", help="Output progress faster", action="store_true")
  parser.add_argument("--pre", help="Run the preprocessor", nargs=1,
      choices=["pipe", "stop"])
  parser.add_argument("--verbose", help="Verbose output", action="store_true")
  parser.add_argument("--random", help="Use random choices when branching", nargs="?", const=1, type=int, default=None)
  parser.add_argument("--stop", help="Stop at the nth solution", nargs="?",
      default=None, type=int, metavar="n")
  parser.add_argument("template", help="File template")
  args = parser.parse_args()
  if args.sharp:
    executable = "dlx2.sharp"
  elif args.skip:
    executable = "dlx2.skip"
  elif args.digits:
    executable = "dlx2.digits"
  elif args.dlx1:
    executable = "dlx1"
  elif args.dlx3:
    executable = "dlx3"
  elif args.dlx5:
    executable = "dlx5"
  else:
    executable = "dlx2"
  verbose = "v391" if args.verbose else ""
  random = "s%d" % args.random if args.random else ""
  if args.speed:
    ticks = "d%d" % args.speed[0]
  elif args.fast:
    ticks = "d100000000"
  elif args.faster:
    ticks = "d10000000"
  else:
    ticks = "d1000000000"
  stop = "t%d" % args.stop if args.stop else ""
  if args.pre and args.pre[0] == "stop":
    commandline = (
        '(time /home/ricbit/work/knuth/dlx/dlx-pre < %s.dlx) > '
        '%s.pre.dlx ' % (args.template, args.template))
  else:
    if args.pre and args.pre[0] == "pipe":
      commandpipe = (
          '/home/ricbit/work/knuth/dlx/dlx-pre < %s.dlx | '
          '/home/ricbit/work/knuth/dlx/%s m1 %s %s %s %s ' % 
          (args.template, executable, ticks, stop, verbose, random))
    else:
      commandpipe = ('/home/ricbit/work/knuth/dlx/%s m1 %s %s %s %s < %s.dlx ' %
          (executable, ticks, stop, verbose, random, args.template))
    commandline = (
        '(time (%s)) |& tee %s.out.txt | grep -v -P "^(\s(?!after)|\d+:|Level|L\d+)"' %
        (commandpipe, args.template))
  subprocess.call(commandline, shell=True, executable="/bin/bash")

main()


